// Fill out your copyright notice in the Description page of Project Settings.

#include "CiviHUDWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Civi_GameModeBase.h"
#include "Civi_PlayerController.h"

void UCiviHUDWidget::OnEndTurnClicked()
{
    // 获取 GameMode 并结束回合
    if (ACivi_GameModeBase* GM = Cast<ACivi_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        GM->EndTurn();
    }
}

void UCiviHUDWidget::OnUnitActionClicked(int32 ActionID)
{
    // 获取控制器并执行单位指令
    if (ACivi_PlayerController* PC = Cast<ACivi_PlayerController>(GetOwningPlayer()))
    {
        PC->ExecuteUnitAction(ActionID);
    }
}