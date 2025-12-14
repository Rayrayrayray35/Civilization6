// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CiviTypes.h"
#include "Wonder.h"
#include "WonderDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FWonderDisplayData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWonderType WonderType = EWonderType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    // 奇观的基础模型
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh = nullptr;

    // 指定特定的 Actor 子类来生成（如果需要复杂的蓝图逻辑或特效）
    // 如果为空，则使用默认的 AWonder 类
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AWonder> ActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MeshScale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeightOffset = 0.0f;

    // 是否需要把地块夷平？（奇观通常需要平地）
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFlattenTerrain = true;
};

UCLASS(BlueprintType)
class CIVI_API UWonderDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wonder Data")
    TArray<FWonderDisplayData> WonderData;

    UFUNCTION(BlueprintCallable, Category = "Wonder Data")
    FWonderDisplayData GetWonderDisplayData(EWonderType Type) const;
};
