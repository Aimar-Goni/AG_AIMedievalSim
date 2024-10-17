// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Placeables/Buildings/MS_StorageBuilding.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "MS_AICharacter.generated.h"

USTRUCT(BlueprintType)
struct FInventory

{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Food_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Wood_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Pelts_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Berries_;
};

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_AICharacter : public ACharacter
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal|AI")
	class UBehaviorTree* behaviorTree_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Behavior")
	AActor* target_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Workplaces")
	AActor* WorkPlacesPool_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	AActor* StorageBuldingsPool_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	FInventory Inventory;

	// Sets default values for this character's properties
	AMS_AICharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
