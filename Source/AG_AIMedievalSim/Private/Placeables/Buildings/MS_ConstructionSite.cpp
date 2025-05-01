#include "Placeables/Buildings/MS_ConstructionSite.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AI/Manager/MS_AIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Movement/MS_PathfindingSubsystem.h"


AMS_ConstructionSite::AMS_ConstructionSite()
{
	// Set this actor to call Tick() noneeded
	PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    // Assign a default scaffolding mesh in Blueprint or here if desired
}

void AMS_ConstructionSite::BeginPlay()
{
	Super::BeginPlay();
	InitializeAIManager();
    CurrentAmount = 0; // Ensure starts at 0
}

void AMS_ConstructionSite::InitializeAIManager()
{
     AActor* FoundManager = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_AIManager::StaticClass());
     AIManager = Cast<AMS_AIManager>(FoundManager);
     if (!AIManager.IsValid())
     {
         UE_LOG(LogTemp, Error, TEXT("ConstructionSite %s: Failed to find AIManager!"), *GetName());
     }
}

bool AMS_ConstructionSite::AddResource(int32 AmountToAdd)
{
    if (AmountToAdd <= 0) return false;
    if (CurrentAmount >= AmountRequired) return false; // Already complete

    CurrentAmount = FMath::Min(CurrentAmount + AmountToAdd, AmountRequired);

    UE_LOG(LogTemp, Log, TEXT("ConstructionSite %s: Received %d %s. Progress: %d / %d"),
        *GetName(), AmountToAdd, *UEnum::GetValueAsString(RequiredResource), CurrentAmount, AmountRequired);

    // Notify manager of progress
    if(AIManager.IsValid())
    {
        AIManager->NotifyConstructionProgress(this, AmountToAdd);
    }

    // Check for completion
    if (CurrentAmount >= AmountRequired)
    {
        CompleteConstruction();
        return true; // Reached completion
    }
    return false; // Still needs more
}

void AMS_ConstructionSite::CompleteConstruction()
{
    UE_LOG(LogTemp, Log, TEXT("ConstructionSite %s: Reached required resources (%d). Completing construction."), *GetName(), AmountRequired);

    UWorld* World = GetWorld();
    if (World && BuildingClassToSpawn)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AActor* NewBuilding = World->SpawnActor<AActor>(BuildingClassToSpawn, GetActorLocation(), GetActorRotation(), SpawnParams);

        if (NewBuilding)
        {
            UE_LOG(LogTemp, Log, TEXT("ConstructionSite %s: Successfully spawned final building %s."), *GetName(), *NewBuilding->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ConstructionSite %s: Failed to spawn final building class %s!"), *GetName(), *BuildingClassToSpawn->GetName());
            BeginDestroy(); 
            return; 
        }
    }
    else
    {
         UE_LOG(LogTemp, Error, TEXT("ConstructionSite %s: Cannot complete construction - World or BuildingClassToSpawn invalid."), *GetName());
         BeginDestroy(); // Clean up nodes if completion fails immediately
         return;
    }
    
    Destroy();
}

void AMS_ConstructionSite::BeginDestroy()
{
     UE_LOG(LogTemp, Log, TEXT("ConstructionSite %s: BeginDestroy called. Unblocking nodes."), *GetName());

  
    // UMS_PathfindingSubsystem* PathSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
    // if (PathSubsystem && OccupiedNodes.Num() > 0)
    // {
    //     for (const FIntPoint& NodePos : OccupiedNodes)
    //     {
    //         TSharedPtr<FMoveNode> Node = PathSubsystem->FindNodeByGridPosition(NodePos);
    //         if (Node.IsValid())
    //         {
    //              PathSubsystem->UnblockNode(Node->Position); // Assuming UnblockNode exists
    //              // UE_LOG(LogTemp, Verbose, TEXT("ConstructionSite %s: Unblocking node %s."), *GetName(), *NodePos.ToString());
    //         }
    //     }
    //      OccupiedNodes.Empty(); // Clear the list
    // }
    
    Super::BeginDestroy();
}