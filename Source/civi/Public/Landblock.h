// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Civi_GameModeBase.h"
#include "Civitypes.h"
#include "Landblock.generated.h"

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
};
