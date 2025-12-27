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

    // 建造所需的生产力
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameLogic")
    int32 ProductionCost = 60;

    // 维护费
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameLogic")
    int32 MaintenanceCost = 1;

    // 建筑提供的产出 (例如：粮仓 +2 粮食)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameLogic")
    FYields BonusYields;
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
