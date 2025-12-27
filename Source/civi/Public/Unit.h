// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CiviTypes.h"
#include "CombatFunctionLibrary.h"
#include "Unit.generated.h"

class ULandblock;
class UUnitDataAsset;
class ACity;

UCLASS()
class CIVI_API AUnit : public AActor
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

public:
    // --- 初始化 ---
    AUnit();

    // 初始化单位数据
    UFUNCTION(BlueprintCallable, Category = "Unit")
    void InitUnit(ECiviUnitType Type, int32 OwnerIndex, ULandblock* StartBlock, UUnitDataAsset* DataAsset);

    // --- 属性 ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Stats")
    ECiviUnitType UnitType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Stats")
    int32 PlayerOwnerIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Stats")
    int32 CurrentHP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
    int32 MaxHP = 100;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Stats")
    int32 CombatStrength;

    // 移动力
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    int32 MovementPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    int32 MaxMovementPoints;

    // 位置
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Location")
    ULandblock* CurrentBlock;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Location")
    int32 GridX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Location")
    int32 GridY;

    // 是否驻守（防御加成）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsFortified;

    // --- 行为操作 ---

    // 移动到目标地块 (包含寻路和消耗计算)
    UFUNCTION(BlueprintCallable, Category = "Unit Action")
    bool MoveTo(ULandblock* TargetBlock);

    // 攻击目标单位
    UFUNCTION(BlueprintCallable, Category = "Unit Action")
    void AttackUnit(AUnit* Defender);

    // 建立城市 (仅开拓者)
    UFUNCTION(BlueprintCallable, Category = "Unit Action")
    void FoundCity();

    // 驻守
    UFUNCTION(BlueprintCallable, Category = "Unit Action")
    void Fortify();

    // 回合开始时重置点数
    UFUNCTION(BlueprintCallable, Category = "Turn System")
    void OnTurnStart();

    // --- 视觉组件 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
    UStaticMeshComponent* MeshComponent;

    // 更新：攻击现在使用通用逻辑
    // 为了支持攻击城市，可以添加重载或者专用函数
    UFUNCTION(BlueprintCallable, Category = "Unit Action")
    void AttackCity(ACity* TargetCity);

    // 远程攻击单位
    UFUNCTION(BlueprintCallable, Category = "Unit Action")
    void RangedAttackUnit(AUnit* TargetUnit);

    // 远程攻击城市
    UFUNCTION(BlueprintCallable, Category = "Unit Action")
    void RangedAttackCity(ACity* TargetCity);

    // 获取当前防御力 (包含地形修正)
    UFUNCTION(BlueprintPure, Category = "Combat")
    int32 GetDefensiveStrength() const;

private:
    // 简单的伤害计算公式
    int32 CalculateDamage(int32 AttackerStr, int32 DefenderStr);

    // 更新世界坐标位置
    void UpdateWorldLocation();
};