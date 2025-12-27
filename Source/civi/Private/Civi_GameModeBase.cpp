// Fill out your copyright notice in the Description page of Project Settings.


#include "Civi_GameModeBase.h"
#include "Landblock.h"
#include "EngineUtils.h"
#include "City.h"
#include "Unit.h"
#include "HexMapRenderer.h"
#include "Kismet/GameplayStatics.h"

ACivi_GameModeBase::ACivi_GameModeBase()
{
    // 初始化排列表
    Permutation.SetNum(512);
}

void ACivi_GameModeBase::BeginPlay()
{
    Super::BeginPlay();

    // 初始化资源数组
    PlayerGlobalYields.SetNumZeroed(TotalPlayers);
    // 给予初始资金 (例如 100 金币)
    for (int32 i = 0; i < TotalPlayers; i++)
    {
        PlayerGlobalYields[i].Gold = 100;
    }

    // 初始化研发状态
    InitResearch();

    // 1. 初始化地图
    InitMap();

    // 2. 尝试找到场景中的渲染器并绘制地图 (如果场景里放了 BP_HexMapRenderer)
    for (TActorIterator<AHexMapRenderer> It(GetWorld()); It; ++It)
    {
        AHexMapRenderer* Renderer = *It;
        if (Renderer)
        {
            // 确保渲染器有数据引用
            Renderer->RenderMap(MapGrid, MapWidth, MapHeight);
            break;
        }
    }

    // 3. 启动游戏第一回合
    CurrentTurn = 1;
    CurrentPlayerIndex = 0;
    StartTurn();
}

void ACivi_GameModeBase::EndTurn()
{
    if (bIsGameOver) return; // 游戏结束无法操作

    UE_LOG(LogTemp, Log, TEXT("Player %d ending turn %d"), CurrentPlayerIndex, CurrentTurn);

    // 1. 结算当前玩家的结束阶段 (城市产出、科研等)
    ProcessTurnEndForPlayer(CurrentPlayerIndex);

    CheckVictoryConditions(); // 检测科技胜利等

    // 2. 切换到下一个玩家
    CurrentPlayerIndex++;

    // 如果超过总玩家数，重置为第一个玩家，并增加回合数
    if (CurrentPlayerIndex >= TotalPlayers)
    {
        CurrentPlayerIndex = 0;
        CurrentTurn++;
        UE_LOG(LogTemp, Log, TEXT("--- New Turn: %d ---"), CurrentTurn);
    }

    // 3. 开始新玩家的行动阶段
    StartTurn();
}

void ACivi_GameModeBase::StartTurn()
{
    UE_LOG(LogTemp, Log, TEXT("Start turn for Player %d"), CurrentPlayerIndex);

    // 1. 处理单位重置 (恢复移动力)
    ProcessTurnStartForPlayer(CurrentPlayerIndex);

    // 2. 更新战争迷雾 (只显示当前玩家的视野)
    UpdateFogOfWar();

    // 3. (可选) 通知 UI 更新
    // OnTurnChanged.Broadcast(CurrentPlayerIndex, CurrentTurn); 
}

void ACivi_GameModeBase::ProcessTurnStartForPlayer(int32 PlayerIndex)
{
    // 遍历所有单位，重置属于该玩家的单位状态
    for (TActorIterator<AUnit> It(GetWorld()); It; ++It)
    {
        AUnit* Unit = *It;
        if (Unit && Unit->PlayerOwnerIndex == PlayerIndex)
        {
            Unit->OnTurnStart(); // 调用 Unit 类的回合开始函数 (重置移动力、驻守回血)
        }
    }
}

