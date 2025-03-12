// Fill out your copyright notice in the Description page of Project Settings.


#include "Movement/MS_MovementNode.h"
#include "Movement/MS_PathfindingSubsystem.h"

// Sets default values
AMS_MovementNode::AMS_MovementNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    // Create the mesh component and set it as the root component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // Find and assign the cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMesh.Object);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load cube mesh."));
    }

    // Optionally, adjust the scale of the cube
    MeshComponent->SetWorldScale3D(FVector(0.5f)); 

    MeshComponent->ComponentTags.Add(FName("Floor"));

    // Set the collision preset to OverlapAllDynamic
    MeshComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));



    //CollisionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionTrigger"));
    //CollisionTrigger->SetupAttachment(RootComponent);
    //CollisionTrigger->SetBoxExtent(FVector(50.0f)); // Adjust size based on node spacing
    //CollisionTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    //// Bind event functions
    //CollisionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AMS_MovementNode::OnObstacleEnter);
    //CollisionTrigger->OnComponentEndOverlap.AddDynamic(this, &AMS_MovementNode::OnObstacleExit);
}
// Called when the game starts or when spawned
void AMS_MovementNode::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AMS_MovementNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMS_MovementNode::OnObstacleEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->ActorHasTag(TEXT("Obstacle")))  // Ensure only obstacles trigger
    {
        UE_LOG(LogTemp, Warning, TEXT("Obstacle detected at node %s"), *GetName());
        DrawDebugSphere(GetWorld(), GetActorLocation(), 30.0f, 12, FColor::Red, false, 10.0f);

        // Notify pathfinding system to remove this node
        UMS_PathfindingSubsystem* PathfindingSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
        if (PathfindingSubsystem)
        {
            PathfindingSubsystem->BlockNode(GetActorLocation());
        }
    }
}

// Called when an obstacle exits
void AMS_MovementNode::OnObstacleExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->ActorHasTag(TEXT("Obstacle")))
    {
        UE_LOG(LogTemp, Warning, TEXT("Obstacle removed at node %s"), *GetName());
        DrawDebugSphere(GetWorld(), GetActorLocation(), 30.0f, 12, FColor::Green, false, 10.0f);

        // Notify pathfinding system to restore this node
        UMS_PathfindingSubsystem* PathfindingSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
        if (PathfindingSubsystem)
        {
            PathfindingSubsystem->UnblockNode(GetActorLocation());
        }
    }
}