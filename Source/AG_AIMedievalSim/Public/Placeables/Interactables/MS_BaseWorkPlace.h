// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MS_BaseWorkPlace.generated.h"

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_BaseWorkPlace : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_BaseWorkPlace();
	bool IsPlaceOccupied();
	virtual void ExecuteAction();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	bool bWorkPlaceOcupied;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