void ACivi_GameModeBase::ProcessTurnEndForPlayer(int32 PlayerIndex)
{
    FYields TotalTurnIncome;

    // 遍历该玩家的所有城市进行结算
    for (TActorIterator<ACity> It(GetWorld()); It; ++It)
    {
        ACity* City = *It;
        if (City && City->PlayerOwnerIndex == PlayerIndex)
        {
            // 调用 City 的回合处理，传入全局数据资产以计算具体数值
            City->ProcessTurn(GlobalTerrainData, GlobalBuildingData);

            // 获取该城市本回合产生的盈余 (金币/科技/文化)
            // 注意：CalculateTurnYields 已经在 ProcessTurn 内部被调用过一次用于计算粮食/生产力，
            // 这里我们再次调用以获取“上缴”给国家的资源。
            FYields CityYield = City->CalculateTurnYields(GlobalTerrainData, GlobalBuildingData);

            // 累加全局资源 (金币、科技、文化)
            TotalTurnIncome.Gold += CityYield.Gold;
            TotalTurnIncome.Science += CityYield.Science;
            TotalTurnIncome.Culture += CityYield.Culture;
        }
    }

    // 更新玩家库存
    if (PlayerGlobalYields.IsValidIndex(PlayerIndex))
    {
        PlayerGlobalYields[PlayerIndex] = PlayerGlobalYields[PlayerIndex] + TotalTurnIncome;

        UE_LOG(LogTemp, Log, TEXT("Player %d Turn End. Income: Gold+%d, Sci+%d. Total Gold: %d"),
            PlayerIndex, TotalTurnIncome.Gold, TotalTurnIncome.Science, PlayerGlobalYields[PlayerIndex].Gold);
    }

    for (TActorIterator<ACity> It(GetWorld()); It; ++It)
    {
        ACity* City = *It;
        if (City && City->PlayerOwnerIndex == PlayerIndex)
        {
            City->ProcessTurn(GlobalTerrainData, GlobalBuildingData);
            FYields CityYield = City->CalculateTurnYields(GlobalTerrainData, GlobalBuildingData);

            TotalTurnIncome.Gold += CityYield.Gold;
            TotalTurnIncome.Science += CityYield.Science;
            TotalTurnIncome.Culture += CityYield.Culture;
        }
    }

    // 更新库存
    if (PlayerGlobalYields.IsValidIndex(PlayerIndex))
    {
        PlayerGlobalYields[PlayerIndex] = PlayerGlobalYields[PlayerIndex] + TotalTurnIncome;
    }

    // 处理研发进度
    ProcessResearchProgress(PlayerIndex, TotalTurnIncome.Science, TotalTurnIncome.Culture);

    UE_LOG(LogTemp, Log, TEXT("Player %d Turn End. Sci+%d, Cult+%d"), PlayerIndex, TotalTurnIncome.Science, TotalTurnIncome.Culture);
}

FYields ACivi_GameModeBase::GetPlayerResources(int32 PlayerIndex) const
{
    if (PlayerGlobalYields.IsValidIndex(PlayerIndex))
    {
        return PlayerGlobalYields[PlayerIndex];
    }
    return FYields();
}

void ACivi_GameModeBase::AddPlayerResources(int32 PlayerIndex, const FYields& YieldsToAdd)
{
    if (PlayerGlobalYields.IsValidIndex(PlayerIndex))
    {
        PlayerGlobalYields[PlayerIndex] = PlayerGlobalYields[PlayerIndex] + YieldsToAdd;
    }
}

