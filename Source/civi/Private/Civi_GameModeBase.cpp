// Fill out your copyright notice in the Description page of Project Settings.


#include "Civi_GameModeBase.h"
#include "Landblock.h"

ACivi_GameModeBase::ACivi_GameModeBase()
{
    // 初始化排列表
    Permutation.SetNum(512);
}

void ACivi_GameModeBase::BeginPlay()
{
    Super::BeginPlay();

    // 游戏开始时初始化地图
    InitMap();
}

void ACivi_GameModeBase::InitMap()
{
    // 1. 生成随机种子
    if (MapSeed == 0)
    {
        MapSeed = FMath::Rand();
    }
    FMath::SRandInit(MapSeed);

    UE_LOG(LogTemp, Log, TEXT("Initializing map with seed: %d, Size: %dx%d"), MapSeed, MapWidth, MapHeight);

    // 2. 初始化柏林噪声排列表
    InitPermutation();

    // 3. 清空并初始化地图网格
    MapGrid.Empty();
    MapGrid.SetNum(MapWidth * MapHeight);

    // 4. 生成各种噪声图
    TArray<float> ElevationMap;
    TArray<float> MoistureMap;
    TArray<float> TemperatureMap;

    GenerateElevationMap(ElevationMap);
    GenerateMoistureMap(MoistureMap);
    GenerateTemperatureMap(TemperatureMap);

    // 5. 创建地块并分配地形/地貌
    for (int32 Y = 0; Y < MapHeight; Y++)
    {
        for (int32 X = 0; X < MapWidth; X++)
        {
            int32 Index = GetIndex(X, Y);

            // 创建新地块
            ULandblock* NewBlock = NewObject<ULandblock>(this, ULandblock::StaticClass());
            NewBlock->X = X;
            NewBlock->Y = Y;
            NewBlock->InitNeighbors();

            // 获取该位置的噪声值
            float Elevation = ElevationMap[Index];
            float Moisture = MoistureMap[Index];
            float Temperature = TemperatureMap[Index];

            // 确定地形和地貌
            NewBlock->Terrain = DetermineTerrain(Elevation, Moisture, Temperature, Y);
            NewBlock->Landform = DetermineLandform(NewBlock->Terrain, Elevation, Moisture, Temperature);

            MapGrid[Index] = NewBlock;
        }
    }

    // 6. 链接邻居关系
    LinkNeighbors();

    UE_LOG(LogTemp, Log, TEXT("Map initialization complete. Total tiles: %d"), MapGrid.Num());
}

//==============================
// 柏林噪声实现
//==============================

void ACivi_GameModeBase::InitPermutation()
{
    // 创建0-255的排列
    TArray<int32> P;
    P.SetNum(256);
    for (int32 i = 0; i < 256; i++)
    {
        P[i] = i;
    }

    // Fisher-Yates 洗牌算法
    for (int32 i = 255; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        int32 Temp = P[i];
        P[i] = P[j];
        P[j] = Temp;
    }

    // 复制一份以避免溢出
    for (int32 i = 0; i < 256; i++)
    {
        Permutation[i] = P[i];
        Permutation[256 + i] = P[i];
    }
}

float ACivi_GameModeBase::Fade(float T)
{
    // 6t^5 - 15t^4 + 10t^3 (改进的缓动函数)
    return T * T * T * (T * (T * 6.0f - 15.0f) + 10.0f);
}

float ACivi_GameModeBase::Lerp(float A, float B, float T)
{
    return A + T * (B - A);
}

float ACivi_GameModeBase::Grad(int32 Hash, float X, float Y)
{
    // 使用哈希值的低位来选择梯度方向
    int32 H = Hash & 3;
    float U = (H < 2) ? X : Y;
    float V = (H < 2) ? Y : X;
    return ((H & 1) ? -U : U) + ((H & 2) ? -2.0f * V : 2.0f * V);
}

float ACivi_GameModeBase::PerlinNoise2D(float X, float Y)
{
    // 计算网格单元坐标
    int32 Xi = FMath::FloorToInt(X) & 255;
    int32 Yi = FMath::FloorToInt(Y) & 255;

    // 计算单元内的相对坐标
    float Xf = X - FMath::FloorToFloat(X);
    float Yf = Y - FMath::FloorToFloat(Y);

    // 计算缓动曲线
    float U = Fade(Xf);
    float V = Fade(Yf);

    // 获取四个角的哈希值
    int32 AA = Permutation[Permutation[Xi] + Yi];
    int32 AB = Permutation[Permutation[Xi] + Yi + 1];
    int32 BA = Permutation[Permutation[Xi + 1] + Yi];
    int32 BB = Permutation[Permutation[Xi + 1] + Yi + 1];

    // 计算梯度点积并插值
    float X1 = Lerp(Grad(AA, Xf, Yf), Grad(BA, Xf - 1.0f, Yf), U);
    float X2 = Lerp(Grad(AB, Xf, Yf - 1.0f), Grad(BB, Xf - 1.0f, Yf - 1.0f), U);

    // 返回值范围约为 [-1, 1]，归一化到 [0, 1]
    return (Lerp(X1, X2, V) + 1.0f) * 0.5f;
}

