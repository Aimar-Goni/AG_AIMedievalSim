// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/MS_PoolManager.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Kismet/GameplayStatics.h"

void UMS_PoolManager::Initialize()
{
    UWorld* World = GetWorld();
    
    // Initialize StorageBuildingPool
    AActor* FoundActor = UGameplayStatics::GetActorOfClass(World, AMS_StorageBuildingPool::StaticClass());
    StorageBuildingPool = FoundActor ? Cast<AMS_StorageBuildingPool>(FoundActor)
        : World->SpawnActor<AMS_StorageBuildingPool>();

    // Initialize StorageBuildingPool
    // FoundActor = UGameplayStatics::GetActorOfClass(World, AMS_WorkpPlacePool::StaticClass());
    // WorkPlacePool = FoundActor ? Cast<AMS_WorkpPlacePool>(FoundActor)
    //     : World->SpawnActor<AMS_WorkpPlacePool>();
}

AMS_StorageBuildingPool* UMS_PoolManager::GetStorageBuildingPool() const
{
    return StorageBuildingPool;
}

AMS_WorkpPlacePool* UMS_PoolManager::GetWorkPlacePool() const
{
    return WorkPlacePool;
}