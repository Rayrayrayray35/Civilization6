// Fill out your copyright notice in the Description page of Project Settings.

#include "CivicDataAsset.h"

FCivicInfo UCivicDataAsset::GetCivicInfo(ECivicType Type) const
{
    for (const FCivicInfo& Info : Civics)
    {
        if (Info.Type == Type) return Info;
    }
    return FCivicInfo();
}