#include "Movement/MS_PathfindingSubsystemLib.h"
#include "Movement/MS_PathfindingSubsystem.h"
#include "Engine/World.h"

void UMS_PathfindingSubsystemLib::AddNodeVector(FVector Position)
{
    UGameInstance* GameInstance = GEngine->GetWorld()->GetGameInstance();
    if (!IsValid(GameInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("AddNodeVector: No valid GameInstance found!"));
        return;
    }

    UMS_PathfindingSubsystem* PathfindingSubsystem = GameInstance->GetSubsystem<UMS_PathfindingSubsystem>();
    if (IsValid(PathfindingSubsystem))
    {
        PathfindingSubsystem->AddNodeAtPosition(Position);
    }
}

TArray<FVector> UMS_PathfindingSubsystemLib::FindPathVector(const UObject* WorldContextObject, FVector Start, FVector Goal)
{
    TArray<FVector> Path;

    if (!IsValid(WorldContextObject)) {
        ensureAlwaysMsgf(false, TEXT("No fucking valid"));
        return Path;
    }

    UWorld* world = WorldContextObject->GetWorld();
    if (!IsValid(world)) {
        UE_LOG(LogTemp, Error, TEXT("FindPathVector: No valid World found!"));
        return Path;
    }

    UGameInstance* GameInstance = world->GetGameInstance();
    if (!IsValid(GameInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("FindPathVector: No valid GameInstance found!"));
        return Path;
    }

    UMS_PathfindingSubsystem* PathfindingSubsystem = GameInstance->GetSubsystem<UMS_PathfindingSubsystem>();
    if (IsValid(PathfindingSubsystem))
    {
        TArray<TSharedPtr<FMoveNode>> NodePath = PathfindingSubsystem->FindPathNodes(PathfindingSubsystem->FindClosestNodeToPosition(Start),
            PathfindingSubsystem->FindClosestNodeToPosition(Goal));
        for (TSharedPtr<FMoveNode> Node : NodePath)
        {
            Path.Add(Node->Position); 
        }
    }

    return Path;
}


