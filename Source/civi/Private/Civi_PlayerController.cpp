// Fill out your copyright notice in the Description page of Project Settings.

#include "Civi_PlayerController.h"
#include "CiviHUDWidget.h"
#include "Civi_GameModeBase.h"
#include "Unit.h"
#include "City.h"
#include "Landblock.h"
#include "Kismet/GameplayStatics.h"

ACivi_PlayerController::ACivi_PlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void ACivi_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 创建 UI
    if (HUDWidgetClass)
    {
        HUDInstance = CreateWidget<UCiviHUDWidget>(this, HUDWidgetClass);
        if (HUDInstance)
        {
            HUDInstance->AddToViewport();
        }
    }
}

void ACivi_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // 绑定鼠标点击 (需要在 Project Settings -> Input 中设置 Action Mapping，或者直接用 Key)
    // 这里为了方便直接绑定 Key，实际建议用 EnhancedInput
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACivi_PlayerController::OnLeftClick);
    InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ACivi_PlayerController::OnRightClick);
}

void ACivi_PlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    UpdateUI();
}

void ACivi_PlayerController::UpdateUI()
{
    if (!HUDInstance) return;

    ACivi_GameModeBase* GM = Cast<ACivi_GameModeBase>(GetWorld()->GetAuthGameMode());
    if (GM)
    {
        // 1. 更新全局信息
        HUDInstance->UpdateTurnInfo(GM->CurrentTurn, GM->CurrentPlayerIndex, GM->GetPlayerResources(GM->CurrentPlayerIndex));
    }
}

void ACivi_PlayerController::OnLeftClick()
{
    FHitResult Hit;
    GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    if (Hit.bBlockingHit)
    {
        AActor* HitActor = Hit.GetActor();

        // 1. 点击了单位
        if (AUnit* ClickedUnit = Cast<AUnit>(HitActor))
        {
            // 只能选中自己的单位
            ACivi_GameModeBase* GM = Cast<ACivi_GameModeBase>(GetWorld()->GetAuthGameMode());
            if (GM && ClickedUnit->PlayerOwnerIndex == GM->CurrentPlayerIndex)
            {
                SelectUnit(ClickedUnit);
                return;
            }
        }

        // 2. 点击了城市
        if (ACity* ClickedCity = Cast<ACity>(HitActor))
        {
            // 只能选中自己的城市
            ACivi_GameModeBase* GM = Cast<ACivi_GameModeBase>(GetWorld()->GetAuthGameMode());
            if (GM && ClickedCity->PlayerOwnerIndex == GM->CurrentPlayerIndex)
            {
                SelectCity(ClickedCity);
                return;
            }
        }

        // 3. 点击了地形 (处理移动或查看信息)
        ACivi_GameModeBase* GM = Cast<ACivi_GameModeBase>(GetWorld()->GetAuthGameMode());
        if (GM)
        {
            ULandblock* ClickedBlock = GM->GetLandblockFromWorldPos(Hit.Location);
            if (ClickedBlock)
            {
                if (SelectedUnit)
                {
                    // 如果已选中单位，点击地板尝试移动
                    HandleUnitMovement(ClickedBlock);
                }
                else
                {
                    // 否则查看地块信息
                    SelectTile(ClickedBlock);
                }
            }
        }
    }
    else
    {
        ClearSelection();
    }
}

void ACivi_PlayerController::OnRightClick()
{
    // 右键通常用于取消选择或直接移动
    if (SelectedUnit)
    {
        FHitResult Hit;
        GetHitResultUnderCursor(ECC_Visibility, false, Hit);

        ACivi_GameModeBase* GM = Cast<ACivi_GameModeBase>(GetWorld()->GetAuthGameMode());
        if (GM)
        {
            ULandblock* TargetBlock = GM->GetLandblockFromWorldPos(Hit.Location);
            if (TargetBlock)
            {
                HandleUnitMovement(TargetBlock);
            }
        }
    }
    else
    {
        ClearSelection();
    }
}

void ACivi_PlayerController::HandleUnitMovement(ULandblock* TargetBlock)
{
    if (SelectedUnit && TargetBlock)
    {
        bool bMoved = SelectedUnit->MoveTo(TargetBlock);
        if (bMoved)
        {
            // 移动后刷新UI
            if (HUDInstance) HUDInstance->UpdateUnitPanel(SelectedUnit, true);
        }
        else if (TargetBlock->HasUnit() && TargetBlock->OccupyingUnit != SelectedUnit)
        {
            // 尝试攻击逻辑已经在 MoveTo 里包含了，如果 MoveTo 失败可能是距离不够
            // 如果 MoveTo 内部处理了攻击，这里不需要额外操作
        }
    }
}

void ACivi_PlayerController::SelectUnit(AUnit* Unit)
{
    ClearSelection();
    SelectedUnit = Unit;

    // UI 反馈
    if (HUDInstance) HUDInstance->UpdateUnitPanel(SelectedUnit, true);
    UE_LOG(LogTemp, Log, TEXT("Selected Unit: %s"), *Unit->GetName());
}

void ACivi_PlayerController::SelectCity(ACity* City)
{
    ClearSelection();
    SelectedCity = City;

    // UI 反馈
    if (HUDInstance) HUDInstance->UpdateCityPanel(SelectedCity, true);
}

void ACivi_PlayerController::SelectTile(ULandblock* Block)
{
    ClearSelection();
    if (HUDInstance && Block)
    {
        HUDInstance->UpdateTilePanel(Block->Terrain, Block->Landform, true);
    }
}

void ACivi_PlayerController::ClearSelection()
{
    SelectedUnit = nullptr;
    SelectedCity = nullptr;

    if (HUDInstance)
    {
        HUDInstance->UpdateUnitPanel(nullptr, false);
        HUDInstance->UpdateCityPanel(nullptr, false);
        HUDInstance->UpdateTilePanel(ETerrain::Plain, ELandform::None, false);
    }
}

void ACivi_PlayerController::ExecuteUnitAction(int32 ActionID)
{
    if (!SelectedUnit) return;

    switch (ActionID)
    {
        case 1: // 驻守
            SelectedUnit->Fortify();
            ClearSelection(); // 驻守后取消选择
            break;
        case 2: // 建城
            SelectedUnit->FoundCity();
            ClearSelection();
            break;
        case 3: // 跳过/休眠
            // SelectedUnit->Sleep();
            ClearSelection();
            break;
    }
}