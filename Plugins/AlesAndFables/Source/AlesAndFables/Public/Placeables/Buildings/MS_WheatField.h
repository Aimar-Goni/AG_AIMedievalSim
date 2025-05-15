
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/MS_InventoryComponent.h"
#include "Movement/MS_MovementNode.h"
#include "TimerManager.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "MS_WheatField.generated.h"

class AMS_AIManager;

UENUM(BlueprintType)
enum class EFieldState : uint8
{
	Constructed		UMETA(DisplayName = "Constructed"),      // Just built, empty earth
	Planted			UMETA(DisplayName = "Planted"),          // Seeds sown, needs water
	Watered			UMETA(DisplayName = "Watered"),		     // Watered, now growing (triggers growth timer)
	Growing			UMETA(DisplayName = "Growing"),          // Actively growing (visual change, no direct AI interaction needed here)
	ReadyToHarvest	UMETA(DisplayName = "ReadyToHarvest"),   // Fully grown, ready to collect
    Harvested       UMETA(DisplayName = "Harvested")         // Resources collected, field is now empty/reset
};

// Delegates to notify AIManager of state changes needing quests
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFieldNeedsPlanting, AMS_WheatField*, Field);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFieldNeedsWatering, AMS_WheatField*, Field);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFieldReadyToHarvest, AMS_WheatField*, Field);

UCLASS()
class ALESANDFABLES_API AMS_WheatField : public AMS_BaseWorkPlace
{
	GENERATED_BODY()

public:
	AMS_WheatField();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnWateringGrowthTimerComplete(); // Timer after watering, moves to Growing, then ReadyToHarvest
	void OnGrowingTimerComplete();
    void InitializeAIManager();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WheatField|State")
	EFieldState CurrentState = EFieldState::Constructed;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WheatField|Config")
	float WateredToGrowingDuration = 60.0f; // Total time from watered to ready for harvest (includes "Growing" phase)
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WheatField|Config")
	float GrowingToReadyDuration = 60.0f; // Total time from watered to ready for harvest (includes "Growing" phase)

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WheatField|Config")
	int32 HarvestAmount = 10;
	
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WheatField|Visuals")
	TObjectPtr<UStaticMesh> MeshState_Constructed; // Empty tilled earth

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WheatField|Visuals")
	TObjectPtr<UStaticMesh> MeshState_Planted; // Earth with seed pattern/small shoots

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WheatField|Visuals")
	TObjectPtr<UStaticMesh> MeshState_Watered; 
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WheatField|Visuals")
	TObjectPtr<UStaticMesh> MeshState_Growing; // Medium height green plants

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WheatField|Visuals")
	TObjectPtr<UStaticMesh> MeshState_ReadyToHarvest; // Tall, golden wheat

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WheatField|Visuals")
	TObjectPtr<UStaticMesh> MeshState_Harvested; // Tall, golden wheat
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;



    UPROPERTY(BlueprintAssignable, Category = "WheatField|Events")
    FOnFieldNeedsPlanting OnFieldNeedsPlanting;

    UPROPERTY(BlueprintAssignable, Category = "WheatField|Events")
    FOnFieldNeedsWatering OnFieldNeedsWatering;

    UPROPERTY(BlueprintAssignable, Category = "WheatField|Events")
    FOnFieldReadyToHarvest OnFieldReadyToHarvest;



	UFUNCTION(BlueprintCallable, Category = "WheatField")
	bool PerformPlanting(); // AI confirms planting action

	UFUNCTION(BlueprintCallable, Category = "WheatField")
	bool PerformWatering(); // AI confirms watering action

	UFUNCTION(BlueprintCallable, Category = "WheatField")
	FResource PerformHarvesting(); // AI confirms harvesting action

    UFUNCTION(BlueprintPure, Category = "WheatField")
    EFieldState GetCurrentFieldState() const { return CurrentState; }

private:
	FTimerHandle GrowthCycleTimerHandleWatered; // Single timer for post-watering growth
	FTimerHandle GrowthCycleTimerHandleGrowing; // Single timer for post-watering growth

    UPROPERTY()
    TWeakObjectPtr<AMS_AIManager> AIManager;

    void ChangeState(EFieldState NewState);
};