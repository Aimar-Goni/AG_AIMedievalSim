
#include "Placeables/Buildings/MS_WheatField.h" 
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "AI/Manager/MS_AIManager.h" 
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"

AMS_WheatField::AMS_WheatField()
{
	PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootFieldComponent"));
    RootComponent = RootSceneComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FieldMesh"));
	MeshComponent->SetupAttachment(RootComponent);
}

void AMS_WheatField::BeginPlay()
{
	Super::BeginPlay();
    InitializeAIManager();
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
    AIManager->InitializeFieldListeners();
}

void AMS_WheatField::ChangeState(EFieldState NewState)
{
    if (CurrentState == NewState && NewState != EFieldState::Constructed) return; // Allow resetting to Constructed

    UE_LOG(LogTemp, Log, TEXT("WheatField %s: Changing state from %s to %s."),
        *GetName(), *UEnum::GetValueAsString(CurrentState), *UEnum::GetValueAsString(NewState));

    CurrentState = NewState;

    //  Update Visual Mesh 
    UStaticMesh* MeshToSet = nullptr;
    switch (CurrentState)
    {
        case EFieldState::Constructed: MeshToSet = MeshState_Constructed; break;
        case EFieldState::Planted:     MeshToSet = MeshState_Planted;     break;
        case EFieldState::Watered:     MeshToSet = MeshState_Watered;     break; // Still looks planted until growth
        case EFieldState::Growing:     MeshToSet = MeshState_Growing;     break;
        case EFieldState::ReadyToHarvest: MeshToSet = MeshState_ReadyToHarvest; break;
        case EFieldState::Harvested:   MeshToSet = MeshState_Harvested; break; // Same as constructed after harvest
        default: MeshToSet = MeshState_Constructed;
    }
    if(MeshComponent) MeshComponent->SetStaticMesh(MeshToSet);

    //  Clear Previous Timers 
    GetWorldTimerManager().ClearTimer(GrowthCycleTimerHandleWatered);
    GetWorldTimerManager().ClearTimer(GrowthCycleTimerHandleGrowing);

    //  Broadcast Events & Start Next Timers 
    switch (CurrentState)
    {
        case EFieldState::Constructed:
            // After construction OR after being harvested, it needs planting.
            OnFieldNeedsPlanting.Broadcast(this);
            break;
        case EFieldState::Planted:
            OnFieldNeedsWatering.Broadcast(this);
            break;
        case EFieldState::Watered:
            // Start the single growth timer that will transition through Growing to ReadyToHarvest
            GetWorldTimerManager().SetTimer(GrowthCycleTimerHandleWatered, this, &AMS_WheatField::OnWateringGrowthTimerComplete, WateredToGrowingDuration, false);
            UE_LOG(LogTemp, Log, TEXT("WheatField %s: Watered. Will be ready for harvest in %.1f s."), *GetName(), WateredToGrowingDuration);
            break;
        case EFieldState::Growing:
            GetWorldTimerManager().SetTimer(GrowthCycleTimerHandleGrowing, this, &AMS_WheatField::OnGrowingTimerComplete, GrowingToReadyDuration, false);
        UE_LOG(LogTemp, Log, TEXT("WheatField %s: Watered. Will be ready for harvest in %.1f s."), *GetName(), GrowingToReadyDuration);
            break;
        case EFieldState::ReadyToHarvest:
            OnFieldReadyToHarvest.Broadcast(this);
            break;
        case EFieldState::Harvested:
            OnFieldNeedsPlanting.Broadcast(this);
            break;
        default: break;
    }
}


//  AI Interaction 

bool AMS_WheatField::PerformPlanting()
{
    if (CurrentState == EFieldState::Constructed)
    {
        ChangeState(EFieldState::Planted);
        return true;
    }
    UE_LOG(LogTemp, Warning, TEXT("WheatField %s: Tried to plant but state is %s (Expected Constructed)."), *GetName(), *UEnum::GetValueAsString(CurrentState));
    return false;
}

bool AMS_WheatField::PerformWatering()
{
    if (CurrentState == EFieldState::Planted)
    {
        ChangeState(EFieldState::Watered);
        return true;
    }
    UE_LOG(LogTemp, Warning, TEXT("WheatField %s: Tried to water field but state is %s (Expected Planted)."), *GetName(), *UEnum::GetValueAsString(CurrentState));
    return false;
}

FResource AMS_WheatField::PerformHarvesting()
{
	if (CurrentState == EFieldState::ReadyToHarvest)
	{
        FResource Yield = FResource{ResourceType::WHEAT, HarvestAmount};
		ChangeState(EFieldState::Harvested); // This will then transition to Constructed & broadcast NeedsPlanting
		return Yield;
	}
	UE_LOG(LogTemp, Warning, TEXT("WheatField %s: Tried to harvest but not ready (State: %s)."), *GetName(), *UEnum::GetValueAsString(CurrentState));
	return FResource{ResourceType::ERROR, 0};
}

//  Timer Callback 

void AMS_WheatField::OnWateringGrowthTimerComplete()
{

    if (CurrentState == EFieldState::Watered) 
    {
        ChangeState(EFieldState::Growing);
    }

}

void AMS_WheatField::OnGrowingTimerComplete()
{


    if (CurrentState == EFieldState::Growing)
    {
        ChangeState(EFieldState::ReadyToHarvest);
    }
}