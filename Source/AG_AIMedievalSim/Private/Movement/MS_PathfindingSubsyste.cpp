// Fill out your copyright notice in the Description page of Project Settings.


#include "Movement/MS_PathfindingSubsyste.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h" 
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

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

// Calculate the heuristic value between the nodes
float Heuristic(FNode* NodeA, FNode* NodeB)
{
    return FMath::Abs(NodeA->GridPosition.X - NodeB->GridPosition.X) +
        FMath::Abs(NodeA->GridPosition.Y - NodeB->GridPosition.Y);
}


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


  //  DrawDebugSphere(World, StartNode->Position, 50.0f, 12, FColor::Green, false, 10.0f);
   // DrawDebugSphere(World, GoalNode->Position, 50.0f, 12, FColor::Red, false, 10.0f);


    TSet<FNode*> OpenSet;       // Nodes to be evaluated
    TSet<FNode*> ClosedSet;     // Nodes already evaluated


    TMap<FNode*, float> GScore; // Cost from start to node
    GScore.Add(StartNode, 0.0f);


    TMap<FNode*, float> FScore; // Estimated cost from start to goal via node
    FScore.Add(StartNode, Heuristic(StartNode, GoalNode));


    TMap<FNode*, FNode*> CameFrom;  // Path reconstruction

    TArray<FNode*> PriorityQueue;   // Priority queue to process nodes in order of FScore
    PriorityQueue.Add(StartNode);

    while (PriorityQueue.Num() > 0)
    {
        // Get the node with the lowest FScore
        FNode* CurrentNode = PriorityQueue[0];
        PriorityQueue.RemoveAt(0);
   
       // DrawDebugSphere(World, CurrentNode->Position, 30.0f, 12, FColor::Yellow, false, 2.0f);
       
        // Check if goal is reached
        if (CurrentNode == GoalNode)
        {
           
            TArray<FNode*> Path;

            // Reconstruct the path by backtracking using CameFrom
            while (CameFrom.Contains(CurrentNode))
            {
                Path.Add(CurrentNode);            
               /* if (CameFrom[CurrentNode])
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
                }*/
                CurrentNode = CameFrom[CurrentNode];
            }
            Path.Add(StartNode);
            // Reverse path for correct order
            Algo::Reverse(Path);
            return Path;
        }

        // Mark node as evaluated
        ClosedSet.Add(CurrentNode);

        // Explore neighbors
        for (FNode* Neighbor : CurrentNode->Neighbors)
        {
            if (ClosedSet.Contains(Neighbor))
                continue;

            float TentativeGScore = GScore[CurrentNode] + FVector::Dist(CurrentNode->Position, Neighbor->Position);

            // Check if this path is better or if it's unexplored
            if (!GScore.Contains(Neighbor) || TentativeGScore < GScore[Neighbor])
            {
                CameFrom.Add(Neighbor, CurrentNode);
                GScore.Add(Neighbor, TentativeGScore);
                FScore.Add(Neighbor, TentativeGScore + FVector::Dist(CurrentNode->Position, GoalNode->Position));

                // Add neighbor to the queue if not already there
                if (!PriorityQueue.Contains(Neighbor))
                {
                    PriorityQueue.Add(Neighbor);

                    //DrawDebugSphere(World, Neighbor->Position, 30.0f, 12, FColor::Cyan, false, 2.0f);
                }
            }
        }

        // Sort the priority queue by FScore
        PriorityQueue.Sort([&](const FNode& A, const FNode& B)
            {
                return FScore[&A] < FScore[&B];
            });
    }

    // Return an empty path if no path was found
    return TArray<FNode*>();
}


// Finds the closest node from a actor
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

    // Iterates all the nodes until it finds the closest one
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

// Setter for the node separation
void UMS_PathfindingSubsyste::SetNodeSeparation(int32 newSeparation) {
    NodeSeparation_ = newSeparation;
}

// Function that creates a new node on any position and connects it to the surrounding nodes
FIntPoint UMS_PathfindingSubsyste::AddNodeAtPosition(const FVector& Position)
{
    // CAlculate the grid posinton for the map
    FIntPoint GridPosition = FIntPoint(FMath::RoundToInt(Position.X ), FMath::RoundToInt(Position.Y ));

    FNode* NewNode = new FNode();
    NewNode->Position = Position;
    NewNode->GridPosition = GridPosition;

    //Check if a node already exists
    if (!NodeMap.Contains(GridPosition))
    {
        NodeMap.Add(GridPosition, NewNode);
       // DrawDebugSphere(GetWorld(), Position, 50.0f, 12, FColor::Purple, false, 10.0f);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Node at %s already exists in the NodeMap."), *Position.ToString());
    }

    // connect the node to neighbours
    for (auto& Pair : NodeMap)
    {
        FNode* ExistingNode = Pair.Value;
        if (FVector::Dist(NewNode->Position, ExistingNode->Position) <= NodeSeparation_ * 2)
        {

            NewNode->Neighbors.Add(ExistingNode);
            ExistingNode->Neighbors.Add(NewNode);

            //DrawDebugLine(GetWorld(), NewNode->Position, ExistingNode->Position, FColor::Cyan, false, 10.0f, 0, 3.0f);
        }
    }

    return GridPosition;
}
