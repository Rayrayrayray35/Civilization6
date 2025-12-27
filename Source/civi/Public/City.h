// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CiviTypes.h"
#include "City.generated.h"

class ULandblock;
class UTerraindataasset;
class UBuildingDataAsset;
class AUnit;

UCLASS()
class CIVI_API ACity : public AActor
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

public:
    ACity();
    // --- 基础属性 ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Info")
    FText CityName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Info")
    int32 PlayerOwnerIndex;

    // 网格坐标
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Info")
    int32 GridX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Info")
    int32 GridY;

    // --- 经济与人口 ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Stats")
    int32 Population = 1;

    // 当前积累的粮食库存
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Stats")
    int32 FoodStock = 0;

    // 当前积累的溢出生产力
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Stats")
    int32 ProductionOverflow = 0;

    // --- 领土管理 ---

    // 城市中心所在的地块
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory")
    ULandblock* CityCenterTile;

    // 城市管辖的所有地块
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory")
    TArray<ULandblock*> OwnedTiles;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void AddTerritory(ULandblock* NewTile);

    // --- 产出计算 ---

    // 获取城市每回合总产出 (遍历地块 + 建筑 + 人口消耗)
    UFUNCTION(BlueprintCallable, Category = "City Economy")
    FYields CalculateTurnYields(const UTerraindataasset* TData, const UBuildingDataAsset* BData) const;

    // --- 建造系统 ---

    // 当前正在建造的项目
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    FProductionItem CurrentProduction;

    // 是否有正在进行的项目
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    bool bHasActiveProduction = false;

    // 开始建造（添加到队列）
    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartProductionBuilding(EBuildingType BuildingType, UBuildingDataAsset* DataAsset);

    // --- 回合流程 ---

    // 回合结束时调用：处理粮食增长、生产力结算
    UFUNCTION(BlueprintCallable, Category = "Turn System")
    void ProcessTurn(const UTerraindataasset* TData, const UBuildingDataAsset* BData);

    // 辅助：获取下一级人口所需粮食
    int32 GetFoodNeededForGrowth() const;

    // --- 战斗属性 ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 MaxHP = 200; // 城市血量通常比单位厚

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    int32 CurrentHP;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    int32 CombatStrength = 10; // 城市防御力 (应随人口/城墙提升)

    // --- 战斗行为 ---

    // 城市受到伤害
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ReceiveDamage(int32 DamageAmount, AUnit* Attacker);

    // 城市被攻占处理
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnCityCaptured(int32 NewOwnerIndex);

    // 回合开始时回血
    void RegenerateHealth();

private:
    // 内部：完成当前建造
    void FinishProduction();
};
