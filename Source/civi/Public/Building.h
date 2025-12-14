// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CiviTypes.h"
#include "Building.generated.h"

/**
 * 建筑逻辑实体
 */
UCLASS(BlueprintType)
class CIVI_API UBuilding : public UObject
{
    GENERATED_BODY()

public:
    UBuilding();

    // 建筑类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", Meta = (ExposeOnSpawn = true))
    EBuildingType Type;

    // 是否被掠夺
    UPROPERTY(BlueprintReadWrite, Category = "Building")
    bool bIsPillaged;

    // 可以在这里添加更多逻辑，例如:
    // int32 Level;
    // int32 ConstructionTurn;

    // 获取该建筑的产出修正（示例接口）
    // virtual void GetYieldModifier(...) const;
};
