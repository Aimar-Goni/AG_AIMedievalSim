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
			if (NodeMeshStarter->bNodeMapReady)
			{
				OnNodeMapInitialized();
			}
			break;
		}
	}

	StorageBuildingClass =  AMS_StorageBuilding::StaticClass();

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

				StorageBuldings_.Add(TWeakObjectPtr<AMS_StorageBuilding>(storage));
				storage->placeActive_ = true;
				// Adds the billboard to the navigation mesh
				storage->GridPosition_ =  PathfindingSubsystem->AddNodeAtPosition(storage->GetActorLocation());
				n_StorageBuldings_++;
			}
		}

		if (!StorageBuildingClass) return;
	

		 // for (int32 i = 0; i < 10; ++i)
		 // {
		 // 	FVector SpawnLocation = FVector(100 * i, 0, 0); // Offset spawn to avoid overlaps
		 // 	FRotator SpawnRotation = FRotator::ZeroRotator;
		 //
		 // 	FActorSpawnParameters SpawnParams;
		 // 	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		 //
		 // 	AMS_StorageBuilding* NewStorage = world->SpawnActor<AMS_StorageBuilding>(StorageBuildingClass, SpawnLocation, SpawnRotation, SpawnParams);
		 // 	if (NewStorage)
		 // 	{
		 // 		DeactivateStorageBuilding(NewStorage);
		 // 		StorageBuldings_.Add(TWeakObjectPtr<AMS_StorageBuilding>(NewStorage));
		 // 		n_StorageBuldings_++;
		 // 	}
		 // }
	}
		
	
}

void AMS_StorageBuildingPool::OnNodeMapInitialized()
{
	UE_LOG(LogTemp, Log, TEXT("Node Map is ready. Initializing Storage Buildings."));
	FindStorageBuildingsOnScene();
}

void AMS_StorageBuildingPool::DeactivateStorageBuilding(AMS_StorageBuilding* Building)
{
	if (!Building) return;
	
	Building->placeActive_ = false;
	Building->SetActorLocation(FVector(0, 0, -50000));
	Building->SetActorHiddenInGame(true);
	Building->SetActorEnableCollision(false);
	Building->SetActorTickEnabled(false);
}

void AMS_StorageBuildingPool::ReactivateStorageBuilding(AMS_StorageBuilding* Building, const FVector& NewLocation)
{
	if (!Building) return;
	
	Building->placeActive_ = false;
	Building->SetActorLocation(NewLocation);
	Building->SetActorHiddenInGame(false);
	Building->SetActorEnableCollision(true);
	Building->SetActorTickEnabled(true);

	// Recalculate grid position if needed
	if (UMS_PathfindingSubsystem* PathfindingSubsystem = GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>())
	{
		Building->GridPosition_ = PathfindingSubsystem->AddNodeAtPosition(NewLocation);
	}
}