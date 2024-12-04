// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/MS_PoolManager.h"
#include "Placeables/Buildings/MS_BulletingBoardPool.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Kismet/GameplayStatics.h"

void UMS_PoolManager::Initialize()
{
    UWorld* World = GetWorld();

    // Initialize BulletingBoardPool
    AActor* FoundActor = UGameplayStatics::GetActorOfClass(World, AMS_BulletingBoardPool::StaticClass());
    BulletingBoardPool = FoundActor ? Cast<AMS_BulletingBoardPool>(FoundActor)
        : World->SpawnActor<AMS_BulletingBoardPool>();

    // Initialize StorageBuildingPool
    FoundActor = UGameplayStatics::GetActorOfClass(World, AMS_StorageBuildingPool::StaticClass());
    StorageBuildingPool = FoundActor ? Cast<AMS_StorageBuildingPool>(FoundActor)
        : World->SpawnActor<AMS_StorageBuildingPool>();

    // Initialize StorageBuildingPool
    FoundActor = UGameplayStatics::GetActorOfClass(World, AMS_WorkpPlacePool::StaticClass());
    WorkPlacePool = FoundActor ? Cast<AMS_WorkpPlacePool>(FoundActor)
        : World->SpawnActor<AMS_WorkpPlacePool>();
}

AMS_BulletingBoardPool* UMS_PoolManager::GetBulletingBoardPool() const
{
    return BulletingBoardPool;
}

AMS_StorageBuildingPool* UMS_PoolManager::GetStorageBuildingPool() const
{
    return StorageBuildingPool;
}

AMS_WorkpPlacePool* UMS_PoolManager::GetWorkPlacePool() const
{
    return WorkPlacePool;
}