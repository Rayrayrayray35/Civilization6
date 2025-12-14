// Fill out your copyright notice in the Description page of Project Settings.

#include "WonderDataAsset.h"

FWonderDisplayData UWonderDataAsset::GetWonderDisplayData(EWonderType Type) const
{
    for (const FWonderDisplayData& Data : WonderData)
    {
        if (Data.WonderType == Type)
        {
            return Data;
        }
    }
    return FWonderDisplayData();
}