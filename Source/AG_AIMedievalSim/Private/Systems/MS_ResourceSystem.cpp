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

void AMS_ResourceSystem::SetResource(ResourceType Type, int32 NewAmount)
{
    if (0 != NewAmount)
    {
        int32& ResourceValue = Inventory_.Resources_.FindOrAdd(Type);
        ResourceValue += NewAmount;

        // Broadcast changes based on the resource type
        OnResourceChanged.Broadcast(Type, ResourceValue);
    }
}

int32 AMS_ResourceSystem::GetResource(ResourceType Type)
{
    return Inventory_.GetResourceAmount(Type);
}


FText AMS_ResourceSystem::ConvertResourceTypeToText(ResourceType Type)
{
    switch (Type)
    {
    case ResourceType::BERRIES: return FText::FromString("Berries");
    case ResourceType::WOOD: return FText::FromString("Wood");
    case ResourceType::WATER: return FText::FromString("Water");
        // Add more cases as needed
    default: return FText::FromString("Unknown");
    }
}