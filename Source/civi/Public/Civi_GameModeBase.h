// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CiviTypes.h"
#include "BuildingDataAsset.h"
#include "TerrainDataAsset.h"
#include "TechDataAsset.h"
#include "CivicDataAsset.h"
#include "Civi_GameModeBase.generated.h"

class ULandblock;
class ACity;
class AUnit;
class AHexMapRenderer;

// 玩家研发状态结构体
USTRUCT(BlueprintType)
struct FPlayerResearchState
{
    GENERATED_BODY()

    // 已解锁的科技集合
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TSet<ETechType> UnlockedTechs;

    // 当前正在研究的科技
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    ETechType CurrentResearch = ETechType::None;

    // 当前科技已投入的科技值
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    int32 CurrentScienceProgress = 0;
};

// 玩家市政状态结构体
USTRUCT(BlueprintType)
struct FPlayerCivicState
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TSet<ECivicType> UnlockedCivics;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    ECivicType CurrentCivic = ECivicType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    int32 CurrentCultureProgress = 0;
};

UCLASS()
class CIVI_API ACivi_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
    ACivi_GameModeBase();

    // 初始化地图
    UFUNCTION(BlueprintCallable, Category = "Map Generation")
    void InitMap();

    // 地图尺寸 (小型地图 74x46)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    int32 MapWidth = 74;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    int32 MapHeight = 46;

    // 随机种子
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    int32 MapSeed = 0;

    // 噪声参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Settings")
    float ElevationScale = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Settings")
    float MoistureScale = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Settings")
    float TemperatureScale = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Settings")
    int32 Octaves = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Settings")
    float Persistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise Settings")
    float Lacunarity = 2.0f;

    // 地图数据 (二维数组用一维表示)
    UPROPERTY(BlueprintReadOnly, Category = "Map Data")
    TArray<ULandblock*> MapGrid;

    //回合制系统

    // 当前回合数 (从1开始)
    UPROPERTY(BlueprintReadOnly, Category = "Turn System")
    int32 CurrentTurn = 1;

    // 当前行动的玩家索引 (0:玩家1, 1:玩家2)
    UPROPERTY(BlueprintReadOnly, Category = "Turn System")
    int32 CurrentPlayerIndex = 0;

    // 总玩家数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn System")
    int32 TotalPlayers = 2;

    // 结束当前回合 (UI按钮绑定此函数)
    UFUNCTION(BlueprintCallable, Category = "Turn System")
    void EndTurn();

    // 获取当前玩家索引
    UFUNCTION(BlueprintCallable, Category = "Turn System")
    int32 GetCurrentPlayerIndex() const { return CurrentPlayerIndex; }

    //资源与经济系统

    // 玩家全局资源库存 (索引对应 PlayerIndex)
    // 包含累积的金币、科技值、文化值等
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
    TArray<FYields> PlayerGlobalYields;

    // 游戏逻辑需要的核心数据资产 (需在蓝图中配置)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game Data")
    UTerraindataasset* GlobalTerrainData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game Data")
    UBuildingDataAsset* GlobalBuildingData;

    // 获取玩家当前的资源
    UFUNCTION(BlueprintCallable, Category = "Economy")
    FYields GetPlayerResources(int32 PlayerIndex) const;

    // 修改玩家资源 (例如消费金币)
    UFUNCTION(BlueprintCallable, Category = "Economy")
    void AddPlayerResources(int32 PlayerIndex, const FYields& YieldsToAdd);

    // 科技与文化数据

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game Data")
    UTechDataAsset* GlobalTechData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game Data")
    UCivicDataAsset* GlobalCivicData;

    // 玩家科技状态 (索引对应 PlayerIndex)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Research")
    TArray<FPlayerResearchState> PlayerTechStates;

    // 玩家市政状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Research")
    TArray<FPlayerCivicState> PlayerCivicStates;

    // --- 接口 ---

    // 玩家选择研究项目
    UFUNCTION(BlueprintCallable, Category = "Research")
    void SetPlayerResearch(int32 PlayerIndex, ETechType Tech);

    UFUNCTION(BlueprintCallable, Category = "Research")
    void SetPlayerCivic(int32 PlayerIndex, ECivicType Civic);

    // 检查是否解锁
    UFUNCTION(BlueprintCallable, Category = "Research")
    bool IsTechUnlocked(int32 PlayerIndex, ETechType Tech) const;

    // 根据世界坐标找到对应的地块
    UFUNCTION(BlueprintCallable, Category = "Map Helper")
    ULandblock* GetLandblockFromWorldPos(FVector WorldPos);

    // --- 胜利判定系统 ---

    // 游戏是否已结束
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Victory")
    bool bIsGameOver = false;

    // 获胜者索引
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Victory")
    int32 WinnerPlayerIndex = -1;

    // 胜利类型
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Victory")
    EVictoryType VictoryType = EVictoryType::None;

    // 核心函数：检查是否有玩家达成胜利条件
    // 通常在回合结束、城市被攻占、科技研发完成时调用
    UFUNCTION(BlueprintCallable, Category = "Victory")
    void CheckVictoryConditions();

    // 触发游戏结束
    UFUNCTION(BlueprintCallable, Category = "Victory")
    void DeclareVictory(int32 WinnerIndex, EVictoryType Type);

    // --- UI 事件 ---

    // 通知蓝图显示胜利画面 (BlueprintImplementableEvent 不需要 C++ 实现)
    UFUNCTION(BlueprintImplementableEvent, Category = "Victory")
    void OnGameOver(int32 WinnerIndex, EVictoryType Type);

protected:
    virtual void BeginPlay() override;

    // 开始新的回合（处理单位重置、迷雾更新）
    void StartTurn();

    // 处理回合结束逻辑（城市产出结算）
    void ProcessTurnEndForPlayer(int32 PlayerIndex);

    // 处理回合开始逻辑（单位行动力重置）
    void ProcessTurnStartForPlayer(int32 PlayerIndex);

    // 辅助：更新战争迷雾
    void UpdateFogOfWar();

    // 内部：初始化所有玩家的默认科技
    void InitResearch();

    // 内部：处理研发进度结算
    void ProcessResearchProgress(int32 PlayerIndex, int32 ScienceYield, int32 CultureYield);

private:
    // 柏林噪声相关
    TArray<int32> Permutation;

    void InitPermutation();
    float PerlinNoise2D(float X, float Y);
    float OctavePerlinNoise(float X, float Y, int32 NumOctaves, float InPersistence, float InLacunarity);
    float Fade(float T);
    float Lerp(float A, float B, float T);
    float Grad(int32 Hash, float X, float Y);

    // 地图生成
    void GenerateElevationMap(TArray<float>& OutElevation);
    void GenerateMoistureMap(TArray<float>& OutMoisture);
    void GenerateTemperatureMap(TArray<float>& OutTemperature);

    ETerrain DetermineTerrain(float Elevation, float Moisture, float Temperature, int32 Y);
    ELandform DetermineLandform(ETerrain Terrain, float Elevation, float Moisture, float Temperature);

    void LinkNeighbors();
    int32 GetIndex(int32 X, int32 Y) const;
    ULandblock* GetLandblock(int32 X, int32 Y) const;

    // 六边形邻居偏移 (偶数行和奇数行不同)
    void GetHexNeighborOffsets(int32 Y, TArray<FIntPoint>& OutOffsets) const;
};
