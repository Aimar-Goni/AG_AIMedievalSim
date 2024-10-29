// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/Buildings/MS_StorageBuilding.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMS_StorageBuilding::AMS_StorageBuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Inventory Component
	Inventory_ = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

}

// Called when the game starts or when spawned
void AMS_StorageBuilding::BeginPlay()
{
	
	for (auto& Resource : Inventory_->Resources_)
	{
		Inventory_->OnResourceChanged.Broadcast(Resource.Key, 50);

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

