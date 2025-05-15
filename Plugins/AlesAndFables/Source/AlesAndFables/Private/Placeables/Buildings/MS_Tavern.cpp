#include "Placeables/Buildings/MS_Tavern.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

AMS_Tavern::AMS_Tavern()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootTavernComponent"));
	RootComponent = RootSceneComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TavernMesh"));
	MeshComponent->SetupAttachment(RootComponent);
	// Assign a default mesh for the Tavern in Blueprint or here

	InteractionPoint = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPoint"));
	InteractionPoint->SetupAttachment(RootComponent);
	// Position this interaction point relative to the root in BP/Editor
}

void AMS_Tavern::BeginPlay()
{
	Super::BeginPlay();
}