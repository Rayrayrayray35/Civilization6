// Fill out your copyright notice in the Description page of Project Settings.

#include "City.h"
#include "Landblock.h"
#include "BuildingDataAsset.h"
#include "TerrainDataAsset.h"
#include "Unit.h"
#include "Civi_GameModeBase.h"
#include "Kismet/GameplayStatics.h"

ACity::ACity()
{
    PrimaryActorTick.bCanEverTick = false;

    // 创建根组件用于在世界中显示位置
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    CurrentHP = MaxHP;
}

void ACity::BeginPlay()
{
    Super::BeginPlay();
}

void ACity::AddTerritory(ULandblock* NewTile)
{
    if (NewTile && !OwnedTiles.Contains(NewTile))
    {
        OwnedTiles.Add(NewTile);
        NewTile->SetOwningCity(this);
    }
}

FYields ACity::CalculateTurnYields(const UTerraindataasset* TData, const UBuildingDataAsset* BData) const
{
    FYields TotalYields;

    // 1. 城市中心基础产出
    TotalYields.Food += 2;
    TotalYields.Production += 1;
    TotalYields.Gold += 1; // 城市中心自带1金币

    // 2. 遍历领土产出
    for (ULandblock* Tile : OwnedTiles)
    {
        if (Tile)
        {
            // 调用 Landblock 的完整计算逻辑
            TotalYields = TotalYields + Tile->GetTotalYield(TData, BData);
        }
    }

    // 3. 人口消耗
    int32 FoodConsumption = Population * 2;
    TotalYields.Food -= FoodConsumption;

    return TotalYields;
}

void ACity::StartProductionBuilding(EBuildingType BuildingType, UBuildingDataAsset* DataAsset)
{
    if (!DataAsset) return;

    FBuildingDisplayData BData = DataAsset->GetBuildingDisplayData(BuildingType);

    CurrentProduction.BuildingType = BuildingType;
    CurrentProduction.TotalCost = BData.ProductionCost;
    CurrentProduction.Progress = 0;
    CurrentProduction.Name = BData.DisplayName;

    // 加上之前的溢出产能
    CurrentProduction.Progress += ProductionOverflow;
    ProductionOverflow = 0;

    bHasActiveProduction = true;

    UE_LOG(LogTemp, Log, TEXT("City %s started building %s. Cost: %d"), *CityName.ToString(), *BData.DisplayName.ToString(), CurrentProduction.TotalCost);
}

void ACity::ProcessTurn(const UTerraindataasset* TData, const UBuildingDataAsset* BData)
{
    // 获取本回合产出
    FYields TurnYields = CalculateTurnYields(TData, BData);

    // --- 1. 处理人口增长 (使用本地的粮食盈余) ---
    FoodStock += TurnYields.Food;

    int32 GrowthThreshold = GetFoodNeededForGrowth();
    if (FoodStock >= GrowthThreshold)
    {
        Population++;
        FoodStock -= GrowthThreshold;
    }
    else if (FoodStock < 0)
    {
        if (Population > 1) { Population--; FoodStock = 0; }
    }

    // --- 2. 处理生产建造 (使用本地的生产力盈余) ---
    if (bHasActiveProduction)
    {
        int32 ProductionToAdd = FMath::Max(0, TurnYields.Production);
        CurrentProduction.Progress += ProductionToAdd;

        if (CurrentProduction.Progress >= CurrentProduction.TotalCost)
        {
            FinishProduction();
        }
    }

    // 金币、科技、文化不在这里处理，它们会由 GameMode 收集并存入国库

    // 城市回血
    RegenerateHealth();
}

void ACity::RegenerateHealth()
{
    if (CurrentHP < MaxHP)
    {
        CurrentHP = FMath::Min(MaxHP, CurrentHP + 20); // 每回合回20血
    }

    // 动态更新防御力：基础10 + 人口
    CombatStrength = 10 + Population * 2;
}

void ACity::ReceiveDamage(int32 DamageAmount, AUnit* Attacker)
{
    CurrentHP -= DamageAmount;
    UE_LOG(LogTemp, Warning, TEXT("City %s took %d damage! HP: %d/%d"), *CityName.ToString(), DamageAmount, CurrentHP, MaxHP);

    if (CurrentHP <= 0)
    {
        // 城市沦陷
        CurrentHP = 0;
        if (Attacker)
        {
            OnCityCaptured(Attacker->PlayerOwnerIndex);
        }
    }
}

void ACity::OnCityCaptured(int32 NewOwnerIndex)
{
    UE_LOG(LogTemp, Error, TEXT("CITY CAPTURED! %s is now owned by Player %d"), *CityName.ToString(), NewOwnerIndex);

    PlayerOwnerIndex = NewOwnerIndex;
    CurrentHP = 50; // 占领后恢复少量血
    Population = FMath::Max(1, Population / 2); // 人口减半

    // 清空建造队列
    CurrentProduction = FProductionItem();
    bHasActiveProduction = false;

    // 通知 GameMode 检查胜利条件
    if (ACivi_GameModeBase* GM = Cast<ACivi_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        GM->CheckVictoryConditions();
    }
}

int32 ACity::GetFoodNeededForGrowth() const
{
    // 简单的增长公式：15 + 8 * (Population - 1)
    return 15 + 8 * (Population - 1);
}

void ACity::FinishProduction()
{
    // 计算溢出产能
    ProductionOverflow = CurrentProduction.Progress - CurrentProduction.TotalCost;

    UE_LOG(LogTemp, Log, TEXT("City %s finished constructing %s!"), *CityName.ToString(), *CurrentProduction.Name.ToString());

    // --- 实际生成效果 ---
    if (CurrentProduction.BuildingType != EBuildingType::None)
    {
        // 这里的逻辑：
        // 1. 找到一个空地块或者城市中心地块来放置建筑
        // 2. 调用 Landblock->ConstructBuilding()

        // 简化实现：直接建在城市中心（如果有空位）或者仅仅作为一个逻辑标记
        // 在完整版中，需要在 StartProduction 时指定目标地块 TargetPlot

        if (CityCenterTile)
        {
            // 假设我们直接把建筑加到城市列表里，或者修改地块模型
            // CityCenterTile->ConstructBuilding(CurrentProduction.BuildingType);
            // 注意：这会覆盖之前的建筑，需要更完善的"区域"系统
        }
    }

    // 重置队列
    bHasActiveProduction = false;
    CurrentProduction = FProductionItem();
}