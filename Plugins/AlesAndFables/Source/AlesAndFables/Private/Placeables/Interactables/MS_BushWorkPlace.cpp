// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/Interactables/MS_BushWorkPlace.h"


// Called when the game starts or when spawned
void AMS_BushWorkPlace::BeginPlay()
{
	Super::BeginPlay();
	this->SetActorTickEnabled(false);
}

// Called every frame
void AMS_BushWorkPlace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