float ACivi_GameModeBase::OctavePerlinNoise(float X, float Y, int32 NumOctaves, float InPersistence, float InLacunarity)
{
    float Total = 0.0f;
    float Frequency = 1.0f;
    float Amplitude = 1.0f;
    float MaxValue = 0.0f;

    for (int32 i = 0; i < NumOctaves; i++)
    {
        Total += PerlinNoise2D(X * Frequency, Y * Frequency) * Amplitude;
        MaxValue += Amplitude;
        Amplitude *= InPersistence;
        Frequency *= InLacunarity;
    }

    return Total / MaxValue;
}

//==============================
// 地图生成
//==============================

void ACivi_GameModeBase::GenerateElevationMap(TArray<float>& OutElevation)
{
    OutElevation.SetNum(MapWidth * MapHeight);

    // 使用不同的偏移量来产生不同的噪声图
    float OffsetX = FMath::SRand() * 10000.0f;
    float OffsetY = FMath::SRand() * 10000.0f;

    for (int32 Y = 0; Y < MapHeight; Y++)
    {
        for (int32 X = 0; X < MapWidth; X++)
        {
            float NX = (float)X * ElevationScale + OffsetX;
            float NY = (float)Y * ElevationScale + OffsetY;

            float Elevation = OctavePerlinNoise(NX, NY, Octaves, Persistence, Lacunarity);

            // 边缘渐变 - 让地图边缘更可能是海洋
            float EdgeFactorX = 1.0f - FMath::Pow(FMath::Abs((float)X / MapWidth - 0.5f) * 2.0f, 2.0f);
            float EdgeFactorY = 1.0f - FMath::Pow(FMath::Abs((float)Y / MapHeight - 0.5f) * 2.0f, 2.0f);
            float EdgeFactor = EdgeFactorX * EdgeFactorY;

            // 混合噪声和边缘因子
            Elevation = Elevation * 0.7f + EdgeFactor * 0.3f;

            OutElevation[GetIndex(X, Y)] = FMath::Clamp(Elevation, 0.0f, 1.0f);
        }
    }
}

void ACivi_GameModeBase::GenerateMoistureMap(TArray<float>& OutMoisture)
{
    OutMoisture.SetNum(MapWidth * MapHeight);

    float OffsetX = FMath::SRand() * 10000.0f;
    float OffsetY = FMath::SRand() * 10000.0f;

    for (int32 Y = 0; Y < MapHeight; Y++)
    {
        for (int32 X = 0; X < MapWidth; X++)
        {
            float NX = (float)X * MoistureScale + OffsetX;
            float NY = (float)Y * MoistureScale + OffsetY;

            float Moisture = OctavePerlinNoise(NX, NY, Octaves - 1, Persistence, Lacunarity);
            OutMoisture[GetIndex(X, Y)] = FMath::Clamp(Moisture, 0.0f, 1.0f);
        }
    }
}

void ACivi_GameModeBase::GenerateTemperatureMap(TArray<float>& OutTemperature)
{
    OutTemperature.SetNum(MapWidth * MapHeight);

    float OffsetX = FMath::SRand() * 10000.0f;
    float OffsetY = FMath::SRand() * 10000.0f;

    for (int32 Y = 0; Y < MapHeight; Y++)
    {
        for (int32 X = 0; X < MapWidth; X++)
        {
            // 基础温度根据纬度计算 (赤道热，极地冷)
            float LatitudeFactor = 1.0f - FMath::Abs((float)Y / MapHeight - 0.5f) * 2.0f;

            // 添加一些噪声变化
            float NX = (float)X * TemperatureScale + OffsetX;
            float NY = (float)Y * TemperatureScale + OffsetY;
            float NoiseVariation = OctavePerlinNoise(NX, NY, 2, 0.5f, 2.0f) * 0.3f;

            float Temperature = LatitudeFactor * 0.7f + NoiseVariation + 0.15f;
            OutTemperature[GetIndex(X, Y)] = FMath::Clamp(Temperature, 0.0f, 1.0f);
        }
    }
}

ETerrain ACivi_GameModeBase::DetermineTerrain(float Elevation, float Moisture, float Temperature, int32 Y)
{
    // 深海
    if (Elevation < 0.3f)
    {
        return ETerrain::Ocean;
    }
    // 浅海/沿海
    if (Elevation < 0.4f)
    {
        return ETerrain::Coast;
    }

    // 陆地地形根据温度和湿度决定

    // 极地地区 (冷)
    if (Temperature < 0.15f)
    {
        return ETerrain::Snow;
    }

    // 近极地 (较冷)
    if (Temperature < 0.3f)
    {
        return ETerrain::Tundra;
    }

    // 沙漠 (干燥且温暖)
    if (Moisture < 0.25f && Temperature > 0.5f)
    {
        return ETerrain::Desert;
    }

    // 草原 (中等湿度)
    if (Moisture < 0.5f)
    {
        return ETerrain::Grassland;
    }

    // 平原 (湿润)
    return ETerrain::Plain;
}

