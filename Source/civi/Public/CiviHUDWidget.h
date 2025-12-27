// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CiviTypes.h"
#include "CiviHUDWidget.generated.h"

class ACity;
class AUnit;

UCLASS()
class CIVI_API UCiviHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // --- 更新界面显示 (由 PlayerController 调用) ---

    // 更新全局信息 (回合数、金币等)
    UFUNCTION(BlueprintImplementableEvent, Category = "Civi UI")
    void UpdateTurnInfo(int32 TurnNumber, int32 PlayerIndex, FYields GlobalResources);

    // 显示/隐藏 单位面板
    UFUNCTION(BlueprintImplementableEvent, Category = "Civi UI")
    void UpdateUnitPanel(AUnit* SelectedUnit, bool bIsVisible);

    // 显示/隐藏 城市面板
    UFUNCTION(BlueprintImplementableEvent, Category = "Civi UI")
    void UpdateCityPanel(ACity* SelectedCity, bool bIsVisible);

    // 显示/隐藏 地块信息
    UFUNCTION(BlueprintImplementableEvent, Category = "Civi UI")
    void UpdateTilePanel(ETerrain Terrain, ELandform Landform, bool bIsVisible);

    // --- 按钮回调 (由 UI 按钮绑定调用) ---

    // 结束回合按钮点击
    UFUNCTION(BlueprintCallable, Category = "Civi UI")
    void OnEndTurnClicked();

    // 单位行动按钮 (0:移动, 1:驻守, 2:建城, 3:跳过)
    UFUNCTION(BlueprintCallable, Category = "Civi UI")
    void OnUnitActionClicked(int32 ActionID);

    // 显示胜利画面
    UFUNCTION(BlueprintImplementableEvent, Category = "Civi UI")
    void ShowVictoryScreen(int32 WinnerIndex, EVictoryType VictoryType);
};