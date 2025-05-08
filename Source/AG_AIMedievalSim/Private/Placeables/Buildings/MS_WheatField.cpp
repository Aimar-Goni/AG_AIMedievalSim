#include "Placeables/Buildings//MS_WheatField.h" // Adjust path
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "AI/Manager/MS_AIManager.h" // Adjust path
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h" // Include for mesh setting

AMS_WheatField::AMS_WheatField()
{
	PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootFieldComponent"));
    RootComponent = RootSceneComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FieldMesh"));
	MeshComponent->SetupAttachment(RootComponent);
	// Assign MeshState_Constructed as the default mesh in Blueprint/Editor
}

void AMS_WheatField::BeginPlay()
{
	Super::BeginPlay();
    InitializeAIManager();
	// Start in Constructed state when placed/built
	ChangeState(EFieldState::Constructed);
}

void AMS_WheatField::InitializeAIManager()
{
     AActor* FoundManager = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_AIManager::StaticClass());
     AIManager = Cast<AMS_AIManager>(FoundManager);
     if (!AIManager.IsValid())
     {
         UE_LOG(LogTemp, Error, TEXT("WheatField %s: Failed to find AIManager!"), *GetName());
     }
}

void AMS_WheatField::ChangeState(EFieldState NewState)
{
    if (CurrentState == NewState) return; // No change

    UE_LOG(LogTemp, Log, TEXT("WheatField %s: Changing state from %s to %s."),
        *GetName(), *UEnum::GetValueAsString(CurrentState), *UEnum::GetValueAsString(NewState));

    CurrentState = NewState;

    // --- Update Visual Mesh ---
    UStaticMesh* MeshToSet = nullptr;
    switch (CurrentState)
    {
        case EFieldState::Constructed: MeshToSet = MeshState_Constructed; break;
        case EFieldState::Planted:     MeshToSet = MeshState_Planted; break; // Optional different mesh
        case EFieldState::Watered:     MeshToSet = MeshState_Planted; break; // Keep planted mesh while waiting
        case EFieldState::Sprouting:   MeshToSet = MeshState_Sprouting; break;
        case EFieldState::Growing:     MeshToSet = MeshState_Growing; break;
        case EFieldState::ReadyToHarvest: MeshToSet = MeshState_ReadyToHarvest; break;
        default: MeshToSet = MeshState_Constructed; // Fallback
    }
    if(MeshComponent) MeshComponent->SetStaticMesh(MeshToSet); // SetStaticMesh(nullptr) is valid to hide it

    // --- Clear Previous Timers ---
    GetWorldTimerManager().ClearTimer(WateredTimerHandle);
    GetWorldTimerManager().ClearTimer(SproutTimerHandle);
    GetWorldTimerManager().ClearTimer(GrowthTimerHandle);

    // --- Broadcast Events & Start Next Timers ---
    switch (CurrentState)
    {
        case EFieldState::Constructed:
            OnFieldNeedsPlanting.Broadcast(this);
            break;
        case EFieldState::Planted:
            OnFieldNeedsWatering.Broadcast(this);
            break;
        case EFieldState::Watered:
            GetWorldTimerManager().SetTimer(WateredTimerHandle, this, &AMS_WheatField::OnWateredTimerComplete, WateredToSproutDuration, false);
            break;
        case EFieldState::Sprouting:
             GetWorldTimerManager().SetTimer(SproutTimerHandle, this, &AMS_WheatField::OnSproutTimerComplete, SproutToGrowthDuration, false);
            break;
        case EFieldState::Growing:
             GetWorldTimerManager().SetTimer(GrowthTimerHandle, this, &AMS_WheatField::OnGrowthTimerComplete, GrowthToHarvestDuration, false);
            break;
        case EFieldState::ReadyToHarvest:
            OnFieldReadyToHarvest.Broadcast(this);
            break;
        default: break;
    }
}


// --- AI Interaction ---

bool AMS_WheatField::PlantSeeds()
{
    if (CurrentState == EFieldState::Constructed)
    {
        ChangeState(EFieldState::Planted);
        return true;
    }
    UE_LOG(LogTemp, Warning, TEXT("WheatField %s: Tried to plant seeds but state is %s."), *GetName(), *UEnum::GetValueAsString(CurrentState));
    return false;
}

bool AMS_WheatField::WaterField()
{
    if (CurrentState == EFieldState::Planted)
    {
        ChangeState(EFieldState::Watered);
        return true;
    }
    UE_LOG(LogTemp, Warning, TEXT("WheatField %s: Tried to water field but state is %s."), *GetName(), *UEnum::GetValueAsString(CurrentState));
    return false;
}

FResource AMS_WheatField::HarvestField()
{
	if (CurrentState == EFieldState::ReadyToHarvest)
	{
        FResource Yield = FResource{ResourceType::WHEAT, HarvestAmount};
		ChangeState(EFieldState::Constructed); // Reset to empty after harvest
		UE_LOG(LogTemp, Log, TEXT("WheatField %s: Harvested. Yielded %d Wheat. Resetting to Constructed."), *GetName(), HarvestAmount);
		return Yield;
	}
	UE_LOG(LogTemp, Warning, TEXT("WheatField %s: Tried to harvest but not ready (State: %s)."), *GetName(), *UEnum::GetValueAsString(CurrentState));
	return FResource{ResourceType::ERROR, 0};
}

// --- Timer Callbacks ---

void AMS_WheatField::OnWateredTimerComplete()
{
    if (CurrentState == EFieldState::Watered)
    {
        ChangeState(EFieldState::Sprouting);
    }
}

void AMS_WheatField::OnSproutTimerComplete()
{
     if (CurrentState == EFieldState::Sprouting)
    {
        ChangeState(EFieldState::Growing);
    }
}

void AMS_WheatField::OnGrowthTimerComplete()
{
	if (CurrentState == EFieldState::Growing)
	{
		ChangeState(EFieldState::ReadyToHarvest);
	}
}