// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "MS_WorkpPlacePool.generated.h"

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_WorkpPlacePool : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_WorkpPlacePool();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Workplaces")
	int n_workplaces_;
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Workplaces")
	TArray< TWeakObjectPtr<AMS_BaseWorkPlace>> Workplaces_;


	void FindWorkplacesOnScene();

	UFUNCTION(BlueprintCallable)
	void DeactivateWorkplace(AMS_BaseWorkPlace* Workplace);

	UFUNCTION(BlueprintCallable)
	void ReactivateWorkplace(AMS_BaseWorkPlace* Workplace, const FVector& NewLocation);

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TArray<TSubclassOf<AMS_BaseWorkPlace>> WorkplaceClasses;

};
