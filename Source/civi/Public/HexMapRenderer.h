// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CiviTypes.h"
#include "BuildingDataAsset.h"
#include "WonderDataAsset.h"
#include "Wonder.h"
#include "HexMapRenderer.generated.h"

class UTerrainDataAsset;
class ULandblock;
class UInstancedStaticMeshComponent;
class UHierarchicalInstancedStaticMeshComponent;

/**
 * 六边形地图渲染器
 * 负责将逻辑地图数据转换为可视化的3D地图
 */

class CIVI_API UTerraindataasset;

UCLASS()
class CIVI_API AHexMapRenderer : public AActor
{
    GENERATED_BODY()

public:
    AHexMapRenderer();

    // 地形数据资产引用（在编辑器中设置）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Renderer")
    UTerraindataasset* TerrainDataAsset;

    // 六边形尺寸（外接圆半径）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Renderer")
    float HexRadius = 100.0f;

    // 地块之间的间隙
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Renderer")
    float HexGap = 2.0f;

    // 使用实例化渲染（性能优化）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Renderer")
    bool bUseInstancing = true;

    // 渲染整个地图
    UFUNCTION(BlueprintCallable, Category = "Map Renderer")
    void RenderMap(const TArray<ULandblock*>& MapGrid, int32 MapWidth, int32 MapHeight);

    // 清除已渲染的地图
    UFUNCTION(BlueprintCallable, Category = "Map Renderer")
    void ClearMap();

    // 更新单个地块的显示
    UFUNCTION(BlueprintCallable, Category = "Map Renderer")
    void UpdateTile(ULandblock* Landblock);

    // 新增：建筑数据资产
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Renderer")
    UBuildingDataAsset* BuildingDataAsset;

    // 新增：奇观数据资产
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Renderer")
    UWonderDataAsset* WonderDataAsset;

protected:
    virtual void BeginPlay() override;

private:
    // 根节点组件
    UPROPERTY()
    USceneComponent* RootSceneComponent;

    // 实例化网格组件（按地形类型分组）
    UPROPERTY()
    TMap<ETerrain, UHierarchicalInstancedStaticMeshComponent*> TerrainMeshComponents;

    // 地貌实例化网格组件
    UPROPERTY()
    TMap<ELandform, UHierarchicalInstancedStaticMeshComponent*> LandformMeshComponents;

    // 存储地块对应的Actor（非实例化模式）
    UPROPERTY()
    TArray<AActor*> SpawnedTileActors;

    // 计算六边形世界坐标
    FVector CalculateHexWorldPosition(int32 X, int32 Y) const;

    // 创建单个地块（非实例化模式）
    AActor* SpawnTileActor(ULandblock* Landblock, const FVector& Position);

    // 获取或创建实例化网格组件
    UHierarchicalInstancedStaticMeshComponent* GetOrCreateTerrainMeshComponent(ETerrain TerrainType);
    UHierarchicalInstancedStaticMeshComponent* GetOrCreateLandformMeshComponent(ELandform LandformType);

    // 新增：建筑实例化组件映射
    UPROPERTY()
    TMap<EBuildingType, UHierarchicalInstancedStaticMeshComponent*> BuildingMeshComponents;

    // 新增：获取或创建建筑组件
    UHierarchicalInstancedStaticMeshComponent* GetOrCreateBuildingMeshComponent(EBuildingType BuildingType);

    // 新增：存储生成的奇观 Actor 引用，以便清除地图时销毁
    UPROPERTY()
    TArray<AWonder*> SpawnedWonderActors;

    // 辅助函数：生成奇观
    void SpawnWonder(ULandblock* Block, const FVector& Position);
};


