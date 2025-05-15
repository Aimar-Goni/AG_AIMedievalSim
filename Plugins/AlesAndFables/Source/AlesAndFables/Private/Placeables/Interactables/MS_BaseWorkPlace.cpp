// Fill out your copyright notice in the Description page of Project Settings.

#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "Engine.h"

// Sets default values
AMS_BaseWorkPlace::AMS_BaseWorkPlace()
{
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMS_BaseWorkPlace::BeginPlay()
{
	Super::BeginPlay();
	bWorkPlaceOcupied_ = false;
	ResourceAvaliable_ = true;

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
	if(!InfiniteResource_)
	{

		bWorkPlaceOcupied_ = false;
		ResourceAvaliable_ = false;

		GetWorld()->GetTimerManager().SetTimer(TH_ResourceReset_, this, &AMS_BaseWorkPlace::ResetWorkPlace, RespawnTime_, false);

	}
	return { ResourceType_ , ResourceAmount_ };
}

void AMS_BaseWorkPlace::ResetWorkPlace() 
{
	ResourceAvaliable_ = true;
}
