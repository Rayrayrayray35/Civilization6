// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Civitypes.h"
#include "TerrainDataAsset.generated.h"

// 单个地形的显示配置
USTRUCT(BlueprintType)
struct FTerrainDisplayData
{
    GENERATED_BODY()

    // 地形类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    ETerrain TerrainType;

    // 地形名称（用于UI显示）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FText DisplayName;

    // 基础材质
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    UMaterialInterface* BaseMaterial;

    // 地形颜色（用于小地图或简单显示）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FLinearColor MinimapColor;

    // 是否可通行
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsPassable = true;

    // 移动消耗
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 MovementCost = 1;

    FTerrainDisplayData()
        : TerrainType(ETerrain::Plain)
        , DisplayName(FText::FromString(TEXT("平原")))
        , BaseMaterial(nullptr)
        , MinimapColor(FLinearColor::Green)
        , bIsPassable(true)
        , MovementCost(1)
    {
    }
};

// 单个地貌的显示配置
USTRUCT(BlueprintType)
struct FLandformDisplayData
{
    GENERATED_BODY()

    // 地貌类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landform")
    ELandform LandformType;

    // 地貌名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landform")
    FText DisplayName;

    // 地貌使用的静态网格（如树木、山脉模型）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landform")
    UStaticMesh* Mesh;

    // 地貌材质（如果需要覆盖基础地形材质）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landform")
    UMaterialInterface* OverlayMaterial;

    // 网格缩放
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landform")
    FVector MeshScale = FVector(1.0f);

    // 网格高度偏移
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landform")
    float HeightOffset = 0.0f;

    // 是否随机旋转
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landform")
    bool bRandomRotation = true;

    // 额外移动消耗
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landform")
    int32 ExtraMovementCost = 0;

    FLandformDisplayData()
        : LandformType(ELandform::None)
        , DisplayName(FText::FromString(TEXT("无")))
        , Mesh(nullptr)
        , OverlayMaterial(nullptr)
        , MeshScale(FVector(1.0f))
        , HeightOffset(0.0f)
        , bRandomRotation(true)
        , ExtraMovementCost(0)
    {
    }
};

/**
 * 地形数据资产 - 在编辑器中配置所有地形和地貌的显示属性
 */
UCLASS(BlueprintType)
class CIVI_API UTerraindataasset : public UDataAsset
{
    GENERATED_BODY()

public:
    // 所有地形配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Data")
    TArray<FTerrainDisplayData> TerrainData;

    // 所有地貌配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landform Data")
    TArray<FLandformDisplayData> LandformData;

    // 六边形基础网格
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Settings")
    UStaticMesh* HexBaseMesh;

    // 六边形大小（外接圆半径）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Settings")
    float HexSize = 100.0f;

    // 根据地形类型获取显示数据
    UFUNCTION(BlueprintCallable, Category = "Terrain Data")
    FTerrainDisplayData GetTerrainDisplayData(ETerrain TerrainType) const;

    // 根据地貌类型获取显示数据
    UFUNCTION(BlueprintCallable, Category = "Terrain Data")
    FLandformDisplayData GetLandformDisplayData(ELandform LandformType) const;
};
