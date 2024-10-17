// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MS_ResourceSystem.generated.h"

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_ResourceSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_ResourceSystem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Food_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Wood_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Pelts_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Berries_;


};
