// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitDataAsset.h"

FUnitInfo UUnitDataAsset::GetUnitInfo(ECiviUnitType Type) const
{
    for (const FUnitInfo& Info : Units)
    {
        if (Info.UnitType == Type) return Info;
    }
    return FUnitInfo();
}