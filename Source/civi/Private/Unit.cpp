// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit.h"
#include "Landblock.h"
#include "UnitDataAsset.h"
#include "City.h"
#include "CombatFunctionLibrary.h"

AUnit::AUnit()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitMesh"));
    RootComponent = MeshComponent;
}

void AUnit::BeginPlay()
{
    Super::BeginPlay();
}

void AUnit::InitUnit(ECiviUnitType Type, int32 OwnerIndex, ULandblock* StartBlock, UUnitDataAsset* DataAsset)
{
    if (!DataAsset || !StartBlock) return;

    FUnitInfo Info = DataAsset->GetUnitInfo(Type);

    UnitType = Type;
    PlayerOwnerIndex = OwnerIndex;
    MaxMovementPoints = Info.MaxMovementPoints;
    MovementPoints = MaxMovementPoints;
    CombatStrength = Info.CombatStrength;
    MaxHP = 100;
    CurrentHP = MaxHP;
    bIsFortified = false;

    // 设置位置
    CurrentBlock = StartBlock;
    GridX = StartBlock->X;
    GridY = StartBlock->Y;
    StartBlock->OccupyingUnit = this;

    // 设置视觉
    if (Info.Mesh)
    {
        MeshComponent->SetStaticMesh(Info.Mesh);
        SetActorScale3D(FVector(Info.Scale));
    }

    UpdateWorldLocation();
}

void AUnit::UpdateWorldLocation()
{
    // 这里简单复用六边形坐标公式，建议提取到通用工具类
    float HexRadius = 100.0f; // 需要与 MapRenderer 一致
    float Width = HexRadius * 2.0f + 2.0f;
    float Height = HexRadius * FMath::Sqrt(3.0f) + 2.0f;

    float WorldX = GridX * Width * 0.75f;
    float WorldY = GridY * Height;
    if (GridX % 2 == 1) WorldY += Height * 0.5f;

    SetActorLocation(FVector(WorldX, WorldY, 20.0f)); // Z轴抬高一点避免穿模
}

bool AUnit::MoveTo(ULandblock* TargetBlock)
{
    if (!TargetBlock || MovementPoints <= 0) return false;
    if (TargetBlock == CurrentBlock) return false;

    // 1. 检查目标是否可通行
    if (!TargetBlock->IsPassable())
    {
        UE_LOG(LogTemp, Warning, TEXT("Target is impassable"));
        return false;
    }

    // 2. 检查是否有敌方单位 (如果有则需要攻击，不能直接移动)
    if (TargetBlock->HasUnit())
    {
        if (TargetBlock->OccupyingUnit->PlayerOwnerIndex != this->PlayerOwnerIndex)
        {
            AttackUnit(TargetBlock->OccupyingUnit);
            return true; // 攻击消耗了行动，算作一次"Move"尝试
        }
        else
        {
            return false; // 不能与己方单位重叠 (简化规则)
        }
    }

    // 3. 计算移动消耗 (简化版：只判断相邻)
    // 实际项目中需要 A* 寻路算法来支持长距离点击
    bool bIsNeighbor = false;
    for (ULandblock* Neighbor : CurrentBlock->Neighbors)
    {
        if (Neighbor == TargetBlock)
        {
            bIsNeighbor = true;
            break;
        }
    }

    if (!bIsNeighbor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unit can only move to neighbors directly (Pathfinding WIP)"));
        return false;
    }

    int32 Cost = TargetBlock->GetMovementCost();
    if (MovementPoints < Cost)
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough movement points"));
        return false;
    }

    // 4. 执行移动
    // 解除旧地块引用
    if (CurrentBlock) CurrentBlock->OccupyingUnit = nullptr;

    // 更新状态
    MovementPoints -= Cost;
    CurrentBlock = TargetBlock;
    GridX = TargetBlock->X;
    GridY = TargetBlock->Y;
    bIsFortified = false; // 移动取消驻守

    // 建立新引用
    TargetBlock->OccupyingUnit = this;

    // 更新视觉
    UpdateWorldLocation();

    return true;
}

void AUnit::AttackUnit(AUnit* Defender)
{
    if (!Defender || MovementPoints <= 0) return;

    // 消耗所有移动力
    MovementPoints = 0;
    bIsFortified = false;

    // 获取双方修正后的战力
    int32 MyStr = this->CombatStrength; // 攻击者通常没有地形加成(除非特殊单位)
    int32 DefStr = Defender->GetDefensiveStrength();

    // 计算伤害
    int32 DmgToDef = UCombatFunctionLibrary::CalculateDamage(MyStr, DefStr);
    int32 DmgToAtt = UCombatFunctionLibrary::CalculateDamage(DefStr, MyStr); // 反击

    // 应用伤害
    Defender->CurrentHP -= DmgToDef;
    this->CurrentHP -= DmgToAtt;

    UE_LOG(LogTemp, Log, TEXT("Melee Combat: %s(%d) vs %s(%d). Dmg: %d / %d"),
        *GetName(), MyStr, *Defender->GetName(), DefStr, DmgToDef, DmgToAtt);

    // 死亡判定
    if (Defender->CurrentHP <= 0)
    {
        Defender->Destroy();
        if (Defender->CurrentBlock)
        {
            Defender->CurrentBlock->OccupyingUnit = nullptr;
            // 近战胜利进驻
            MoveTo(Defender->CurrentBlock);
        }
    }

    if (this->CurrentHP <= 0)
    {
        this->Destroy();
        if (CurrentBlock) CurrentBlock->OccupyingUnit = nullptr;
    }
}

