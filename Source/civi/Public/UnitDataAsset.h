// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CiviTypes.h"
#include "UnitDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FUnitInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECiviUnitType UnitType = ECiviUnitType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;

    // --- 基础属性 ---

    // 最大移动点数
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxMovementPoints = 2;

    // 近战攻击力
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CombatStrength = 10;

    // 远程攻击力 (0表示无远程能力)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RangedStrength = 0;

    // 射程
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Range = 0;

    // 生产成本
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ProductionCost = 40;

    // --- 视觉表现 ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Scale = 1.0f;
};

UCLASS(BlueprintType)
class CIVI_API UUnitDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Data")
    TArray<FUnitInfo> Units;

    UFUNCTION(BlueprintCallable)
    FUnitInfo GetUnitInfo(ECiviUnitType Type) const;
};