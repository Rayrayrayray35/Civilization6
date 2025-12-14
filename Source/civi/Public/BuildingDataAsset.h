// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CiviTypes.h"
#include "BuildingDataAsset.generated.h"

// 单个建筑的显示配置
USTRUCT(BlueprintType)
struct FBuildingDisplayData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingType BuildingType = EBuildingType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MeshScale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeightOffset = 0.0f;

    // 某些建筑（如农田）可能需要随机旋转，而某些（如矿山）可能固定朝向
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRandomRotation = false;
};

UCLASS(BlueprintType)
class CIVI_API UBuildingDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Data")
    TArray<FBuildingDisplayData> BuildingData;

    UFUNCTION(BlueprintCallable, Category = "Building Data")
    FBuildingDisplayData GetBuildingDisplayData(EBuildingType Type) const;
};
