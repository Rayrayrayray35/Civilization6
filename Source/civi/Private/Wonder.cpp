// Fill out your copyright notice in the Description page of Project Settings.

#include "Wonder.h"

AWonder::AWonder()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // 奇观通常很大，默认设置一些碰撞预设
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
}

void AWonder::BeginPlay()
{
    Super::BeginPlay();
}

void AWonder::InitVisuals(UStaticMesh* Mesh, FVector Scale)
{
    if (MeshComponent && Mesh)
    {
        MeshComponent->SetStaticMesh(Mesh);
        SetActorScale3D(Scale);
    }
}
