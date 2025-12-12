// Fill out your copyright notice in the Description page of Project Settings.


#include "Landblock.h"

ULandblock::ULandblock()
{
    X = 0;
    Y = 0;
    Terrain = ETerrain::Plain;
    Landform = ELandform::None;
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