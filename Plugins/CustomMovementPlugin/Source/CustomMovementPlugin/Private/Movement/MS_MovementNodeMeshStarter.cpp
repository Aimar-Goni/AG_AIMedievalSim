// Fill out your copyright notice in the Description page of Project Settings.

#include "Movement/MS_MovementNodeMeshStarter.h"
#include "DrawDebugHelpers.h" 
#include "Kismet/KismetSystemLibrary.h" 
#include "Movement/MS_PathfindingSubsystem.h"

// Sets default values
AMS_MovementNodeMeshStarter::AMS_MovementNodeMeshStarter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMS_MovementNodeMeshStarter::BeginPlay()
{
	Super::BeginPlay();
	
	UWorld* World = GetWorld();
	if (!World)
		return;

    FVector Start = GetActorLocation() + FVector(0, 0, 500.0f);
    FVector End = GetActorLocation() - FVector(0, 0, 30.0f);


    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this); 

    bool bHit = World->LineTraceSingleByProfile(
        HitResult,        
        Start,           
        End,              
        TEXT("TestFloor"),
        Params            
    );

    if (bShowDebugLinesStarter) {
        // Debug draw
        DrawDebugLine(World, Start, End, bHit ? FColor::Green : FColor::Red, false, 2.0f);
    }


    if (bHit)
    {

        UE_LOG(LogTemp, Log, TEXT("Hit: %s"), *HitResult.GetActor()->GetName());
       
        if (HitResult.GetActor()->ActorHasTag("Floor"))
        {
            UE_LOG(LogTemp, Log, TEXT("The hit actor has the tag: Floor"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("The hit actor does NOT have the tag: Floor"));
        }

        // Check if the component has the "Floor" tag
        if (HitResult.Component->ComponentHasTag("Floor"))
        {
            UE_LOG(LogTemp, Log, TEXT("The hit component has the tag: Floor"));
            GenerateNodes(HitResult.ImpactPoint);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("The hit component does NOT have the tag: Floor"));
        }

    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No hit detected."));
    }


    // Send the data to the pathfinding subsystem
    UMS_PathfindingSubsystem* PathfindingSubsystem = GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
    if (PathfindingSubsystem)
    {
        PathfindingSubsystem->SetNodeMap(NodeMap);
        PathfindingSubsystem->SetNodeSeparation(NodeSeparationX_);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PathfindingSubsystem not found."));
    }

    // Send the delegate indicating its ready
    OnNodeMapReady.Broadcast();

    GetWorld()->GetTimerManager().SetTimer(
        PathCheckTimer, this, &AMS_MovementNodeMeshStarter::UpdateBlockedPaths, 2.0f, true
    );




}

// Called every frame
void AMS_MovementNodeMeshStarter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// Does a vertical raycast to check if the position is avaliable
bool AMS_MovementNodeMeshStarter::PerformRaycastAtPosition(const FVector& Position)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FVector Start = Position + FVector(0, 0, NodeSeparationX_);
    FVector End = Position - FVector(0, 0, 30.0f);
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this); 


    bool bHit = World->LineTraceSingleByProfile(
        HitResult,
        Start,
        End,
        TEXT("TestFloor"),
        Params
    );

    if (bHit)
    {
        UE_LOG(LogTemp, Log, TEXT("Hit: %s"), *HitResult.GetActor()->GetName());
        if (bShowDebugLinesStarter) {
            DrawDebugLine(World, Start, End, FColor::Red, false, 2.0f);
        }
        return true;
    }
    else
    {
        if (bShowDebugLinesStarter) {
            DrawDebugLine(World, Start, End, FColor::Green, false, 2.0f);
        }
        return false;
    }


    return false;
}


//TODO, change the colision channels so it collides with the correct enviroment.
// Does a horizontal raycas to check if two nodes can be connected
bool AMS_MovementNodeMeshStarter::PerformRaycastToPosition(const FVector& Start, const FVector& End)
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
    Params.AddIgnoredActor(this);


    bool bHit = World->LineTraceSingleByProfile(
        HitResult,
        StartPos,
        EndPos,
        TEXT("CheckFreeWay"),
        Params
    );

    if (bHit)
    {
        UE_LOG(LogTemp, Log, TEXT("Hit: %s"), *HitResult.GetActor()->GetName());
        if (bShowDebugLinesStarter) {
            DrawDebugLine(World, StartPos, EndPos, FColor::Red, false, 2.0f);
        }
        return false; 
    }
    else
    {
        if (bShowDebugLinesStarter) {
            DrawDebugLine(World, StartPos, EndPos, FColor::Green, false, 2.0f);
        }
        return true;
    }


    return false;
}

