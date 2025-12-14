// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CiviTypes.h"
#include "Civi_GameModeBase.generated.h"

/**
 * 
 */

class ULandblock;

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

protected:
    virtual void BeginPlay() override;

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
