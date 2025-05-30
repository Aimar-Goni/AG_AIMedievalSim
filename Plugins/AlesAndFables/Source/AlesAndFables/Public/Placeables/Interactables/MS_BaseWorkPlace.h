// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/MS_InventoryComponent.h"
#include "MS_BaseWorkPlace.generated.h"



UCLASS()
class ALESANDFABLES_API AMS_BaseWorkPlace : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_BaseWorkPlace();

	virtual bool IsPlaceOccupied();
	virtual void ReservePlace();
	virtual FResource TakeResources();
	virtual void ResetWorkPlace();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design | Resources")
	ResourceType ResourceType_ = ResourceType::ERROR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design | Resources")
	int32 ResourceAmount_ = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design | Resources")
	bool InfiniteResource_ = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design | Resources")
	float RespawnTime_ = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal | Resources")
	bool ResourceAvaliable_ = true;

	FTimerHandle TH_ResourceReset_;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Movement")
	FIntPoint GridPosition_;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	bool bWorkPlaceOcupied_;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	bool placeActive_ = false;

};
