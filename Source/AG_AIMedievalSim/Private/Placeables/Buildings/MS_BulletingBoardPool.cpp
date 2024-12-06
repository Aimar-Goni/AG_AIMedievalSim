// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/Buildings/MS_BulletingBoardPool.h"
#include "Placeables/Buildings/MS_BulletingBoard.h"
#include "Movement/MS_PathfindingSubsyste.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMS_BulletingBoardPool::AMS_BulletingBoardPool()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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
			// Bind to the OnNodeMapReady delegate
			NodeMeshStarter->OnNodeMapReady.AddDynamic(this, &AMS_BulletingBoardPool::OnNodeMapInitialized);
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
	UMS_PathfindingSubsyste* PathfindingSubsystem = GetGameInstance()->GetSubsystem<UMS_PathfindingSubsyste>();
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