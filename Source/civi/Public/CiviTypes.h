// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CiviTypes.generated.h"

// 资源产出结构体
USTRUCT(BlueprintType)
struct FYields
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Food = 0; // 粮食

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Production = 0; // 生产力

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Gold = 0; // 金币

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Science = 0; // 科技值

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Culture = 0; // 文化值

    // 运算符重载，方便累加
    FYields operator+(const FYields& Other) const
    {
        FYields Result;
        Result.Food = Food + Other.Food;
        Result.Production = Production + Other.Production;
        Result.Gold = Gold + Other.Gold;
        Result.Science = Science + Other.Science;
        Result.Culture = Culture + Other.Culture;
        return Result;
    }
};

// 视野状态枚举
UENUM(BlueprintType)
enum class EVisibilityState : uint8
{
    Unexplored  UMETA(DisplayName = "未探索"), // 黑色迷雾，看不见地形
    FogOfWar    UMETA(DisplayName = "战争迷雾"), // 已探索但当前无视野，能看见地形但看不见单位
    Visible     UMETA(DisplayName = "可见") // 当前拥有视野
};

// 地形类型枚举
UENUM(BlueprintType)
enum class ETerrain : uint8
{
    Ocean       UMETA(DisplayName = "海洋"),
    Coast       UMETA(DisplayName = "沿海"),
    Plain       UMETA(DisplayName = "平原"),
    Grassland   UMETA(DisplayName = "草原"),
    Desert      UMETA(DisplayName = "沙漠"),
    Tundra      UMETA(DisplayName = "冻土"),
    Snow        UMETA(DisplayName = "雪地")
};

// 地貌类型枚举
UENUM(BlueprintType)
enum class ELandform : uint8
{
    None        UMETA(DisplayName = "无"),
    Hills       UMETA(DisplayName = "丘陵"),
    Mountain    UMETA(DisplayName = "山脉"),
    Forest      UMETA(DisplayName = "森林"),
    Rainforest  UMETA(DisplayName = "雨林"),
    Marsh       UMETA(DisplayName = "沼泽"),
    Oasis       UMETA(DisplayName = "绿洲"),
    Ice         UMETA(DisplayName = "冰川")
};

// 六边形方向枚举
UENUM(BlueprintType)
enum class EHexDirection : uint8
{
    NorthEast = 0   UMETA(DisplayName = "东北"),
    East = 1        UMETA(DisplayName = "东"),
    SouthEast = 2   UMETA(DisplayName = "东南"),
    SouthWest = 3   UMETA(DisplayName = "西南"),
    West = 4        UMETA(DisplayName = "西"),
    NorthWest = 5   UMETA(DisplayName = "西北")
};

// 建筑类型枚举
UENUM(BlueprintType)
enum class EBuildingType : uint8
{
    None        UMETA(DisplayName = "无"),
    Farm        UMETA(DisplayName = "农场"),
    Mine        UMETA(DisplayName = "矿山"),
    Camp        UMETA(DisplayName = "营地"),
    LumberMill  UMETA(DisplayName = "伐木场"),
    Pasture     UMETA(DisplayName = "牧场")
};

// 奇观类型枚举
UENUM(BlueprintType)
enum class EWonderType : uint8
{
    None            UMETA(DisplayName = "无"),
    Pyramids        UMETA(DisplayName = "金字塔"),
    GreatLighthouse UMETA(DisplayName = "大灯塔"),
    Colossus        UMETA(DisplayName = "巨像"),
    Oracle          UMETA(DisplayName = "神谕")
};

// 单位类型
UENUM(BlueprintType)
enum class ECiviUnitType : uint8
{
    None        UMETA(DisplayName = "无"),
    Settler     UMETA(DisplayName = "开拓者"),
    Warrior     UMETA(DisplayName = "战士"),
    Scout       UMETA(DisplayName = "侦察兵"),
    Builder     UMETA(DisplayName = "建造者"),
    Archer      UMETA(DisplayName = "弓箭手")
};

// 单位行动类型
UENUM(BlueprintType)
enum class EUnitAction : uint8
{
    Move        UMETA(DisplayName = "移动"),
    Attack      UMETA(DisplayName = "攻击"),
    Fortify     UMETA(DisplayName = "驻守"),
    FoundCity   UMETA(DisplayName = "建立城市"),
    Sleep       UMETA(DisplayName = "休眠")
};

// 单位战斗结果
UENUM(BlueprintType)
enum class ECombatResult : uint8
{
    Victory,
    Defeat,
    Draw
};

// 生产项目结构体
USTRUCT(BlueprintType)
struct FProductionItem
{
    GENERATED_BODY()

    // 正在建造的建筑类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingType BuildingType = EBuildingType::None;

    // 还可以扩展支持 EUnitType, EProjectType 等

    // 总共需要的生产力
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalCost = 0;

    // 当前已投入的生产力
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Progress = 0;

    // 显示名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;
};

// 科技类型
UENUM(BlueprintType)
enum class ETechType : uint8
{
    None            UMETA(DisplayName = "无"),
    Agriculture     UMETA(DisplayName = "农业"), // 初始科技
    Pottery         UMETA(DisplayName = "制陶术"),
    Mining          UMETA(DisplayName = "采矿业"),
    AnimalHusbandry UMETA(DisplayName = "畜牧业"),
    Archery         UMETA(DisplayName = "箭术"),
    BronzeWorking   UMETA(DisplayName = "青铜铸造"),
    IronWorking     UMETA(DisplayName = "铁器业")
};

// 市政(文化)类型
UENUM(BlueprintType)
enum class ECivicType : uint8
{
    None            UMETA(DisplayName = "无"),
    CodeOfLaws      UMETA(DisplayName = "法典"), // 初始市政
    Craftsmanship   UMETA(DisplayName = "技艺"),
    ForeignTrade    UMETA(DisplayName = "对外贸易"),
    MilitaryTradition UMETA(DisplayName = "军事传统"),
    StateWorkforce  UMETA(DisplayName = "国家劳动力")
};

// 胜利方式枚举
UENUM(BlueprintType)
enum class EVictoryType : uint8
{
    None            UMETA(DisplayName = "进行中"),
    Conquest        UMETA(DisplayName = "征服胜利"), // 控制所有城市
    Science         UMETA(DisplayName = "科技胜利"), // 这里的简化版：解锁所有科技
    Culture         UMETA(DisplayName = "文化胜利"), // 这里的简化版：解锁所有市政
    Time            UMETA(DisplayName = "分数胜利")  // 回合耗尽
};
