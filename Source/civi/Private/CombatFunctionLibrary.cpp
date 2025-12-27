// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatFunctionLibrary.h"
#include "Landblock.h"

int32 UCombatFunctionLibrary::CalculateDamage(int32 AttackerStrength, int32 DefenderStrength)
{
    int32 StrengthDiff = AttackerStrength - DefenderStrength;

    // 基础伤害 30
    float BaseDamage = 30.0f;

    // 指数修正: 每高 1 点战力，伤害增加约 4%
    // 战力差 +30 点时，伤害约为 100 (秒杀)
    // 战力差 -30 点时，伤害约为 10
    float Multiplier = FMath::Exp(0.04f * StrengthDiff);

    // 随机浮动 +/- 15%
    float RandomFactor = FMath::RandRange(0.85f, 1.15f);

    int32 FinalDamage = FMath::RoundToInt(BaseDamage * Multiplier * RandomFactor);

    // 保证至少造成 1 点伤害，且上限一般不超过 100 (除非溢出)
    return FMath::Clamp(FinalDamage, 1, 200);
}

int32 UCombatFunctionLibrary::GetTerrainDefenseBonus(ULandblock* Block)
{
    if (!Block) return 0;

    int32 Bonus = 0;

    // 地貌加成
    switch (Block->Landform)
    {
        case ELandform::Hills:
            Bonus += 3;
            break;
        case ELandform::Forest:
        case ELandform::Rainforest:
            Bonus += 3; // 森林/雨林提供防御
            break;
        case ELandform::Marsh:
            Bonus -= 2; // 沼泽减少防御
            break;
        default:
            break;
    }

    // 地形加成 (可选)
    // if (Block->Terrain == ETerrain::Coast) ...

    return Bonus;
}