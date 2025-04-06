// Fill out your copyright notice in the Description page of Project Settings.


#include "Movement/MS_PathfindingSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

void UMS_PathfindingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

}

void UMS_PathfindingSubsystem::Deinitialize()
{

    Super::Deinitialize();
}



void UMS_PathfindingSubsystem::SetNodeMap(TMap<FIntPoint, TSharedPtr<FMoveNode>> newNodeMap) {
    NodeMap = newNodeMap;
}

// Calculate the heuristic value between the nodes
float Heuristic(TSharedPtr<FMoveNode> NodeA, TSharedPtr<FMoveNode> NodeB)
{
    return FMath::Abs(NodeA->GridPosition.X - NodeB->GridPosition.X) +
        FMath::Abs(NodeA->GridPosition.Y - NodeB->GridPosition.Y);
}


TArray<TSharedPtr<FMoveNode>> UMS_PathfindingSubsystem::FindPathNodes(TSharedPtr<FMoveNode> StartNode, TSharedPtr<FMoveNode> GoalNode)
{
    if (!StartNode || !GoalNode)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindPath: StartNode or GoalNode is null."));
        return TArray<TSharedPtr<FMoveNode>>();
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindPath: No valid world context."));
        return TArray<TSharedPtr<FMoveNode>>();
    }

    if (bShowDebugLinesPathfinding) {
        DrawDebugSphere(World, StartNode->Position, 50.0f, 12, FColor::Green, false, 10.0f);
        DrawDebugSphere(World, GoalNode->Position, 50.0f, 12, FColor::Red, false, 10.0f);
    }
    TSet<TSharedPtr<FMoveNode>> OpenSet;       // Nodes to be evaluated
    TSet<TSharedPtr<FMoveNode>> ClosedSet;     // Nodes already evaluated


    TMap<TSharedPtr<FMoveNode>, float> GScore; // Cost from start to node
    GScore.Add(StartNode, 0.0f);


    TMap<TSharedPtr<FMoveNode>, float> FScore; // Estimated cost from start to goal via node
    FScore.Add(StartNode, Heuristic(StartNode, GoalNode));


    TMap<TSharedPtr<FMoveNode>, TSharedPtr<FMoveNode>> CameFrom;  // Path reconstruction

    TArray<TSharedPtr<FMoveNode>> PriorityQueue;   // Priority queue to process nodes in order of FScore
    PriorityQueue.Add(StartNode);

    while (PriorityQueue.Num() > 0)
    {
        // Get the node with the lowest FScore
        TSharedPtr<FMoveNode> CurrentNode = PriorityQueue[0];
        PriorityQueue.RemoveAt(0);
   
       // DrawDebugSphere(World, CurrentNode->Position, 30.0f, 12, FColor::Yellow, false, 2.0f);
       
        // Check if goal is reached
        if (CurrentNode == GoalNode)
        {
           
            TArray<TSharedPtr<FMoveNode>> Path;

            // Reconstruct the path by backtracking using CameFrom
            while (CameFrom.Contains(CurrentNode))
            {
                Path.Add(CurrentNode);            
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
        for (const TPair<TSharedPtr<FMoveNode>, bool>& NeighborPair : CurrentNode->Neighbors)
        {
            TSharedPtr<FMoveNode> Neighbor = NeighborPair.Key; // Get the actual node pointer
            bool bIsAccessible = NeighborPair.Value; // Check if path is open

            if (!bIsAccessible || ClosedSet.Contains(Neighbor))
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
        PriorityQueue.Sort([&](const TSharedPtr<FMoveNode>& A, const TSharedPtr<FMoveNode>& B)
            {
                return FScore[A] < FScore[B];
            });
    }

    // Return an empty path if no path was found
    return TArray<TSharedPtr<FMoveNode>>();
}

TArray<FIntPoint> UMS_PathfindingSubsystem::FindPathPoints(TSharedPtr<FMoveNode> StartNode, TSharedPtr<FMoveNode> GoalNode)
{
    if (!StartNode || !GoalNode)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindPath: StartNode or GoalNode is null."));
        return TArray<FIntPoint>();
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindPath: No valid world context."));
        return TArray<FIntPoint>();
    }
    TArray<TSharedPtr<FMoveNode>> NodePath = FindPathNodes(StartNode, GoalNode);
    TArray<FIntPoint> Path;

    
    for (auto MoveNode : NodePath)
    {
        Path.Add(MoveNode->GridPosition);
    }
    return Path;
    // Return an empty path if no path was found
    return TArray<FIntPoint>();
}


// Finds the closest node from a actor
TSharedPtr<FMoveNode> UMS_PathfindingSubsystem::FindClosestNodeToActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindClosestNodeToActor: TargetActor is null."));
        return nullptr;
    }

    FVector ActorLocation = TargetActor->GetActorLocation();

    float MinDistanceSquared = FLT_MAX;
    TSharedPtr<FMoveNode> ClosestNode = nullptr;

    // Iterates all the nodes until it finds the closest one
    for (const TPair<FIntPoint, TSharedPtr<FMoveNode>>& NodePair : NodeMap)
    {
        TSharedPtr<FMoveNode> Node = NodePair.Value;
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

TSharedPtr<FMoveNode> UMS_PathfindingSubsystem::FindClosestNodeToPosition(FVector position)
{

    FVector ActorLocation = position;

    float MinDistanceSquared = FLT_MAX;
    TSharedPtr<FMoveNode> ClosestNode = nullptr;

    // Iterates all the nodes until it finds the closest one
    for (const TPair<FIntPoint, TSharedPtr<FMoveNode>>& NodePair : NodeMap)
    {
        TSharedPtr<FMoveNode> Node = NodePair.Value;
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
void UMS_PathfindingSubsystem::SetNodeSeparation(int32 newSeparation) {
    NodeSeparation_ = newSeparation;
}

// Function that creates a new node on any position and connects it to the surrounding nodes
FIntPoint UMS_PathfindingSubsystem::AddNodeAtPosition(const FVector& Position)
{
    // CAlculate the grid posinton for the map
    FIntPoint GridPosition = FIntPoint(FMath::RoundToInt(Position.X ), FMath::RoundToInt(Position.Y ));

    TSharedPtr<FMoveNode> NewNode = MakeShared<FMoveNode>();
    NewNode->Position = Position;
    NewNode->GridPosition = GridPosition;

    //Check if a node already exists
    if (!NodeMap.Contains(GridPosition))
    {
        NodeMap.Add(GridPosition, NewNode);
        if (bShowDebugLinesPathfinding) {

            DrawDebugSphere(GetWorld(), Position, 50.0f, 12, FColor::Purple, false, 10.0f);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Node at %s already exists in the NodeMap."), *Position.ToString());
    }

    // connect the node to neighbours
    for (auto& Pair : NodeMap)
    {
        TSharedPtr<FMoveNode> ExistingNode = Pair.Value;
        if (FVector::Dist(NewNode->Position, ExistingNode->Position) <= NodeSeparation_ * 2)
        {

            NewNode->Neighbors.Add(ExistingNode);
            ExistingNode->Neighbors.Add(NewNode);
            if (bShowDebugLinesPathfinding) {

                DrawDebugLine(GetWorld(), NewNode->Position, ExistingNode->Position, FColor::Cyan, false, 10.0f, 0, 3.0f);
            }
        }
    }

    return GridPosition;
}


void UMS_PathfindingSubsystem::BlockNode(FVector Position)
{
    TSharedPtr<FMoveNode> Node = FindClosestNodeToPosition(Position);
    if (Node)
    {
        UE_LOG(LogTemp, Warning, TEXT("Blocking paths for node at %s"), *Position.ToString());

        // Mark paths to neighbors as blocked
        for (auto& NeighborPair : Node->Neighbors)
        {
            NeighborPair.Value = false; // Set path to blocked

            // Also mark the reverse connection as blocked
            if (NeighborPair.Key->Neighbors.Contains(Node))
            {
                NeighborPair.Key->Neighbors[Node] = false;
            }
            
            if (bShowDebugLinesPathfinding) {
                // Draw debug line in red to indicate blocked path
                DrawDebugLine(GetWorld(), Node->Position, NeighborPair.Key->Position, FColor::Red, false, 10.0f, 0, 3.0f);
            }
        }

        if (bShowDebugLinesPathfinding) {
            // Change debug color to RED for blocked node
            DrawDebugSphere(GetWorld(), Position, 50.0f, 12, FColor::Red, false, 10.0f);
        }

        OnPathUpdated.Broadcast(Node->GridPosition);
    }
}


void UMS_PathfindingSubsystem::UnblockNode(FVector Position)
{
    TSharedPtr<FMoveNode> Node = FindClosestNodeToPosition(Position);
    if (Node)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unblocking paths for node at %s"), *Position.ToString());

        for (auto& NeighborPair : Node->Neighbors)
        {
            if (PerformRaycastToPosition(Node->Position, NeighborPair.Key->Position)) // Check if path is clear
            {
                NeighborPair.Value = true; // Mark path as accessible

                // Also update the reverse connection
                if (NeighborPair.Key->Neighbors.Contains(Node))
                {
                    NeighborPair.Key->Neighbors[Node] = true;
                }

                if (bShowDebugLinesPathfinding) {
                    // Draw debug line in blue for open path
                    DrawDebugLine(GetWorld(), Node->Position, NeighborPair.Key->Position, FColor::Blue, false, 10.0f, 0, 3.0f);
                }
            }
        }

        if (bShowDebugLinesPathfinding) {
            // Change debug color to GREEN for unblocked node
            DrawDebugSphere(GetWorld(), Position, 50.0f, 12, FColor::Green, false, 10.0f);
        }

        OnPathUpdated.Broadcast(Node->GridPosition);
    }
}


bool UMS_PathfindingSubsystem::PerformRaycastToPosition(const FVector& Start, const FVector& End)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FVector StartPos = Start + FVector(0, 0, 100.0f);
    FVector EndPos = End + FVector(0, 0, 100.0f);

    FHitResult HitResult;
    FCollisionQueryParams Params;


    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartPos,
        EndPos,
        ECC_EngineTraceChannel3,
        Params
    );

    if (bHit)
    {
        UE_LOG(LogTemp, Log, TEXT("Hit: %s"), *HitResult.GetActor()->GetName());
        DrawDebugLine(World, StartPos, EndPos, FColor::Red, false, 2.0f);
        return false;
    }
    else
    {
        //  DrawDebugLine(World, StartPos, EndPos,  FColor::Green, false, 2.0f);
        return true;
    }


    return false;
}

TSharedPtr<FMoveNode> UMS_PathfindingSubsystem::FindNodeByGridPosition(const FIntPoint& GridPosition)
{

    if (NodeMap.Contains(GridPosition))
    {
       
        return NodeMap[GridPosition];
    }
    

    return nullptr;
}

bool UMS_PathfindingSubsystem::GetRandomFreeNode(FVector& OutLocation, FIntPoint& OutGrid)
{
    TArray<FIntPoint> Keys;
    NodeMap.GetKeys(Keys);

    if (Keys.Num() == 0) return false;

    int32 MaxTries = 20;
    for (int32 i = 0; i < MaxTries; ++i)
    {
        int32 Index = FMath::RandRange(0, Keys.Num() - 1);
        FIntPoint RandomKey = Keys[Index];

        TSharedPtr<FMoveNode> Node = NodeMap.FindRef(RandomKey);
        if (Node.IsValid() && Node->Neighbors.Num()>0)
        {
            OutGrid = RandomKey;
            OutLocation = FVector(RandomKey.X * NodeSeparation_, RandomKey.Y * NodeSeparation_, 0.f);
            return true;
        }
    }

    return false; // No free node found in MaxTries
}
