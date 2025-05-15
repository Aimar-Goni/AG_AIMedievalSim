#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Movement/MS_MovementNode.h" 
#include "MS_Tavern.generated.h"

UCLASS()
class ALESANDFABLES_API AMS_Tavern : public AActor
{
	GENERATED_BODY()

public:
	AMS_Tavern();

protected:
	virtual void BeginPlay() override;

public:

	/** The cost of one drink at this pub. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pub")
	int32 DrinkCost = 15;

	/** The amount of happiness gained from one drink. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pub")
	float HappinessGain = 50.0f;

	/** How long the AI "stays" at the pub to drink (for the BT Task). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pub")
	float DrinkDuration = 5.0f;

	/** Grid position of this pub (optional, if your system uses it). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pub|Location")
	FIntPoint GridPosition_;

	/** Visual representation of the Pub. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// Optional: Interaction point for AI to move to within the pub
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pub")
	TObjectPtr<USceneComponent> InteractionPoint;
};