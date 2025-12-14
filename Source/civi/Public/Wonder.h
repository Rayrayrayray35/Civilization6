// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CiviTypes.h"
#include "Wonder.generated.h"

class ULandblock;

/**
 * 奇观的视觉实体 Actor
 */
UCLASS()
class CIVI_API AWonder : public AActor
{
    GENERATED_BODY()

public:
    AWonder();

    // 视觉网格组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wonder")
    UStaticMeshComponent* MeshComponent;

    // 奇观类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wonder", Meta = (ExposeOnSpawn = true))
    EWonderType WonderType;

    // 所属地块坐标（逻辑关联）
    UPROPERTY(BlueprintReadOnly, Category = "Wonder")
    int32 GridX;

    UPROPERTY(BlueprintReadOnly, Category = "Wonder")
    int32 GridY;

    // 初始化视觉（设置模型等）
    virtual void InitVisuals(UStaticMesh* Mesh, FVector Scale);

protected:
    virtual void BeginPlay() override;
};
