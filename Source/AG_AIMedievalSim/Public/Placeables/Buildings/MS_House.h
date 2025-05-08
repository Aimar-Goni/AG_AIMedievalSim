// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Movement/MS_MovementNode.h"
#include "MS_House.generated.h"

class AMS_AICharacter;

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_House : public AActor
{
	GENERATED_BODY()

public:
	AMS_House();

protected:
	virtual void BeginPlay() override;

public:

	/** Maximum number of AI characters this house can accommodate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "House", meta = (ClampMin = "1"))
	int32 MaxOccupants = 2;

	/** Current number of AI characters occupying this house for sleep. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "House")
	int32 CurrentOccupantCount = 0;

	/** Grid position of this house (e.g., for pathfinding or information). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "House|Location")
	FIntPoint GridPosition_;
	
	/** Visual representation of the house. */
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;


	UFUNCTION(BlueprintCallable, Category = "House")
	bool TryEnterHouse(AMS_AICharacter* Character);


	UFUNCTION(BlueprintCallable, Category = "House")
	void LeaveHouse(AMS_AICharacter* Character);
	
	UFUNCTION(BlueprintPure, Category = "House")
	bool HasSpace() const;

private:

	UPROPERTY()
	TArray<TWeakObjectPtr<AMS_AICharacter>> OccupyingCharacters;
};