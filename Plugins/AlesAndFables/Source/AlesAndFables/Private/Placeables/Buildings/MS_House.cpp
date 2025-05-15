// Fill out your copyright notice in the Description page of Project Settings.

#include "Placeables/Buildings/MS_House.h" 
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "AI/Characters/MS_AICharacter.h" 

AMS_House::AMS_House()
{
	PrimaryActorTick.bCanEverTick = false; 

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootHouseComponent"));
    RootComponent = RootSceneComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HouseMesh"));
	MeshComponent->SetupAttachment(RootComponent);
}

void AMS_House::BeginPlay()
{
	Super::BeginPlay();
	CurrentOccupantCount = 0;
    OccupyingCharacters.Empty();
}

bool AMS_House::TryEnterHouse(AMS_AICharacter* Character)
{
	if (!Character) return false;

	if (HasSpace())
	{
		CurrentOccupantCount++;
        OccupyingCharacters.AddUnique(Character); 
		UE_LOG(LogTemp, Log, TEXT("House %s: Character %s entered. Occupants: %d/%d"), *GetName(), *Character->GetName(), CurrentOccupantCount, MaxOccupants);
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("House %s: Character %s tried to enter, but house is full (%d/%d)."), *GetName(), *Character->GetName(), CurrentOccupantCount, MaxOccupants);
	return false;
}

void AMS_House::LeaveHouse(AMS_AICharacter* Character)
{
	if (!Character) return;

    if (OccupyingCharacters.Contains(Character)) 
    {
	    CurrentOccupantCount = FMath::Max(0, CurrentOccupantCount - 1);
        OccupyingCharacters.Remove(Character);
	    UE_LOG(LogTemp, Log, TEXT("House %s: Character %s left. Occupants: %d/%d"), *GetName(), *Character->GetName(), CurrentOccupantCount, MaxOccupants);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("House %s: Character %s tried to leave, but was not registered as an occupant."), *GetName(), *Character->GetName());
    }
}

bool AMS_House::HasSpace() const
{
	return CurrentOccupantCount < MaxOccupants;
}