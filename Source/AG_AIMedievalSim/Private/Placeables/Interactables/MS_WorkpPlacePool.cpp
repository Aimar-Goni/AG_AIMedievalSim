// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "Movement/MS_PathfindingSubsystem.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AMS_WorkpPlacePool::AMS_WorkpPlacePool()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMS_WorkpPlacePool::BeginPlay()
{
	Super::BeginPlay();
	FindWorkplacesOnScene();
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

				Workplaces_.Add(workPlace);
				// Adds the billboard to the navigation mesh
				workPlace->GridPosition_ = PathfindingSubsystem->AddNodeAtPosition(workPlace->GetActorLocation());
				n_workplaces_++;
			}
		}
	}
}