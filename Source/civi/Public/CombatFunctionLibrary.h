// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CiviTypes.h"
#include "CombatFunctionLibrary.generated.h"

class ULandblock;

UCLASS()
class CIVI_API UCombatFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 计算伤害 (基于文明6公式：Damage ~ 30 * e^(0.04 * Diff))
    UFUNCTION(BlueprintCallable, Category = "Combat System")
    static int32 CalculateDamage(int32 AttackerStrength, int32 DefenderStrength);

    // 计算地块提供的防御加成 (如丘陵+3, 森林+3)
    UFUNCTION(BlueprintCallable, Category = "Combat System")
    static int32 GetTerrainDefenseBonus(ULandblock* Block);
};