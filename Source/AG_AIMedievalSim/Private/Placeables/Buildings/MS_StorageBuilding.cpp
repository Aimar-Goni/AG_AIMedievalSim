// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/Buildings/MS_StorageBuilding.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMS_StorageBuilding::AMS_StorageBuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void AMS_StorageBuilding::BeginPlay()
{
	Super::BeginPlay();
	UWorld* world = GetWorld();
	if (world) {
		auto FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_ResourceSystem::StaticClass());
		AMS_ResourceSystem* resource = Cast<AMS_ResourceSystem>(FoundActor);

		if (FoundActor) {
			ResourceSystem_ = resource;
		}
		else {
			ResourceSystem_ = world->SpawnActor<AMS_ResourceSystem>(AMS_ResourceSystem::StaticClass());
		}
	}

}

// Called every frame
void AMS_StorageBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMS_StorageBuilding::LeaveResources()
{

}

void AMS_StorageBuilding::TakeResources()
{

}