void ACivi_GameModeBase::UpdateFogOfWar()
{
    // 找到地图渲染器并更新视野
    // 这里假设场景里只有一个渲染器
    for (TActorIterator<AHexMapRenderer> It(GetWorld()); It; ++It)
    {
        AHexMapRenderer* Renderer = *It;
        if (Renderer)
        {
            // 调用之前任务中实现的迷雾更新函数
            Renderer->UpdateFogOfWarVisuals(MapGrid, CurrentPlayerIndex);
            return;
        }
    }
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

void ACivi_GameModeBase::InitResearch()
{
    PlayerTechStates.SetNum(TotalPlayers);
    PlayerCivicStates.SetNum(TotalPlayers);

    for (int32 i = 0; i < TotalPlayers; i++)
    {
        // 默认解锁 "农业"
        PlayerTechStates[i].UnlockedTechs.Add(ETechType::Agriculture);
        // 默认研究 "制陶术" (示例)
        PlayerTechStates[i].CurrentResearch = ETechType::Pottery;

        // 默认解锁 "法典"
        PlayerCivicStates[i].UnlockedCivics.Add(ECivicType::CodeOfLaws);
        PlayerCivicStates[i].CurrentCivic = ECivicType::Craftsmanship;
    }
}

void ACivi_GameModeBase::ProcessResearchProgress(int32 PlayerIndex, int32 ScienceYield, int32 CultureYield)
{
    if (!PlayerTechStates.IsValidIndex(PlayerIndex) || !GlobalTechData) return;

    // --- 处理科技 ---
    FPlayerResearchState& TechState = PlayerTechStates[PlayerIndex];
    if (TechState.CurrentResearch != ETechType::None)
    {
        TechState.CurrentScienceProgress += ScienceYield;

        FTechInfo TechInfo = GlobalTechData->GetTechInfo(TechState.CurrentResearch);
        if (TechState.CurrentScienceProgress >= TechInfo.ScienceCost)
        {
            // 研发完成！
            TechState.UnlockedTechs.Add(TechState.CurrentResearch);
            UE_LOG(LogTemp, Warning, TEXT("Player %d researched Tech: %s"), PlayerIndex, *TechInfo.DisplayName.ToString());

            // 重置
            TechState.CurrentResearch = ETechType::None;
            TechState.CurrentScienceProgress = 0;
            // 实际 Civ6 会保留溢出值，这里简化处理
        }
    }

    // --- 处理市政 ---
    if (!PlayerCivicStates.IsValidIndex(PlayerIndex) || !GlobalCivicData) return;

    FPlayerCivicState& CivicState = PlayerCivicStates[PlayerIndex];
    if (CivicState.CurrentCivic != ECivicType::None)
    {
        CivicState.CurrentCultureProgress += CultureYield;

        FCivicInfo CivicInfo = GlobalCivicData->GetCivicInfo(CivicState.CurrentCivic);
        if (CivicState.CurrentCultureProgress >= CivicInfo.CultureCost)
        {
            // 研发完成！
            CivicState.UnlockedCivics.Add(CivicState.CurrentCivic);
            UE_LOG(LogTemp, Warning, TEXT("Player %d researched Civic: %s"), PlayerIndex, *CivicInfo.DisplayName.ToString());

            CivicState.CurrentCivic = ECivicType::None;
            CivicState.CurrentCultureProgress = 0;
        }
    }
}

void ACivi_GameModeBase::SetPlayerResearch(int32 PlayerIndex, ETechType Tech)
{
    if (PlayerTechStates.IsValidIndex(PlayerIndex))
    {
        // 实际逻辑应该检查前置科技是否已解锁
        PlayerTechStates[PlayerIndex].CurrentResearch = Tech;
        UE_LOG(LogTemp, Log, TEXT("Player %d started researching Tech %d"), PlayerIndex, (int32)Tech);
    }
}

void ACivi_GameModeBase::SetPlayerCivic(int32 PlayerIndex, ECivicType Civic)
{
    if (PlayerCivicStates.IsValidIndex(PlayerIndex))
    {
        PlayerCivicStates[PlayerIndex].CurrentCivic = Civic;
    }
}

bool ACivi_GameModeBase::IsTechUnlocked(int32 PlayerIndex, ETechType Tech) const
{
    if (PlayerTechStates.IsValidIndex(PlayerIndex))
    {
        return PlayerTechStates[PlayerIndex].UnlockedTechs.Contains(Tech);
    }
    return false;
}

ULandblock* ACivi_GameModeBase::GetLandblockFromWorldPos(FVector WorldPos)
{
    // 简单的数学反算 (假设 HexRadius = 100, HexGap = 2, 与 MapRenderer 一致)
    float R = 100.0f;
    float W = R * 2.0f + 2.0f;
    float H = R * FMath::Sqrt(3.0f) + 2.0f;

    // 估算 GridY
    float ApproxY = WorldPos.Y / H;
    int32 Y = FMath::RoundToInt(ApproxY);

    // 根据行奇偶性修正 X 偏移
    float XOffset = 0.0f;
    if (Y % 2 == 1) XOffset = W * 0.5f; // 奇数行偏移半个宽

    float ApproxX = (WorldPos.X - (XOffset * 0.75f)) / (W * 0.75f); // 0.75 是因为六边形水平重叠
    int32 X = FMath::RoundToInt(WorldPos.X / (W * 0.75f));

    // 注意：上面的反算是近似值，精准的六边形拾取需要更复杂的数学 (Axial Coordinates)
    // 对于原型，我们可以遍历鼠标周围几个格子找最近的，或者使用简单的距离检测

    ULandblock* BestBlock = nullptr;
    float MinDistSq = FLT_MAX;

    // 搜索鼠标点击位置附近的格子 (暴力搜索整个地图太慢，这里简单处理搜全图或局部)
    // 优化：只搜索计算出的 X,Y 附近的 3x3 范围
    // 为演示代码简洁，这里直接用简易逻辑：

    // 我们假设 RenderMap 时存了位置，这里反向查找比较麻烦。
    // 更简单的方法：直接遍历所有 Landblock，找距离 WorldPos 最近且距离 < R 的那个

    for (ULandblock* Block : MapGrid)
    {
        if (!Block) continue;

        // 重新计算该 Block 的中心位置 (代码复用 HexMapRenderer 的公式)
        float WorldX = Block->X * W * 0.75f;
        float WorldY = Block->Y * H;
        if (Block->X % 2 == 1) WorldY += H * 0.5f;

        float DistSq = FVector::DistSquared2D(WorldPos, FVector(WorldX, WorldY, 0));
        if (DistSq < (R * R) && DistSq < MinDistSq)
        {
            MinDistSq = DistSq;
            BestBlock = Block;
        }
    }

    return BestBlock;
}

void ACivi_GameModeBase::CheckVictoryConditions()
{
    if (bIsGameOver) return;

    // --- 1. 检测征服胜利 (Conquest Victory) ---
    // 规则：如果一个玩家控制了地图上所有的城市，则获胜。
    // (简化版规则，实际 Civ6 是占领所有原始首都)

    TMap<int32, int32> CityCounts; // PlayerIndex -> CityCount
    int32 TotalCities = 0;

    for (TActorIterator<ACity> It(GetWorld()); It; ++It)
    {
        ACity* City = *It;
        if (City)
        {
            CityCounts.FindOrAdd(City->PlayerOwnerIndex)++;
            TotalCities++;
        }
    }

    // 如果只有一个玩家拥有城市，且城市总数 > 0
    if (TotalCities > 0)
    {
        for (auto& Elem : CityCounts)
        {
            if (Elem.Value == TotalCities)
            {
                // 该玩家拥有所有城市
                DeclareVictory(Elem.Key, EVictoryType::Conquest);
                return;
            }
        }
    }

    // 如果没有城市（极少数情况），或者没有任何人拥有所有城市，继续检测...

    // --- 2. 检测科技胜利 (Science Victory) ---
    // 检查是否有玩家解锁了所有科技 (简单判定)
    if (GlobalTechData)
    {
        int32 TotalTechs = GlobalTechData->Techs.Num();
        for (int32 i = 0; i < TotalPlayers; ++i)
        {
            if (PlayerTechStates.IsValidIndex(i))
            {
                if (PlayerTechStates[i].UnlockedTechs.Num() >= TotalTechs && TotalTechs > 0)
                {
                    DeclareVictory(i, EVictoryType::Science);
                    return;
                }
            }
        }
    }
}

void ACivi_GameModeBase::DeclareVictory(int32 WinnerIndex, EVictoryType Type)
{
    if (bIsGameOver) return;

    bIsGameOver = true;
    WinnerPlayerIndex = WinnerIndex;
    VictoryType = Type;

    UE_LOG(LogTemp, Warning, TEXT("GAME OVER! Winner: Player %d, Type: %d"), WinnerIndex, (int32)Type);

    // 通知 UI
    OnGameOver(WinnerIndex, Type);

    // 可以在这里暂停游戏，或者禁止后续操作
    // UGameplayStatics::SetGamePaused(this, true);
}