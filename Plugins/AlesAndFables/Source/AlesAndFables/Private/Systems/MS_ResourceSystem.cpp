// Fill out your copyright notice in the Description page of Project Settings.



#include "Systems/MS_ResourceSystem.h"

// Sets default values
AMS_ResourceSystem::AMS_ResourceSystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMS_ResourceSystem::BeginPlay()
{
	Super::BeginPlay();
    
    
}

// Called every frame
void AMS_ResourceSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

