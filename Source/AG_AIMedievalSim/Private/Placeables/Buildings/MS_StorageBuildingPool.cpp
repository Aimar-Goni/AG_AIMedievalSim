// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Placeables/Buildings/MS_StorageBuilding.h"
#include "Movement/MS_PathfindingSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// Sets default values
AMS_StorageBuildingPool::AMS_StorageBuildingPool()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMS_StorageBuildingPool::BeginPlay()
{
	Super::BeginPlay();


	for (TActorIterator<AMS_MovementNodeMeshStarter> It(GetWorld()); It; ++It)
	{
		AMS_MovementNodeMeshStarter* NodeMeshStarter = *It;
		if (NodeMeshStarter)
		{
			// Bind to the OnNodeMapReady delegate
			NodeMeshStarter->OnNodeMapReady.AddDynamic(this, &AMS_StorageBuildingPool::OnNodeMapInitialized);
			break;
		}
	}



}

// Called every frame
void AMS_StorageBuildingPool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Checks the scene for already existing storages to add to the pool
void AMS_StorageBuildingPool::FindStorageBuildingsOnScene() {
	UMS_PathfindingSubsystem* PathfindingSubsystem = GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
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
				// Adds the billboard to the navigation mesh
				storage->GridPosition_ =  PathfindingSubsystem->AddNodeAtPosition(storage->GetActorLocation());
				n_StorageBuldings_++;
			}
		}
	}
}

void AMS_StorageBuildingPool::OnNodeMapInitialized()
{
	UE_LOG(LogTemp, Log, TEXT("Node Map is ready. Initializing Storage Buildings."));
	FindStorageBuildingsOnScene();
}