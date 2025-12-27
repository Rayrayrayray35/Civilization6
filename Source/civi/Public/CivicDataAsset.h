// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CiviTypes.h"
#include "CivicDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FCivicInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECivicType Type = ECivicType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CultureCost = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<ECivicType> Prerequisites;

    // 可以在此添加解锁的政策卡槽 (Policy Cards) 等
};

UCLASS(BlueprintType)
class CIVI_API UCivicDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civic Tree")
    TArray<FCivicInfo> Civics;

    UFUNCTION(BlueprintCallable)
    FCivicInfo GetCivicInfo(ECivicType Type) const;
};