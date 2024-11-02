// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Placeables/Buildings/MS_StorageBuilding.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMS_StorageBuildingPool::AMS_StorageBuildingPool()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMS_StorageBuildingPool::BeginPlay()
{
	Super::BeginPlay();
	FindStorageBuildingsOnScene();
}

// Called every frame
void AMS_StorageBuildingPool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMS_StorageBuildingPool::FindStorageBuildingsOnScene() {
	UWorld* world = GetWorld();
	if (world) {

		TSubclassOf<AMS_StorageBuilding> StorageClass = AMS_StorageBuilding::StaticClass();
		TArray<AActor*> Storages;
		UGameplayStatics::GetAllActorsOfClass(world, StorageClass, Storages);
		for (AActor* actor : Storages)
		{
			if (actor)
			{
				AMS_StorageBuilding* storage = Cast<AMS_StorageBuilding>(actor);

				StorageBuldings_.Add(storage);
				n_StorageBuldings_++;
			}
		}
	}
}