ELandform ACivi_GameModeBase::DetermineLandform(ETerrain Terrain, float Elevation, float Moisture, float Temperature)
{
    // 水域没有地貌
    if (Terrain == ETerrain::Ocean || Terrain == ETerrain::Coast)
    {
        return ELandform::None;
    }

    // 高海拔 - 山脉
    if (Elevation > 0.85f)
    {
        return ELandform::Mountain;
    }

    // 较高海拔 - 丘陵
    if (Elevation > 0.7f)
    {
        return ELandform::Hills;
    }

    // 冰雪地区可能有冰川
    if (Terrain == ETerrain::Snow && FMath::SRand() < 0.3f)
    {
        return ELandform::Ice;
    }

    // 沙漠中可能有绿洲
    if (Terrain == ETerrain::Desert && Moisture > 0.3f && FMath::SRand() < 0.05f)
    {
        return ELandform::Oasis;
    }

    // 温暖湿润地区 - 森林或雨林
    if (Moisture > 0.6f)
    {
        // 热带 - 雨林
        if (Temperature > 0.7f)
        {
            if (FMath::SRand() < 0.6f)
            {
                return ELandform::Rainforest;
            }
        }
        // 温带 - 森林
        else if (Temperature > 0.35f)
        {
            if (FMath::SRand() < 0.5f)
            {
                return ELandform::Forest;
            }
        }
    }

    // 湿地 - 沼泽
    if (Moisture > 0.7f && Temperature > 0.4f && Elevation < 0.5f)
    {
        if (FMath::SRand() < 0.2f)
        {
            return ELandform::Marsh;
        }
    }

    // 中等湿度可能有森林
    if (Moisture > 0.4f && Temperature > 0.3f && Temperature < 0.8f)
    {
        if (FMath::SRand() < 0.25f)
        {
            return ELandform::Forest;
        }
    }

    return ELandform::None;
}

//==============================
// 六边形邻居系统
//==============================

void ACivi_GameModeBase::GetHexNeighborOffsets(int32 Y, TArray<FIntPoint>& OutOffsets) const
{
    OutOffsets.SetNum(6);

    // 偏移量顺序: 东北(0), 东(1), 东南(2), 西南(3), 西(4), 西北(5)
    // 对于偶数行和奇数行，六边形的邻居偏移不同

    if (Y % 2 == 0) // 偶数行
    {
        OutOffsets[0] = FIntPoint(0, -1);   // 东北
        OutOffsets[1] = FIntPoint(1, 0);    // 东
        OutOffsets[2] = FIntPoint(0, 1);    // 东南
        OutOffsets[3] = FIntPoint(-1, 1);   // 西南
        OutOffsets[4] = FIntPoint(-1, 0);   // 西
        OutOffsets[5] = FIntPoint(-1, -1);  // 西北
    }
    else // 奇数行
    {
        OutOffsets[0] = FIntPoint(1, -1);   // 东北
        OutOffsets[1] = FIntPoint(1, 0);    // 东
        OutOffsets[2] = FIntPoint(1, 1);    // 东南
        OutOffsets[3] = FIntPoint(0, 1);    // 西南
        OutOffsets[4] = FIntPoint(-1, 0);   // 西
        OutOffsets[5] = FIntPoint(0, -1);   // 西北
    }
}

void ACivi_GameModeBase::LinkNeighbors()
{
    for (int32 Y = 0; Y < MapHeight; Y++)
    {
        for (int32 X = 0; X < MapWidth; X++)
        {
            ULandblock* CurrentBlock = GetLandblock(X, Y);
            if (!CurrentBlock) continue;

            TArray<FIntPoint> Offsets;
            GetHexNeighborOffsets(Y, Offsets);

            for (int32 Dir = 0; Dir < 6; Dir++)
            {
                int32 NX = X + Offsets[Dir].X;
                int32 NY = Y + Offsets[Dir].Y;

                // 处理水平方向的环绕 (文明6风格的圆柱形地图)
                if (NX < 0) NX = MapWidth - 1;
                if (NX >= MapWidth) NX = 0;

                // 垂直方向不环绕
                if (NY >= 0 && NY < MapHeight)
                {
                    ULandblock* Neighbor = GetLandblock(NX, NY);
                    CurrentBlock->SetNeighbor(Dir, Neighbor);
                }
            }
        }
    }
}

int32 ACivi_GameModeBase::GetIndex(int32 X, int32 Y) const
{
    return Y * MapWidth + X;
}

ULandblock* ACivi_GameModeBase::GetLandblock(int32 X, int32 Y) const
{
    if (X < 0 || X >= MapWidth || Y < 0 || Y >= MapHeight)
    {
        return nullptr;
    }

    int32 Index = GetIndex(X, Y);
    if (Index >= 0 && Index < MapGrid.Num())
    {
        return MapGrid[Index];
    }

    return nullptr;
}

