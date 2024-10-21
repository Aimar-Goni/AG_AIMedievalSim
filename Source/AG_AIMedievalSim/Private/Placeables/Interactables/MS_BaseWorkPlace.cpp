// Fill out your copyright notice in the Description page of Project Settings.

#include "Engine.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"

// Sets default values
AMS_BaseWorkPlace::AMS_BaseWorkPlace()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMS_BaseWorkPlace::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMS_BaseWorkPlace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AMS_BaseWorkPlace::IsPlaceOccupied()
{
	return bWorkPlaceOcupied_;
}

void AMS_BaseWorkPlace::ReservePlace()
{
	bWorkPlaceOcupied_ = true;
}

FResource AMS_BaseWorkPlace::TakeResources()
{
	bWorkPlaceOcupied_ = false;
	ResourceAvaliable_ = false;

	GetWorld()->GetTimerManager().SetTimer(TH_ResourceReset_, this, &AMS_BaseWorkPlace::ResetWorkPlace, RespawnTime_, false);

	return { ResourceType_ , ResourceAmmount_ };
}

void AMS_BaseWorkPlace::ResetWorkPlace() 
{
	ResourceAvaliable_ = true;
}
