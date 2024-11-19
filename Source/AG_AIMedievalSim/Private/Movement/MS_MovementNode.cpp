// Fill out your copyright notice in the Description page of Project Settings.


#include "Movement/MS_MovementNode.h"

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
    MeshComponent->SetWorldScale3D(FVector(0.5f)); // Adjust size as needed
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

