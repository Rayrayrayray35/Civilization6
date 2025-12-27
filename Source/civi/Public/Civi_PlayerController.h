// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Civi_PlayerController.generated.h"

class ACity;
class AUnit;
class UCiviHUDWidget;
class ULandblock;

UCLASS()
class CIVI_API ACivi_PlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ACivi_PlayerController();

    // UI 蓝图类引用 (在编辑器中设置 BP_CiviHUDWidget)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UCiviHUDWidget> HUDWidgetClass;

    // 当前选中的单位
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
    AUnit* SelectedUnit;

    // 当前选中的城市
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
    ACity* SelectedCity;

    // UI 实例
    UPROPERTY()
    UCiviHUDWidget* HUDInstance;

    // 执行单位操作 (0:移动模式, 1:驻守, 2:建城)
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ExecuteUnitAction(int32 ActionID);

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void PlayerTick(float DeltaTime) override;

    // 鼠标点击处理
    void OnLeftClick();
    void OnRightClick();

    // 更新 UI 显示
    void UpdateUI();

private:
    // 辅助：处理单位移动/攻击指令
    void HandleUnitMovement(ULandblock* TargetBlock);

    // 辅助：选中对象
    void SelectUnit(AUnit* Unit);
    void SelectCity(ACity* City);
    void SelectTile(ULandblock* Block);
    void ClearSelection();
};