// HexMapRenderer.cpp
#include "HexMapRenderer.h"
#include "Terraindataasset.h"
#include "Landblock.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

AHexMapRenderer::AHexMapRenderer()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;
}

void AHexMapRenderer::BeginPlay()
{
    Super::BeginPlay();
}

FVector AHexMapRenderer::CalculateHexWorldPosition(int32 X, int32 Y) const
{
    // 六边形网格坐标转世界坐标
    // 使用 "pointy-top" 六边形布局（顶点朝上）

    float Width = HexRadius * 2.0f + HexGap;
    float Height = HexRadius * FMath::Sqrt(3.0f) + HexGap;

    float WorldX = X * Width * 0.75f;
    float WorldY = Y * Height;

    // 奇数列向下偏移半个高度
    if (X % 2 == 1)
    {
        WorldY += Height * 0.5f;
    }

    return FVector(WorldX, WorldY, 0.0f);
}

void AHexMapRenderer::RenderMap(const TArray<ULandblock*>& MapGrid, int32 MapWidth, int32 MapHeight)
{
    if (!TerrainDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("HexMapRenderer: TerrainDataAsset is not set!"));
        return;
    }

    // 清除之前的渲染
    ClearMap();

    UE_LOG(LogTemp, Log, TEXT("HexMapRenderer: Rendering map %dx%d"), MapWidth, MapHeight);

    if (bUseInstancing)
    {
        // 使用实例化渲染（高性能）
        for (int32 Y = 0; Y < MapHeight; Y++)
        {
            for (int32 X = 0; X < MapWidth; X++)
            {
                int32 Index = Y * MapWidth + X;
                if (Index >= MapGrid.Num()) continue;

                ULandblock* Block = MapGrid[Index];
                if (!Block) continue;

                FVector Position = CalculateHexWorldPosition(X, Y);

                // 添加地形实例
                UHierarchicalInstancedStaticMeshComponent* TerrainComp = GetOrCreateTerrainMeshComponent(Block->Terrain);
                if (TerrainComp)
                {
                    FTransform Transform(FRotator::ZeroRotator, Position, FVector(1.0f));
                    TerrainComp->AddInstance(Transform);
                }

                // 添加地貌实例（如果有）
                if (Block->Landform != ELandform::None)
                {
                    UHierarchicalInstancedStaticMeshComponent* LandformComp = GetOrCreateLandformMeshComponent(Block->Landform);
                    if (LandformComp)
                    {
                        FLandformDisplayData LandformData = TerrainDataAsset->GetLandformDisplayData(Block->Landform);

                        FRotator Rotation = FRotator::ZeroRotator;
                        if (LandformData.bRandomRotation)
                        {
                            Rotation.Yaw = FMath::RandRange(0.0f, 360.0f);
                        }

                        FVector LandformPos = Position + FVector(0, 0, LandformData.HeightOffset);
                        FTransform Transform(Rotation, LandformPos, LandformData.MeshScale);
                        LandformComp->AddInstance(Transform);
                    }
                }
            }
        }
    }
    else
    {
        // 非实例化渲染（用于调试或特殊效果）
        for (int32 Y = 0; Y < MapHeight; Y++)
        {
            for (int32 X = 0; X < MapWidth; X++)
            {
                int32 Index = Y * MapWidth + X;
                if (Index >= MapGrid.Num()) continue;

                ULandblock* Block = MapGrid[Index];
                if (!Block) continue;

                FVector Position = CalculateHexWorldPosition(X, Y);
                AActor* TileActor = SpawnTileActor(Block, Position);
                if (TileActor)
                {
                    SpawnedTileActors.Add(TileActor);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("HexMapRenderer: Map rendering complete"));
}

void AHexMapRenderer::ClearMap()
{
    // 清除实例化组件
    for (auto& Pair : TerrainMeshComponents)
    {
        if (Pair.Value)
        {
            Pair.Value->ClearInstances();
            Pair.Value->DestroyComponent();
        }
    }
    TerrainMeshComponents.Empty();

    for (auto& Pair : LandformMeshComponents)
    {
        if (Pair.Value)
        {
            Pair.Value->ClearInstances();
            Pair.Value->DestroyComponent();
        }
    }
    LandformMeshComponents.Empty();

    // 清除生成的Actor
    for (AActor* Actor : SpawnedTileActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    SpawnedTileActors.Empty();
}

void AHexMapRenderer::UpdateTile(ULandblock* Landblock)
{
    // TODO: 实现单个地块的更新
    // 这需要追踪每个地块对应的实例索引
    UE_LOG(LogTemp, Warning, TEXT("UpdateTile not fully implemented yet"));
}

AActor* AHexMapRenderer::SpawnTileActor(ULandblock* Landblock, const FVector& Position)
{
    if (!Landblock || !TerrainDataAsset) return nullptr;

    FTerrainDisplayData TerrainData = TerrainDataAsset->GetTerrainDisplayData(Landblock->Terrain);

    // 创建一个简单的静态网格Actor
    // 在实际项目中，你可能想创建一个专门的 HexTile Actor 类

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;

    AActor* TileActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Position, FRotator::ZeroRotator, SpawnParams);

    if (TileActor && TerrainDataAsset->HexBaseMesh)
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(TileActor);
        MeshComp->SetStaticMesh(TerrainDataAsset->HexBaseMesh);

        if (TerrainData.BaseMaterial)
        {
            MeshComp->SetMaterial(0, TerrainData.BaseMaterial);
        }

        MeshComp->RegisterComponent();
        TileActor->SetRootComponent(MeshComp);
    }

    return TileActor;
}

UHierarchicalInstancedStaticMeshComponent* AHexMapRenderer::GetOrCreateTerrainMeshComponent(ETerrain TerrainType)
{
    if (!TerrainDataAsset || !TerrainDataAsset->HexBaseMesh) return nullptr;

    // 检查是否已存在
    if (UHierarchicalInstancedStaticMeshComponent** Found = TerrainMeshComponents.Find(TerrainType))
    {
        return *Found;
    }

    // 创建新的实例化组件
    FName ComponentName = FName(*FString::Printf(TEXT("TerrainMesh_%d"), (int32)TerrainType));
    UHierarchicalInstancedStaticMeshComponent* NewComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, ComponentName);

    NewComp->SetStaticMesh(TerrainDataAsset->HexBaseMesh);

    FTerrainDisplayData TerrainData = TerrainDataAsset->GetTerrainDisplayData(TerrainType);
    if (TerrainData.BaseMaterial)
    {
        NewComp->SetMaterial(0, TerrainData.BaseMaterial);
    }

    NewComp->SetupAttachment(RootComponent);
    NewComp->RegisterComponent();

    // 优化设置
    NewComp->SetCullDistances(10000, 50000);
    NewComp->bUseAsOccluder = false;

    TerrainMeshComponents.Add(TerrainType, NewComp);

    return NewComp;
}

UHierarchicalInstancedStaticMeshComponent* AHexMapRenderer::GetOrCreateLandformMeshComponent(ELandform LandformType)
{
    if (!TerrainDataAsset) return nullptr;

    FLandformDisplayData LandformData = TerrainDataAsset->GetLandformDisplayData(LandformType);
    if (!LandformData.Mesh) return nullptr;

    // 检查是否已存在
    if (UHierarchicalInstancedStaticMeshComponent** Found = LandformMeshComponents.Find(LandformType))
    {
        return *Found;
    }

    // 创建新的实例化组件
    FName ComponentName = FName(*FString::Printf(TEXT("LandformMesh_%d"), (int32)LandformType));
    UHierarchicalInstancedStaticMeshComponent* NewComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, ComponentName);

    NewComp->SetStaticMesh(LandformData.Mesh);

    if (LandformData.OverlayMaterial)
    {
        NewComp->SetMaterial(0, LandformData.OverlayMaterial);
    }

    NewComp->SetupAttachment(RootComponent);
    NewComp->RegisterComponent();

    // 优化设置
    NewComp->SetCullDistances(8000, 40000);

    LandformMeshComponents.Add(LandformType, NewComp);

    return NewComp;
}

