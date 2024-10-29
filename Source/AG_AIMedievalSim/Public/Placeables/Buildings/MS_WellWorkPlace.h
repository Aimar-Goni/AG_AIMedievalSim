// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "MS_WellWorkPlace.generated.h"

/**
 * 
 */
UCLASS()
class AG_AIMEDIEVALSIM_API AMS_WellWorkPlace : public AMS_BaseWorkPlace
{
	GENERATED_BODY()
public:

	AMS_WellWorkPlace();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
