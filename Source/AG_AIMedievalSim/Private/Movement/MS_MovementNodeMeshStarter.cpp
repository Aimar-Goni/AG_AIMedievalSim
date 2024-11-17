// Fill out your copyright notice in the Description page of Project Settings.

#include "DrawDebugHelpers.h" 
#include "Kismet/KismetSystemLibrary.h" 
#include "Movement/MS_PathfindingSubsyste.h"
#include "Movement/MS_MovementNodeMeshStarter.h"

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

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,        
        Start,           
        End,              
        ECC_GameTraceChannel3,    
        Params            
    );

    // Debug draw
    DrawDebugLine(World, Start, End, bHit ? FColor::Green : FColor::Red, false, 2.0f);


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

    UMS_PathfindingSubsyste* PathfindingSubsystem = GetGameInstance()->GetSubsystem<UMS_PathfindingSubsyste>();
    if (PathfindingSubsystem)
    {
        PathfindingSubsystem->SetNodeMap(NodeMap);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PathfindingSubsystem not found."));
    }

}

// Called every frame
void AMS_MovementNodeMeshStarter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



bool AMS_MovementNodeMeshStarter::PerformRaycastAtPosition(const FVector& Position)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FVector Start = Position + FVector(0, 0, 500.0f);
    FVector End = Position - FVector(0, 0, 30.0f);

    TArray<FHitResult> HitResults;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this); 


    bool bHit = World->LineTraceMultiByChannel(
        HitResults,
        Start,
        End,
        ECC_GameTraceChannel3, 
        Params
    );
    DrawDebugLine(World, Start, End, bHit ? FColor::Green : FColor::Red, false, 2.0f);

    if (bHit && HitResults.Num() > 0)
    {
        // Iterate through the hits to find the first valid blocking hit
        for (const FHitResult& Hit : HitResults)
        {

                // Check if the hit component has the "Floor" tag
                if (Hit.Component.IsValid() && Hit.Component->ComponentHasTag(FName("Floor")))
                {
                    return true; // First collision is the floor
                }
                else
                {
                    return false;
                }
            
        }
    }

    return false; 
}


//TODO, change the colision channels so it collides with the correct enviroment.
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
        DrawDebugLine(World, StartPos, EndPos,  FColor::Red , false, 2.0f);
        return false; 
    }
    else
    {
        DrawDebugLine(World, StartPos, EndPos,  FColor::Green, false, 2.0f);
        return true;
    }


    return false;
}


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

    
}


void AMS_MovementNodeMeshStarter::GenerateNodes(FVector FirstPos)
{

    FIntPoint StartGridPos = FIntPoint(0, 0); 

    // Create the starting node
    FNode* StartNode = new FNode();
    StartNode->Position = FirstPos;
    StartNode->GridPosition = StartGridPos;

    // Add the starting node to NodeMap
    NodeMap.Add(StartGridPos, StartNode);

    // Keep track of nodes to process
    TQueue<FNode*> NodeQueue;
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
        FNode* CurrentNode;
        NodeQueue.Dequeue(CurrentNode);

        // For each direction
        for (const FIntPoint& Dir : Directions)
        {
            FIntPoint NeighborGridPos = CurrentNode->GridPosition + Dir;

            // Calculate the neighbor's world position
            FVector NeighborPosition = CurrentNode->Position + FVector(Dir.X * 500.0f, Dir.Y * 500.0f, 0.0f);

            // Perform the raycast to check if the node is available
            bool bIsAvailable = PerformRaycastAtPosition(NeighborPosition);
            bool bIsTraversable = PerformRaycastToPosition(CurrentNode->Position, NeighborPosition);

            if (bIsAvailable)
            {
                FNode* NeighborNode = nullptr;

                // Check if the neighbor node already exists
                if (FNode** ExistingNode = NodeMap.Find(NeighborGridPos))
                {
                    NeighborNode = *ExistingNode;
                }
                else
                {
                    // Create a new node
                    NeighborNode = new FNode();
                    NeighborNode->Position = NeighborPosition;
                    NeighborNode->GridPosition = NeighborGridPos;

                    // Add to NodeMap
                    NodeMap.Add(NeighborGridPos, NeighborNode);

                    // Spawn an empty agent at the available node
                    SpawnAgentAtPosition(NeighborPosition);
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

void AMS_MovementNodeMeshStarter::AddNeighbors(FNode* CurrentNode)
{
    static const TArray<FIntPoint> Directions = {
        FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1)
    };

    for (const FIntPoint& Dir : Directions)
    {
        FIntPoint NeighborPos = CurrentNode->GridPosition + Dir;
        if (FNode** NeighborNodePtr = NodeMap.Find(NeighborPos))
        {
            CurrentNode->Neighbors.Add(*NeighborNodePtr);
        }
    }
}
