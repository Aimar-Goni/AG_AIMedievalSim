// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/MS_InventoryComponent.h"
#include "MS_ResourceSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBerriesChanged, int32, newAmmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWoodChanged, int32, newAmmount);


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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	FInventory Inventory_;

	UPROPERTY(BlueprintAssignable)
	FOnBerriesChanged OnBerriesChanged;

	UPROPERTY(BlueprintAssignable)
	FOnWoodChanged OnWoodChanged;


	void SetWood(int32 NewBerries);
	int32  GetWood();

	void SetBerries(int32 NewBerries);
	int32  GetBerries();

};
