// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/Buildings/MS_BulletingBoardPool.h"
#include "Placeables/Buildings/MS_BulletingBoard.h"
#include "Movement/MS_PathfindingSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// Sets default values
AMS_BulletingBoardPool::AMS_BulletingBoardPool()
{
 	PrimaryActorTick.bCanEverTick = false;
	BulletingBoardClass =  AMS_BulletingBoard::StaticClass();


}

// Called when the game starts or when spawned
void AMS_BulletingBoardPool::BeginPlay()
{
	Super::BeginPlay();
	for (TActorIterator<AMS_MovementNodeMeshStarter> It(GetWorld()); It; ++It)
	{
		AMS_MovementNodeMeshStarter* NodeMeshStarter = *It;
		if (NodeMeshStarter)
		{
			// Bind to delegate
			NodeMeshStarter->OnNodeMapReady.AddDynamic(this, &AMS_BulletingBoardPool::OnNodeMapInitialized);

			// If the event has already been triggered, call the function manually
			if (NodeMeshStarter->bNodeMapReady)
			{
				OnNodeMapInitialized();
			}
			break;
		}
	}
}

// Called every frame
void AMS_BulletingBoardPool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Checks the scene for already existing billboards to add to the pool
void AMS_BulletingBoardPool::FindBulletingBoardsOnScene() {
	UMS_PathfindingSubsystem* PathfindingSubsystem = GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
	UWorld* world = GetWorld();
	if (world) {

		TSubclassOf<AMS_BulletingBoard> WorkPlaceClass = AMS_BulletingBoard::StaticClass();
		TArray<AActor*> Workplaces;
		UGameplayStatics::GetAllActorsOfClass(world, WorkPlaceClass, Workplaces);
		for (AActor* actor : Workplaces)
		{
			if (actor)
			{
				AMS_BulletingBoard* bulletingBoard = Cast<AMS_BulletingBoard>(actor);

				BulletingBoards_.Add(bulletingBoard);

				// Adds the billboard to the navigation mesh
				bulletingBoard->GridPosition_ = PathfindingSubsystem->AddNodeAtPosition(bulletingBoard->GetActorLocation());
				bulletingBoard->placeActive_ = true;
				n_BulletingBoards_++;
			}
		}


		if (!BulletingBoardClass) return;
		
		
		for (int32 i = 0; i < 10; ++i)
		{
			FVector SpawnLocation = FVector(100 * i, 0, 0); // Offset spawn to avoid overlaps
			FRotator SpawnRotation = FRotator::ZeroRotator;
		
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
			AMS_BulletingBoard* NewBoard = world->SpawnActor<AMS_BulletingBoard>(BulletingBoardClass, SpawnLocation, SpawnRotation, SpawnParams);
			if (NewBoard)
			{
				DeactivateBulletingBoardBuilding(NewBoard);
				BulletingBoards_.Add(TWeakObjectPtr<AMS_BulletingBoard>(NewBoard));
				n_BulletingBoards_++;
			}
		}
		
	}
}

void AMS_BulletingBoardPool::OnNodeMapInitialized()
{
	UE_LOG(LogTemp, Log, TEXT("Node Map is ready. Initializing Storage Buildings."));
	FindBulletingBoardsOnScene();
	OnBulletingBoardPoolInitialized.Broadcast();

}

void AMS_BulletingBoardPool::DeactivateBulletingBoardBuilding(AMS_BulletingBoard* Building)
{
	if (!Building) return;

	Building->placeActive_ = false;
	Building->SetActorLocation(FVector(0, 0, -50000));
	Building->SetActorHiddenInGame(true);
	Building->SetActorEnableCollision(false);
	Building->SetActorTickEnabled(false);
}

void AMS_BulletingBoardPool::ReactivateBulletingBoardBuilding(AMS_BulletingBoard* Building, const FVector& NewLocation)
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