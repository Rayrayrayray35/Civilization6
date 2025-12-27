// Fill out your copyright notice in the Description page of Project Settings.

#include "Landblock.h"
#include "TerrainDataAsset.h"
#include "BuildingDataAsset.h"
#include "Building.h"

ULandblock::ULandblock()
{
    X = 0;
    Y = 0;
    Terrain = ETerrain::Plain;
    Landform = ELandform::None;
    Building = nullptr;
}

void ULandblock::InitNeighbors()
{
    Neighbors.SetNum(6);
    for (int32 i = 0; i < 6; i++)
    {
        Neighbors[i] = nullptr;
    }
}

void ULandblock::SetNeighbor(int32 Direction, ULandblock* Neighbor)
{
    if (Direction >= 0 && Direction < 6)
    {
        Neighbors[Direction] = Neighbor;
    }
}

ULandblock* ULandblock::GetNeighbor(int32 Direction) const
{
    if (Direction >= 0 && Direction < 6 && Direction < Neighbors.Num())
    {
        return Neighbors[Direction];
    }
    return nullptr;
}

ULandblock* ULandblock::GetNeighborByDirection(EHexDirection Direction) const
{
    return GetNeighbor(static_cast<int32>(Direction));
}

bool ULandblock::IsWater() const
{
    return Terrain == ETerrain::Ocean || Terrain == ETerrain::Coast;
}

bool ULandblock::IsPassable() const
{
    // 山脉不可通行 (除非有隧道技术)
    if (Landform == ELandform::Mountain)
    {
        return false;
    }

    // 深海默认不可通行 (除非有航海技术)
    if (Terrain == ETerrain::Ocean)
    {
        return false;
    }

    return true;
}

int32 ULandblock::GetMovementCost() const
{
    int32 BaseCost = 1;

    // 地形基础消耗
    switch (Terrain)
    {
    case ETerrain::Ocean:
    case ETerrain::Coast:
        BaseCost = 1;
        break;
    case ETerrain::Plain:
    case ETerrain::Grassland:
        BaseCost = 1;
        break;
    case ETerrain::Desert:
    case ETerrain::Tundra:
    case ETerrain::Snow:
        BaseCost = 1;
        break;
    default:
        BaseCost = 1;
        break;
    }

    // 地貌附加消耗
    switch (Landform)
    {
    case ELandform::Hills:
        BaseCost += 1;
        break;
    case ELandform::Mountain:
        BaseCost = 999; // 不可通行
        break;
    case ELandform::Forest:
    case ELandform::Rainforest:
        BaseCost += 1;
        break;
    case ELandform::Marsh:
        BaseCost += 2;
        break;
    default:
        break;
    }

    return BaseCost;
}

void ULandblock::ConstructBuilding(EBuildingType NewBuildingType)
{
    if (NewBuildingType == EBuildingType::None)
    {
        Building = nullptr;
        return;
    }

    // 创建新的建筑逻辑对象
    Building = NewObject<UBuilding>(this);
    Building->Type = NewBuildingType;
}

void ULandblock::SetWonder(EWonderType NewWonderType)
{
    WonderType = NewWonderType;
    // 这里可以添加逻辑：如果奇观是排他的，检查全局唯一性等
}

FYields ULandblock::GetTotalYield(const UTerraindataasset* TData, const UBuildingDataAsset* BData) const
{
    FYields TotalYield;

    if (!TData) return TotalYield;

    // 1. 地形基础产出
    FTerrainDisplayData TerrainInfo = TData->GetTerrainDisplayData(Terrain);
    TotalYield = TotalYield + TerrainInfo.BaseYields;

    // 2. 地貌额外产出
    if (Landform != ELandform::None)
    {
        FLandformDisplayData LandformInfo = TData->GetLandformDisplayData(Landform);
        TotalYield = TotalYield + LandformInfo.ExtraYields;
    }

    // 3. 建筑产出与维护费
    if (Building && Building->Type != EBuildingType::None && BData)
    {
        FBuildingDisplayData BuildInfo = BData->GetBuildingDisplayData(Building->Type);

        // 加上建筑产出
        TotalYield = TotalYield + BuildInfo.BonusYields;

        // 扣除维护费 (从产出的金币中扣除，如果金币不足则为负，由国库承担)
        TotalYield.Gold -= BuildInfo.MaintenanceCost;
    }

    return TotalYield;
}

EVisibilityState ULandblock::GetVisibility(int32 PlayerIndex) const
{
    if (const EVisibilityState* State = PlayerVisibility.Find(PlayerIndex))
    {
        return *State;
    }
    // 默认为未探索
    return EVisibilityState::Unexplored;
}

void ULandblock::SetVisibility(int32 PlayerIndex, EVisibilityState NewState)
{
    PlayerVisibility.FindOrAdd(PlayerIndex) = NewState;
}