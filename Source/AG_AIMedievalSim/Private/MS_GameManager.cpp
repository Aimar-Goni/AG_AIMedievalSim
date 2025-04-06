// Fill out your copyright notice in the Description page of Project Settings.

#include "MS_GameManager.h"
#include "Systems/MS_PoolManager.h"

//#include "TaskManager.h"
#include "Movement/MS_PathfindingSubsystem.h"
#include "Kismet/GameplayStatics.h"



void UMS_GameManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UWorld* World = GetWorld();

    // Initialize PoolManager
    PoolManager = NewObject<UMS_PoolManager>(this);
    PoolManager->Initialize();

    // Initialize TaskManager
    //TaskManager = NewObject<UTaskManager>(this);
    //TaskManager->Initialize(ResourceManager->GetBulletingBoardPool());


}
void UMS_GameManager::Deinitialize()
{
    Super::Deinitialize();
 
}
AMS_WorkpPlacePool* UMS_GameManager::GetWorkPlacePool() const
{
    return PoolManager ? PoolManager->GetWorkPlacePool() : nullptr;
}