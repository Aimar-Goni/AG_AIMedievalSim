// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/Buildings/MS_StorageBuilding.h"
#include "Systems/MS_ResourceSystem.h"

// Sets default values
AMS_ResourceSystem::AMS_ResourceSystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
   
}

// Called when the game starts or when spawned
void AMS_ResourceSystem::BeginPlay()
{
	Super::BeginPlay();
    
    
}

// Called every frame
void AMS_ResourceSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMS_ResourceSystem::SetBerries(int32 NewBerries)
{
    if (0 != NewBerries)
    {
        Berries_ += NewBerries;
        OnBerriesChanged.Broadcast(Berries_);
    }
}

int32  AMS_ResourceSystem::GetBerries()
{
    return Berries_;
}