int32 AUnit::CalculateDamage(int32 AttackerStr, int32 DefenderStr)
{
    // 简化的Civ6类似公式：根据战力差计算伤害
    // 战力差 = 30 左右秒杀
    int32 Diff = AttackerStr - DefenderStr;
    float BaseDamage = 30.0f;

    // 指数增长/衰减
    float Result = BaseDamage * FMath::Exp(0.04f * Diff);

    // 随机浮动 +/- 10%
    Result *= FMath::RandRange(0.9f, 1.1f);

    return FMath::Clamp(FMath::RoundToInt(Result), 0, 100);
}

void AUnit::FoundCity()
{
    if (UnitType != ECiviUnitType::Settler) return;
    if (MovementPoints <= 0) return;
    if (!CurrentBlock) return;

    // 检查是否已经有城市
    // if (CurrentBlock->OwningCity) return; // 假设不能在别人领土建城(简化)

    UE_LOG(LogTemp, Log, TEXT("Founding City at %d, %d"), GridX, GridY);

    // 生成城市 Actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this->GetOwner();

    ACity* NewCity = GetWorld()->SpawnActor<ACity>(ACity::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams);

    if (NewCity)
    {
        NewCity->GridX = GridX;
        NewCity->GridY = GridY;
        NewCity->PlayerOwnerIndex = PlayerOwnerIndex;
        NewCity->CityName = FText::FromString(TEXT("New City"));

        // 设置领土
        NewCity->AddTerritory(CurrentBlock);
        // 可以添加周围一圈地块...

        // 消耗开拓者
        this->Destroy();
        CurrentBlock->OccupyingUnit = nullptr;
    }
}

void AUnit::Fortify()
{
    if (MovementPoints > 0)
    {
        bIsFortified = true;
        MovementPoints = 0; // 驻守结束回合
    }
}

void AUnit::OnTurnStart()
{
    MovementPoints = MaxMovementPoints;
    // 如果驻守中，可以回复HP
    if (bIsFortified && CurrentHP < MaxHP)
    {
        CurrentHP = FMath::Min(MaxHP, CurrentHP + 15);
    }
}

int32 AUnit::GetDefensiveStrength() const
{
    int32 FinalStr = CombatStrength;

    // 1. 地形修正
    if (CurrentBlock)
    {
        FinalStr += UCombatFunctionLibrary::GetTerrainDefenseBonus(CurrentBlock);
    }

    // 2. 驻守修正
    if (bIsFortified)
    {
        FinalStr += 6; // 驻守+6
    }

    // 3. 受伤修正 (HP越低战斗力越低)
    if (CurrentHP < 100)
    {
        // 每损失10点血，战力-1 (简单算法)
        // FinalStr -= (100 - CurrentHP) / 10;
    }

    return FinalStr;
}

void AUnit::AttackCity(ACity* TargetCity)
{
    if (!TargetCity || MovementPoints <= 0) return;

    MovementPoints = 0;
    bIsFortified = false;

    // 城市防御力
    int32 CityStr = TargetCity->CombatStrength;
    int32 MyStr = this->CombatStrength;

    // 近战攻城：城市会反击
    int32 DmgToCity = UCombatFunctionLibrary::CalculateDamage(MyStr, CityStr);
    int32 DmgToUnit = UCombatFunctionLibrary::CalculateDamage(CityStr, MyStr);

    TargetCity->ReceiveDamage(DmgToCity, this);
    this->CurrentHP -= DmgToUnit;

    UE_LOG(LogTemp, Log, TEXT("Siege Combat: Unit took %d damage"), DmgToUnit);

    if (this->CurrentHP <= 0)
    {
        this->Destroy();
        if (CurrentBlock) CurrentBlock->OccupyingUnit = nullptr;
    }
}

void AUnit::RangedAttackUnit(AUnit* TargetUnit)
{
    // 检查：必须有远程能力，且未移动完(或设专门的攻击点数)
    // 这里的 Range 已经在 UUnitDataAsset 中定义，需确保 InitUnit 时读取到 Unit 成员变量
    // 假设 Unit.h 中已添加 RangedStrength 和 Range 成员 (请确保从 DataAsset 赋值)

    if (!TargetUnit || MovementPoints <= 0) return;

    // 简易距离检查 (曼哈顿距离或六边形距离)
    // int32 Dist = FMath::Max(FMath::Abs(GridX - TargetUnit->GridX), FMath::Abs(GridY - TargetUnit->GridY)); // 近似
    // if (Dist > Range) return;

    MovementPoints = 0;
    bIsFortified = false;

    // 远程攻击：攻击者不受反击伤害
    int32 MyRangedStr = 20; // 暂定，实际应从 Unit 属性读取
    int32 DefStr = TargetUnit->GetDefensiveStrength();

    int32 Dmg = UCombatFunctionLibrary::CalculateDamage(MyRangedStr, DefStr);
    TargetUnit->CurrentHP -= Dmg;

    if (TargetUnit->CurrentHP <= 0)
    {
        TargetUnit->Destroy();
        if (TargetUnit->CurrentBlock) TargetUnit->CurrentBlock->OccupyingUnit = nullptr;
    }
}

void AUnit::RangedAttackCity(ACity* TargetCity)
{
    if (!TargetCity || MovementPoints <= 0) return;

    MovementPoints = 0;

    int32 MyRangedStr = 20; // 需从属性读取
    int32 CityStr = TargetCity->CombatStrength;

    // 远程攻城：城市有区域防御，可能会受减免，这里简化为直接伤害
    int32 Dmg = UCombatFunctionLibrary::CalculateDamage(MyRangedStr, CityStr);

    // 城市不能被远程直接占领，最多打到 0 HP
    if (TargetCity->CurrentHP - Dmg <= 0)
    {
        TargetCity->CurrentHP = 1; // 留 1 血给近战占领
    }
    else
    {
        TargetCity->ReceiveDamage(Dmg, nullptr);
    }
}