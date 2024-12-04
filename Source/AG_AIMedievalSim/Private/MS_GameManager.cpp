// Fill out your copyright notice in the Description page of Project Settings.

#include "MS_GameManager.h"
#include "Systems/MS_PoolManager.h"

//#include "TaskManager.h"
#include "Movement/MS_PathfindingSubsyste.h"
#include "Kismet/GameplayStatics.h"

void AMS_GameManager::BeginPlay()
{
    Super::BeginPlay();
    Initialize();
}

void AMS_GameManager::Initialize()
{
    UWorld* World = GetWorld();

    // Initialize PoolManager
    PoolManager = NewObject<UMS_PoolManager>(this);
    PoolManager->Initialize();

    // Initialize TaskManager
    //TaskManager = NewObject<UTaskManager>(this);
    //TaskManager->Initialize(ResourceManager->GetBulletingBoardPool());


}