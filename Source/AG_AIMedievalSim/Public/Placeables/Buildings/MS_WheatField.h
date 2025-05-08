#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/MS_InventoryComponent.h" // For ResourceType
#include "Movement/MS_MovementNode.h"    // For FIntPoint
#include "TimerManager.h"              // For FTimerHandle
#include "MS_WheatField.generated.h"

class AMS_AIManager; // Forward declare

// Updated states for the detailed process
UENUM(BlueprintType)
enum class EFieldState : uint8
{
	Constructed		UMETA(DisplayName = "Constructed"), // Just built, empty earth
	Planted			UMETA(DisplayName = "Planted"),     // Seeds sown, needs water
	Watered			UMETA(DisplayName = "Watered"),		// Watered, short wait before sprout
	Sprouting		UMETA(DisplayName = "Sprouting"),   // Small sprouts visible, needs growth time
	Growing			UMETA(DisplayName = "Growing"),     // Larger plants visible, needs growth time
	ReadyToHarvest	UMETA(DisplayName = "ReadyToHarvest") // Fully grown, ready to collect
};

// Delegates to notify AIManager of state changes needing quests
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFieldNeedsPlanting, AMS_WheatField*, Field);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFieldNeedsWatering, AMS_WheatField*, Field);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFieldReadyToHarvest, AMS_WheatField*, Field); // Keep this

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_WheatField : public AActor
{
	GENERATED_BODY()

public:
	AMS_WheatField();

protected:
	virtual void BeginPlay() override;

	UFUNCTION() // Make UFUNCTION for timer binding
	void OnWateredTimerComplete(); 

    UFUNCTION() // Make UFUNCTION for timer binding
    void OnSproutTimerComplete(); 

	UFUNCTION() // Make UFUNCTION for timer binding
	void OnGrowthTimerComplete(); 

    /** Finds and caches the AI Manager */
    void InitializeAIManager();


public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WheatField|State") // Use VisibleAnywhere for state
	EFieldState CurrentState = EFieldState::Constructed;

	// --- Configuration ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WheatField|Config")
	float WateredToSproutDuration = 15.0f; // Time from watered until sprouts appear

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WheatField|Config")
	float SproutToGrowthDuration = 45.0f; // Time from sprouts to larger plants (optional extra stage)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WheatField|Config")
	float GrowthToHarvestDuration = 60.0f; // Time from larger plants to ready

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WheatField|Config")
	int32 HarvestAmount = 10; // Amount of Wheat yielded

	// --- Location & Components ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WheatField|Location")
	FIntPoint GridPosition_;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

    // --- Meshes for different states ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WheatField|Visuals")
	TObjectPtr<UStaticMesh> MeshState_Constructed; // Empty tilled earth

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WheatField|Visuals")
	TObjectPtr<UStaticMesh> MeshState_Planted; // Earth with maybe seed holes (optional)

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WheatField|Visuals")
	TObjectPtr<UStaticMesh> MeshState_Sprouting; // Small green sprouts

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WheatField|Visuals")
	TObjectPtr<UStaticMesh> MeshState_Growing; // Medium height plants

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "WheatField|Visuals")
	TObjectPtr<UStaticMesh> MeshState_ReadyToHarvest; // Tall, golden wheat

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent; // Main visual component


    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "WheatField|Events")
    FOnFieldNeedsPlanting OnFieldNeedsPlanting;

    UPROPERTY(BlueprintAssignable, Category = "WheatField|Events")
    FOnFieldNeedsWatering OnFieldNeedsWatering;

    UPROPERTY(BlueprintAssignable, Category = "WheatField|Events")
    FOnFieldReadyToHarvest OnFieldReadyToHarvest;


	// --- AI Interaction Functions ---
	UFUNCTION(BlueprintCallable, Category = "WheatField")
	bool PlantSeeds(); // Returns true if successful

	UFUNCTION(BlueprintCallable, Category = "WheatField")
	bool WaterField(); // Returns true if successful

	UFUNCTION(BlueprintCallable, Category = "WheatField")
	FResource HarvestField(); // Returns Wheat resource

    UFUNCTION(BlueprintPure, Category = "WheatField")
    EFieldState GetCurrentFieldState() const { return CurrentState; }


private:
	FTimerHandle WateredTimerHandle;
    FTimerHandle SproutTimerHandle;
    FTimerHandle GrowthTimerHandle;

    UPROPERTY() // Keep weak ptr to manager
    TWeakObjectPtr<AMS_AIManager> AIManager;

    // Helper to change state, update mesh, and potentially broadcast
    void ChangeState(EFieldState NewState);
};