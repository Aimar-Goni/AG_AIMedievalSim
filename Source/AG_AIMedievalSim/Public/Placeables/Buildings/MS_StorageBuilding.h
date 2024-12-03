// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/MS_ResourceSystem.h"
#include "Systems/MS_InventoryComponent.h"
#include "MS_StorageBuilding.generated.h"


UCLASS()
class AG_AIMEDIEVALSIM_API AMS_StorageBuilding : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_StorageBuilding();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Inventory")
	UInventoryComponent* Inventory_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Movement")
	FIntPoint GridPosition_;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void LeaveResources();

	void TakeResources();


};
