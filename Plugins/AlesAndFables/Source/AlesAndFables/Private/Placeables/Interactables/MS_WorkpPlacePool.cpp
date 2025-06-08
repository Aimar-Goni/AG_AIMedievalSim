#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "Movement/MS_PathfindingSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Placeables/Buildings/MS_WellWorkPlace.h"
#include "Placeables/Interactables/MS_BushWorkPlace.h"
#include "Systems/MS_TreeWorkPlace.h"

// Sets default values
AMS_WorkpPlacePool::AMS_WorkpPlacePool()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	WorkplaceClasses = { AMS_TreeWorkPlace::StaticClass(), AMS_WellWorkPlace::StaticClass(), AMS_BushWorkPlace::StaticClass() };
	
}

// Called when the game starts or when spawned
void AMS_WorkpPlacePool::BeginPlay()
{
	Super::BeginPlay();
	FindWorkplacesOnScene();
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AMS_WorkpPlacePool::SpawnWorkplaceAtRandomNode, WorkplaceSpawnInterval, true);
}

// Called every frame
void AMS_WorkpPlacePool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Checks the scene for already existing workplaces to add to the pool
void AMS_WorkpPlacePool::FindWorkplacesOnScene() {
	UMS_PathfindingSubsystem* PathfindingSubsystem = GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
	UWorld* world = GetWorld();
	if (world) {

		TSubclassOf<AMS_BaseWorkPlace> WorkPlaceClass = AMS_BaseWorkPlace::StaticClass();
		TArray<AActor*> Workplaces;
		UGameplayStatics::GetAllActorsOfClass(world, WorkPlaceClass, Workplaces);
		for (AActor* actor : Workplaces)
		{
			if (actor)
			{
				AMS_BaseWorkPlace* workPlace = Cast<AMS_BaseWorkPlace>(actor);
				workPlace->placeActive_ = true;
				ActiveWorkplaces_.Add(TWeakObjectPtr<AMS_BaseWorkPlace>(workPlace));
				// Adds the workplace to the navigation mesh
				workPlace->GridPosition_ = PathfindingSubsystem->AddNodeAtPosition(workPlace->GetActorLocation());
				n_workplaces_++;
			}
		}

		// Spawn new workplaces if necessary
		for (int32 i = 0; i < 300; ++i)
		{
			int32 ClassIndex = i % WorkplaceClasses.Num(); // Cycle through available classes
			TSubclassOf<AMS_BaseWorkPlace> SelectedClass = WorkplaceClasses[ClassIndex];
		
			if (!SelectedClass) continue;
		
			FVector SpawnLocation = FVector(100 * i, 200, 0); // Offset spawn
			FRotator SpawnRotation = FRotator::ZeroRotator;
		
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
			AMS_BaseWorkPlace* NewWorkplace = world->SpawnActor<AMS_BaseWorkPlace>(SelectedClass, SpawnLocation, SpawnRotation, SpawnParams);
			if (NewWorkplace)
			{
				InactiveWorkplaces_.Add(TWeakObjectPtr<AMS_BaseWorkPlace>(NewWorkplace));
				DeactivateWorkplace(NewWorkplace);
				n_workplaces_++;
			}
		}
	}
}

void AMS_WorkpPlacePool::DeactivateWorkplace(AMS_BaseWorkPlace* Workplace)
{
	if (!Workplace) return;
	Workplace->placeActive_ = false;

	Workplace->SetActorLocation(FVector(0, 0, -50000));
	Workplace->SetActorHiddenInGame(true);
	Workplace->SetActorEnableCollision(false);
	Workplace->SetActorTickEnabled(false);

	// Move the workplace to the inactive pool
	ActiveWorkplaces_.Remove(TWeakObjectPtr<AMS_BaseWorkPlace>(Workplace));
	InactiveWorkplaces_.Add(TWeakObjectPtr<AMS_BaseWorkPlace>(Workplace));
}

void AMS_WorkpPlacePool::ReactivateWorkplace(AMS_BaseWorkPlace* Workplace, const FVector& NewLocation)
{
	if (!Workplace) return;
	
	Workplace->placeActive_ = true;
	Workplace->SetActorLocation(NewLocation);
	Workplace->SetActorHiddenInGame(false);
	Workplace->SetActorEnableCollision(true);
	Workplace->SetActorTickEnabled(true);

	// Move the workplace to the active pool
	InactiveWorkplaces_.Remove(TWeakObjectPtr<AMS_BaseWorkPlace>(Workplace));
	ActiveWorkplaces_.Add(TWeakObjectPtr<AMS_BaseWorkPlace>(Workplace));
}

void AMS_WorkpPlacePool::SpawnWorkplaceAtRandomNode()
{
	UMS_PathfindingSubsystem* PathfindingSubsystem = GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
	if (!PathfindingSubsystem || WorkplaceClasses.Num() == 0) return;

	if (!bSpawning) return;
	
	// Get a random free node position
	FVector NodeWorldLocation;
	FIntPoint NodeGrid;
	if (!PathfindingSubsystem->GetRandomFreeNode(NodeWorldLocation, NodeGrid)) return;

	// Pick a random workplace class
	int32 ClassIndex = FMath::RandRange(0, WorkplaceClasses.Num() - 1);
	TSubclassOf<AMS_BaseWorkPlace> SelectedClass = WorkplaceClasses[ClassIndex];
	
	AMS_BaseWorkPlace* NewWorkplace = nullptr;
	for (int32 i = InactiveWorkplaces_.Num() - 1; i >= 0; --i)
	{
		TWeakObjectPtr<AMS_BaseWorkPlace> Candidate = InactiveWorkplaces_[i];
		if (Candidate.Get() && Candidate->GetClass() == SelectedClass)
		{
			NewWorkplace = Candidate.Get();
			break;
		}
	}
	
	if (NewWorkplace)
	{
		// Register workplace node and block original node
		PathfindingSubsystem->BlockNode(NodeWorldLocation);
		NewWorkplace->GridPosition_ = PathfindingSubsystem->AddNodeAtPosition(NodeWorldLocation);

		ReactivateWorkplace(NewWorkplace, NodeWorldLocation);
	}
}

void AMS_WorkpPlacePool::RemoveWorkplaceAndFreeNode(AMS_BaseWorkPlace* TargetWorkplace)
{
	if (!TargetWorkplace || !ActiveWorkplaces_.Contains(TargetWorkplace))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid or unknown workplace passed to RemoveWorkplaceAndFreeNode."));
		return;
	}

	// Deactivate the workplace logic
	DeactivateWorkplace(TargetWorkplace);
	
	UMS_PathfindingSubsystem* PathfindingSubsystem = GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();

	// Free the node
	if (PathfindingSubsystem)
	{
		FVector Position = TargetWorkplace->GetActorLocation();
		PathfindingSubsystem->UnblockNode(Position);
	}
}
