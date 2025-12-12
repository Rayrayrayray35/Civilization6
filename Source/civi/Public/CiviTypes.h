// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// 地形类型枚举
UENUM(BlueprintType)
enum class ETerrain : uint8
{
    Ocean       UMETA(DisplayName = "海洋"),
    Coast       UMETA(DisplayName = "沿海"),
    Plain       UMETA(DisplayName = "平原"),
    Grassland   UMETA(DisplayName = "草原"),
    Desert      UMETA(DisplayName = "沙漠"),
    Tundra      UMETA(DisplayName = "冻土"),
    Snow        UMETA(DisplayName = "雪地")
};

// 地貌类型枚举
UENUM(BlueprintType)
enum class ELandform : uint8
{
    None        UMETA(DisplayName = "无"),
    Hills       UMETA(DisplayName = "丘陵"),
    Mountain    UMETA(DisplayName = "山脉"),
    Forest      UMETA(DisplayName = "森林"),
    Rainforest  UMETA(DisplayName = "雨林"),
    Marsh       UMETA(DisplayName = "沼泽"),
    Oasis       UMETA(DisplayName = "绿洲"),
    Ice         UMETA(DisplayName = "冰川")
};

// 六边形方向枚举
UENUM(BlueprintType)
enum class EHexDirection : uint8
{
    NorthEast = 0   UMETA(DisplayName = "东北"),
    East = 1        UMETA(DisplayName = "东"),
    SouthEast = 2   UMETA(DisplayName = "东南"),
    SouthWest = 3   UMETA(DisplayName = "西南"),
    West = 4        UMETA(DisplayName = "西"),
    NorthWest = 5   UMETA(DisplayName = "西北")
};