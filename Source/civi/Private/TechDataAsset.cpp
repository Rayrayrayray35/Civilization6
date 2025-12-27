// Fill out your copyright notice in the Description page of Project Settings.

#include "TechDataAsset.h"

FTechInfo UTechDataAsset::GetTechInfo(ETechType Type) const
{
    for (const FTechInfo& Info : Techs)
    {
        if (Info.Type == Type) return Info;
    }
    return FTechInfo();
}