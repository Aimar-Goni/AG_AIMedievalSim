// Fill out your copyright notice in the Description page of Project Settings.


#include "Movement/MS_PathfindingSubsyste.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h" 
#include "Engine/World.h"

void UMS_PathfindingSubsyste::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

}

void UMS_PathfindingSubsyste::Deinitialize()
{

    Super::Deinitialize();
}

void UMS_PathfindingSubsyste::SetNodeMap(TMap<FIntPoint, FNode*> newNodeMap) {
    NodeMap = newNodeMap;
}


float Heuristic(FNode* NodeA, FNode* NodeB)
{
    return FMath::Abs(NodeA->GridPosition.X - NodeB->GridPosition.X) +
        FMath::Abs(NodeA->GridPosition.Y - NodeB->GridPosition.Y);
}

TArray<FNode*> UMS_PathfindingSubsyste::FindPath(FNode* StartNode, FNode* GoalNode)
{
    
    return TArray<FNode*>();
}




FNode* UMS_PathfindingSubsyste::FindClosestNodeToActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindClosestNodeToActor: TargetActor is null."));
        return nullptr;
    }

    FVector ActorLocation = TargetActor->GetActorLocation();

    float MinDistanceSquared = FLT_MAX;
    FNode* ClosestNode = nullptr;

    // Iterate over all nodes
    for (const TPair<FIntPoint, FNode*>& NodePair : NodeMap)
    {
        FNode* Node = NodePair.Value;
        float DistanceSquared = FVector::DistSquared(Node->Position, ActorLocation);

        if (DistanceSquared < MinDistanceSquared)
        {
            MinDistanceSquared = DistanceSquared;
            ClosestNode = Node;
        }
    }

    if (ClosestNode)
    {
        return ClosestNode;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No nodes found in NodeMap."));
        return nullptr;
    }
}