// Spawns a debug actor where a node is created
void AMS_MovementNodeMeshStarter::SpawnAgentAtPosition(const FVector& Position)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AMS_MovementNode* NewAgent = World->SpawnActor<AMS_MovementNode>(AMS_MovementNode::StaticClass(), Position, FRotator::ZeroRotator, SpawnParams);
    if (NewAgent)
    {
        // Disable tick
        NewAgent->SetActorTickEnabled(false);

        // Disable collision
        NewAgent->SetActorEnableCollision(false);

        // Hide from rendering (optional)
       // NewAgent->SetActorHiddenInGame(true);

        // Disable movement (if using movement components)
        if (UActorComponent* MovementComp = NewAgent->FindComponentByClass<UActorComponent>())
        {
            MovementComp->Deactivate();
        }

        // Disable all components
        TArray<UActorComponent*> Components;
        NewAgent->GetComponents(Components);
        for (UActorComponent* Comp : Components)
        {
            if (Comp)
            {
                Comp->Deactivate(); // Disables logic updates
                Comp->SetComponentTickEnabled(false); // Stops ticking
            }
        }
    }
}


void AMS_MovementNodeMeshStarter::GenerateNodes(FVector FirstPos)
{

    FIntPoint StartGridPos = FIntPoint(0, 0); 

    // Create the starting node
    TSharedPtr<FMoveNode> StartNode = MakeShared<FMoveNode>();
    StartNode->Position = FirstPos;
    StartNode->GridPosition = StartGridPos;

    // Add the starting node to NodeMap
    NodeMap.Add(StartGridPos, StartNode);

    // Keep track of nodes to process
    TQueue<TSharedPtr<FMoveNode>> NodeQueue;
    NodeQueue.Enqueue(StartNode);

    // Set to keep track of visited grid positions
    TSet<FIntPoint> Visited;
    Visited.Add(StartGridPos);

    const TArray<FIntPoint> Directions = {
          FIntPoint(1, 0),  // Right
          FIntPoint(-1, 0), // Left
          FIntPoint(0, 1),  // Up
          FIntPoint(0, -1)  // Down
    };

    while (!NodeQueue.IsEmpty())
    {
        TSharedPtr<FMoveNode> CurrentNode;
        NodeQueue.Dequeue(CurrentNode);

        // For each direction
        for (const FIntPoint& Dir : Directions)
        {
            FIntPoint NeighborGridPos = CurrentNode->GridPosition + Dir;

            // Calculate the neighbor's world position
            FVector NeighborPosition = CurrentNode->Position + FVector(Dir.X * NodeSeparationX_, Dir.Y * NodeSeparationY_, 0.0f);

            // Perform the raycast to check if the node is available
            bool bIsAvailable = PerformRaycastAtPosition(NeighborPosition);
            bool bIsTraversable = PerformRaycastToPosition(CurrentNode->Position, NeighborPosition);

            if (bIsAvailable)
            {
                TSharedPtr<FMoveNode> NeighborNode = nullptr;

                // Check if the neighbor node already exists
                if (TSharedPtr<FMoveNode>* ExistingNode = NodeMap.Find(NeighborGridPos))
                {
                    NeighborNode = *ExistingNode;
                }
                else
                {
                    // Create a new node
                    NeighborNode = MakeShared<FMoveNode>();
                    NeighborNode->Position = NeighborPosition;
                    NeighborNode->GridPosition = NeighborGridPos;

                    // Add to NodeMap
                    NodeMap.Add(NeighborGridPos, NeighborNode);

                    // Spawn an empty agent at the available node
                    //if (bShowDebugLinesStarter) {
                        //SpawnAgentAtPosition(NeighborPosition);
                    //}
                }

                // Check if the neighbor node is traversable from the current node
              
                if (bIsTraversable)
                {
                    // Add as a neighbor to the current node
                    CurrentNode->Neighbors.Add(NeighborNode);
                }

                // If the neighbor hasn't been visited yet, enqueue it for processing
                if (!Visited.Contains(NeighborGridPos))
                {
                    NodeQueue.Enqueue(NeighborNode);
                    Visited.Add(NeighborGridPos);
                }
            }
        }
    }
}


void AMS_MovementNodeMeshStarter::UpdateBlockedPaths()
{
    for (auto& Pair : NodeMap)
    {
        TSharedPtr<FMoveNode> Node = Pair.Value;

        for (auto& NeighborPair : Node->Neighbors)
        {
            bool bIsPathClear = PerformRaycastToPosition(Node->Position, NeighborPair.Key->Position);
            NeighborPair.Value = bIsPathClear; // Update path status

            // Update the reverse connection as well
            if (NeighborPair.Key->Neighbors.Contains(Node))
            {
                NeighborPair.Key->Neighbors[Node] = bIsPathClear;
            }

            if (bShowDebugLinesStarter) {
                // Debug visualization
                DrawDebugLine(GetWorld(), Node->Position, NeighborPair.Key->Position,
                    bIsPathClear ? FColor::Blue : FColor::Red, false, 10.0f, 0, 3.0f);
            }
        }
    }
}
AMS_MovementNodeMeshStarter::~AMS_MovementNodeMeshStarter()
{
    // Clear the NodeMap, releasing all shared pointers
    NodeMap.Empty();

    // Ensure the timer is cleared to prevent any invalid references
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PathCheckTimer);
    }

    UE_LOG(LogTemp, Log, TEXT("AMS_MovementNodeMeshStarter destructor called. NodeMap cleared."));
}