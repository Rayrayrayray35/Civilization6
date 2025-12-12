// TerrainDataAsset.cpp
#include "TerrainDataAsset.h"

FTerrainDisplayData UTerraindataasset::GetTerrainDisplayData(ETerrain TerrainType) const
{
    for (const FTerrainDisplayData& Data : TerrainData)
    {
        if (Data.TerrainType == TerrainType)
        {
            return Data;
        }
    }

    // 返回默认值
    return FTerrainDisplayData();
}

FLandformDisplayData UTerraindataasset::GetLandformDisplayData(ELandform LandformType) const
{
    for (const FLandformDisplayData& Data : LandformData)
    {
        if (Data.LandformType == LandformType)
        {
            return Data;
        }
    }

    // 返回默认值
    return FLandformDisplayData();
}