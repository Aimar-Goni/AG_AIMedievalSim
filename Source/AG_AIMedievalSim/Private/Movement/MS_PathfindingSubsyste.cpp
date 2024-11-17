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
    TSet<FNode*> OpenSet;
    TSet<FNode*> ClosedSet;

    StartNode->GCost = 0.0f;
    StartNode->HCost = Heuristic(StartNode, GoalNode);
    StartNode->Parent = nullptr;

    OpenSet.Add(StartNode);

    while (OpenSet.Num() > 0)
    {
        // Get the node in OpenSet with the lowest FCost
        FNode* CurrentNode = nullptr;
        float LowestFCost = FLT_MAX;

        for (FNode* Node : OpenSet)
        {
            if (Node->FCost() < LowestFCost ||
                (Node->FCost() == LowestFCost && Node->HCost < CurrentNode->HCost))
            {
                LowestFCost = Node->FCost();
                CurrentNode = Node;
            }
        }

        if (CurrentNode == GoalNode)
        {
            // Path found
            return ReconstructPath(StartNode, GoalNode);
        }

        OpenSet.Remove(CurrentNode);
        ClosedSet.Add(CurrentNode);

        for (FNode* Neighbor : CurrentNode->Neighbors)
        {
            if (ClosedSet.Contains(Neighbor))
                continue;

            float TentativeGCost = CurrentNode->GCost + 1.0f; // Assuming uniform cost between nodes

            if (!OpenSet.Contains(Neighbor) || TentativeGCost < Neighbor->GCost)
            {
                Neighbor->GCost = TentativeGCost;
                Neighbor->HCost = Heuristic(Neighbor, GoalNode);
                Neighbor->Parent = CurrentNode;

                if (!OpenSet.Contains(Neighbor))
                    OpenSet.Add(Neighbor);
            }
        }
    }

    // No path found
    return TArray<FNode*>();
}

TArray<FNode*> UMS_PathfindingSubsyste::ReconstructPath(FNode* StartNode, FNode* EndNode)
{
    TArray<FNode*> Path;
    FNode* CurrentNode = EndNode;

    while (CurrentNode != StartNode)
    {
        Path.Insert(CurrentNode, 0); // Insert at the beginning
        CurrentNode = CurrentNode->Parent;
    }
    Path.Insert(StartNode, 0);

    return Path;
}


FNode* UMS_PathfindingSubsyste::FindClosestNodeToActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindClosestNodeToActor: TargetActor is null."));
        return nullptr;
    }

    FVector ActorLocation = TargetActor->GetActorLocation();

    // Assuming each grid cell is 500 units apart (consistent with your GenerateNodes function)
    const float GridSize = 500.0f;

    // Calculate the grid position of the actor
    int32 GridX = FMath::RoundToInt(ActorLocation.X / GridSize);
    int32 GridY = FMath::RoundToInt(ActorLocation.Y / GridSize);
    FIntPoint ActorGridPosition(GridX, GridY);

    // Check for the node at this grid position
    FNode** FoundNodePtr = NodeMap.Find(ActorGridPosition);
    if (FoundNodePtr)
    {
        return *FoundNodePtr;
    }
    else
    {
        // Search neighboring grid cells if the exact position isn't available
        const int32 SearchRadius = 1; // Adjust this radius based on your needs

        float MinDistanceSquared = FLT_MAX;
        FNode* ClosestNode = nullptr;

        for (int32 DX = -SearchRadius; DX <= SearchRadius; ++DX)
        {
            for (int32 DY = -SearchRadius; DY <= SearchRadius; ++DY)
            {
                FIntPoint NeighborGridPos = ActorGridPosition + FIntPoint(DX, DY);
                FNode** NeighborNodePtr = NodeMap.Find(NeighborGridPos);
                if (NeighborNodePtr)
                {
                    FNode* NeighborNode = *NeighborNodePtr;
                    float DistanceSquared = FVector::DistSquared(NeighborNode->Position, ActorLocation);
                    if (DistanceSquared < MinDistanceSquared)
                    {
                        MinDistanceSquared = DistanceSquared;
                        ClosestNode = NeighborNode;
                    }
                }
            }
        }

        if (ClosestNode)
        {
            return ClosestNode;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No nearby nodes found for actor %s."), *TargetActor->GetName());
            return nullptr;
        }
    }
}
