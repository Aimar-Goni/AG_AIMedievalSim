// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "MS_TreeWorkPlace.generated.h"

/**
 * 
 */
UCLASS()
class AG_AIMEDIEVALSIM_API AMS_TreeWorkPlace : public AMS_BaseWorkPlace
{
	GENERATED_BODY()
	

public:

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
