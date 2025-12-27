// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CiviTypes.h"
#include "TechDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FTechInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETechType Type = ETechType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    // 研发所需科技值
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ScienceCost = 25;

    // 前置科技 (必须全部解锁才能研究此科技)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<ETechType> Prerequisites;

    // --- 解锁奖励 (UI显示与逻辑检查用) ---

    // 解锁的建筑
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingType UnlocksBuilding = EBuildingType::None;

    // 解锁的单位
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECiviUnitType UnlocksUnit = ECiviUnitType::None;

    // 解锁的奇观
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWonderType UnlocksWonder = EWonderType::None;
};

UCLASS(BlueprintType)
class CIVI_API UTechDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Tree")
    TArray<FTechInfo> Techs;

    UFUNCTION(BlueprintCallable)
    FTechInfo GetTechInfo(ETechType Type) const;
};