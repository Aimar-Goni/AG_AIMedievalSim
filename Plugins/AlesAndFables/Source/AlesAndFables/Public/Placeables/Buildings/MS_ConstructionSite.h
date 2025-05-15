#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/MS_InventoryComponent.h" // For ResourceType
#include "MS_ConstructionSite.generated.h"

class AMS_AIManager;

UCLASS()
class ALESANDFABLES_API AMS_ConstructionSite : public AActor
{
	GENERATED_BODY()

public:
	AMS_ConstructionSite();

protected:
	virtual void BeginPlay() override;

    // Pointer to the AI Manager instance 
    UPROPERTY()
    TWeakObjectPtr<AMS_AIManager> AIManager;

    // Finds the AI Manager 
    void InitializeAIManager();

public:

    // The final building class to spawn when construction is complete. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	TSubclassOf<AActor> BuildingClassToSpawn;

    // The resource needed for construction. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	ResourceType RequiredResource = ResourceType::WOOD;

    // Total amount of the resource needed. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Construction")
	int32 AmountRequired = 50;

    // Current amount of the resource delivered to the site. 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction", meta=(AllowPrivateAccess=true)) // AllowPrivateAccess useful if using BP functions
	int32 CurrentAmount = 0;

    // The nodes occupied by this construction site 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
    TArray<FIntPoint> OccupiedNodes;

    // Root component (e.g., a SceneComponent) 
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

    // Visual representation (e.g., scaffolding mesh) 
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components") // ReadWrite if you change mesh on progress
    TObjectPtr<UStaticMeshComponent> MeshComponent;


    // Called by AI when delivering resources. 
    UFUNCTION(BlueprintCallable, Category="Construction")
    bool AddResource(int32 AmountToAdd);

    // Called when CurrentAmount reaches AmountRequired. 
    UFUNCTION(BlueprintCallable, Category="Construction") // Call from BP if needed
    virtual void CompleteConstruction();

     // Called when this actor is being destroyed */
     virtual void BeginDestroy() override;

};