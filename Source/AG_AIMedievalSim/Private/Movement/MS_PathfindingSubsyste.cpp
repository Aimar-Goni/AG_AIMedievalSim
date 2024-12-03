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

#include "DrawDebugHelpers.h"

TArray<FNode*> UMS_PathfindingSubsyste::FindPath(FNode* StartNode, FNode* GoalNode)
{
    if (!StartNode || !GoalNode)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindPath: StartNode or GoalNode is null."));
        return TArray<FNode*>();
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindPath: No valid world context."));
        return TArray<FNode*>();
    }


    DrawDebugSphere(World, StartNode->Position, 50.0f, 12, FColor::Green, false, 10.0f);
    DrawDebugSphere(World, GoalNode->Position, 50.0f, 12, FColor::Red, false, 10.0f);


    TSet<FNode*> OpenSet;
    TSet<FNode*> ClosedSet;


    TMap<FNode*, float> GScore;
    GScore.Add(StartNode, 0.0f);


    TMap<FNode*, float> FScore;
    FScore.Add(StartNode, Heuristic(StartNode, GoalNode));


    TMap<FNode*, FNode*> CameFrom;

    TArray<FNode*> PriorityQueue;
    PriorityQueue.Add(StartNode);

    while (PriorityQueue.Num() > 0)
    {
        
        FNode* CurrentNode = PriorityQueue[0];
        PriorityQueue.RemoveAt(0);

       
        DrawDebugSphere(World, CurrentNode->Position, 30.0f, 12, FColor::Yellow, false, 2.0f);

        if (CurrentNode == GoalNode)
        {
           
            TArray<FNode*> Path;
            while (CameFrom.Contains(CurrentNode))
            {
                Path.Add(CurrentNode);

            
                if (CameFrom[CurrentNode])
                {
                    DrawDebugLine(
                        World,
                        CurrentNode->Position,
                        CameFrom[CurrentNode]->Position,
                        FColor::Blue,
                        false,
                        10.0f,
                        0,
                        5.0f
                    );
                }

                CurrentNode = CameFrom[CurrentNode];
            }
            Path.Add(StartNode);
            Algo::Reverse(Path);
            return Path;
        }

        ClosedSet.Add(CurrentNode);

        // Explore neighbors
        for (FNode* Neighbor : CurrentNode->Neighbors)
        {
            if (ClosedSet.Contains(Neighbor))
                continue;

            float TentativeGScore = GScore[CurrentNode] + FVector::Dist(CurrentNode->Position, Neighbor->Position);

            if (!GScore.Contains(Neighbor) || TentativeGScore < GScore[Neighbor])
            {
                CameFrom.Add(Neighbor, CurrentNode);
                GScore.Add(Neighbor, TentativeGScore);
                FScore.Add(Neighbor, TentativeGScore + FVector::Dist(CurrentNode->Position, GoalNode->Position));

                if (!PriorityQueue.Contains(Neighbor))
                {
                    PriorityQueue.Add(Neighbor);

                    DrawDebugSphere(World, Neighbor->Position, 30.0f, 12, FColor::Cyan, false, 2.0f);
                }
            }
        }


        PriorityQueue.Sort([&](const FNode& A, const FNode& B)
            {
                return FScore[&A] < FScore[&B];
            });
    }

    // Return an empty path if no path was found
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

void UMS_PathfindingSubsyste::SetNodeSeparation(int32 newSeparation) {
    NodeSeparation_ = newSeparation;
}

FIntPoint UMS_PathfindingSubsyste::AddNodeAtPosition(const FVector& Position)
{

    FIntPoint GridPosition = FIntPoint(FMath::RoundToInt(Position.X / NodeSeparation_), FMath::RoundToInt(Position.Y / NodeSeparation_));

    FNode* NewNode = new FNode();
    NewNode->Position = Position;
    NewNode->GridPosition = GridPosition;


    if (!NodeMap.Contains(GridPosition))
    {
        NodeMap.Add(GridPosition, NewNode);


        DrawDebugSphere(GetWorld(), Position, 50.0f, 12, FColor::Purple, false, 10.0f);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Node at %s already exists in the NodeMap."), *Position.ToString());
    }

    for (auto& Pair : NodeMap)
    {
        FNode* ExistingNode = Pair.Value;


        if (FVector::Dist(NewNode->Position, ExistingNode->Position) <= NodeSeparation_ * 2)
        {

            NewNode->Neighbors.Add(ExistingNode);
            ExistingNode->Neighbors.Add(NewNode);

            DrawDebugLine(GetWorld(), NewNode->Position, ExistingNode->Position, FColor::Cyan, false, 10.0f, 0, 3.0f);
        }
    }

    return GridPosition;
}
