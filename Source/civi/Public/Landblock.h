// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Civi_GameModeBase.h"
#include "CiviTypes.h"
#include "Landblock.generated.h"

class UBuilding;
class UTerraindataasset;
class ACity;
class AUnit;
class UBuildingDataAsset;

UCLASS(BlueprintType)
class CIVI_API ULandblock : public UObject
{
    GENERATED_BODY()

public:
    ULandblock();

    // 地块坐标
    UPROPERTY(BlueprintReadOnly, Category = "Landblock")
    int32 X;

    UPROPERTY(BlueprintReadOnly, Category = "Landblock")
    int32 Y;

    // 地形类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landblock")
    ETerrain Terrain;

    // 地貌类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landblock")
    ELandform Landform;

    // 周围6个邻居 (六边形地图)
    // 顺序: 东北(0), 东(1), 东南(2), 西南(3), 西(4), 西北(5)
    UPROPERTY(BlueprintReadOnly, Category = "Landblock")
    TArray<ULandblock*> Neighbors;

    // 初始化邻居数组
    void InitNeighbors();

    // 设置邻居
    void SetNeighbor(int32 Direction, ULandblock* Neighbor);

    // 获取邻居
    UFUNCTION(BlueprintCallable, Category = "Landblock")
    ULandblock* GetNeighbor(int32 Direction) const;

    UFUNCTION(BlueprintCallable, Category = "Landblock")
    ULandblock* GetNeighborByDirection(EHexDirection Direction) const;

    // 是否为水域
    UFUNCTION(BlueprintCallable, Category = "Landblock")
    bool IsWater() const;

    // 是否可通行
    UFUNCTION(BlueprintCallable, Category = "Landblock")
    bool IsPassable() const;

    // 获取移动消耗
    UFUNCTION(BlueprintCallable, Category = "Landblock")
    int32 GetMovementCost() const;

    // 当前地块上的建筑
    UPROPERTY(BlueprintReadOnly, Category = "Development")
    UBuilding* Building;

    // 辅助函数：建造建筑
    UFUNCTION(BlueprintCallable, Category = "Development")
    void ConstructBuilding(EBuildingType NewBuildingType);

    // 该地块的奇观类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Development")
    EWonderType WonderType;

    // 辅助：设置奇观
    UFUNCTION(BlueprintCallable, Category = "Development")
    void SetWonder(EWonderType NewWonderType);

    // 获取该地块的总产出（地形 + 地貌 + 建筑）
    // 需要传入 DataAsset 来获取基础配置数据
    // 传入 BuildingData 以计算建筑加成和维护费
    UFUNCTION(BlueprintCallable, Category = "Economy")
    FYields GetTotalYield(const UTerraindataasset* TData, const UBuildingDataAsset* BData) const;

    // --- 视野系统 ---

    // 存储每个玩家对该地块的视野状态 (Key: PlayerIndex, Value: State)
    UPROPERTY(BlueprintReadOnly, Category = "FogOfWar")
    TMap<int32, EVisibilityState> PlayerVisibility;

    // 获取指定玩家的视野状态
    UFUNCTION(BlueprintCallable, Category = "FogOfWar")
    EVisibilityState GetVisibility(int32 PlayerIndex) const;

    // 设置指定玩家的视野状态
    UFUNCTION(BlueprintCallable, Category = "FogOfWar")
    void SetVisibility(int32 PlayerIndex, EVisibilityState NewState);

    // 该地块所属的城市 (如果为空则表示无主之地)
    UPROPERTY(BlueprintReadOnly, Category = "City")
    ACity* OwningCity = nullptr;

    // 辅助函数：设置所有者
    void SetOwningCity(ACity* NewCity) { OwningCity = NewCity; }

    // 当前地块上的战斗单位（假设每个格子只能有一个战斗单位）
    UPROPERTY(BlueprintReadOnly, Category = "Unit")
    AUnit* OccupyingUnit = nullptr;

    // 辅助：是否有单位
    bool HasUnit() const { return OccupyingUnit != nullptr; }

};
