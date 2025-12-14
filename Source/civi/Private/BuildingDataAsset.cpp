// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildingDataAsset.h"

FBuildingDisplayData UBuildingDataAsset::GetBuildingDisplayData(EBuildingType Type) const
{
    for (const FBuildingDisplayData& Data : BuildingData)
    {
        if (Data.BuildingType == Type)
        {
            return Data;
        }
    }
    return FBuildingDisplayData();
}