// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/Buildings/MS_BulletingBoard.h"

// Sets default values
AMS_BulletingBoard::AMS_BulletingBoard()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMS_BulletingBoard::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMS_BulletingBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

