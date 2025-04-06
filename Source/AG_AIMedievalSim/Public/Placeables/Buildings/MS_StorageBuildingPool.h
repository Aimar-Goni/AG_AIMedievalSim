// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Placeables/Buildings/MS_StorageBuilding.h"
#include "MS_StorageBuildingPool.generated.h"

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_StorageBuildingPool : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_StorageBuildingPool();

	UFUNCTION()
	void OnNodeMapInitialized();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	int n_StorageBuldings_ = 0;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	TArray< TWeakObjectPtr<AMS_StorageBuilding>> StorageBuldings_;

	void FindStorageBuildingsOnScene();

	UFUNCTION(BlueprintCallable)
	void DeactivateStorageBuilding(AMS_StorageBuilding* Building);

	UFUNCTION(BlueprintCallable)
	void ReactivateStorageBuilding(AMS_StorageBuilding* Building, const FVector& NewLocation);

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<AMS_StorageBuilding> StorageBuildingClass;

};
