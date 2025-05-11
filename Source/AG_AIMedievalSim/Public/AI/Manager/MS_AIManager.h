// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/MS_ResourceSystem.h"        
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Placeables/Buildings/MS_BulletingBoardPool.h"
#include "Placeables/Buildings/MS_ConstructionSite.h"
#include "Systems/MS_InventoryComponent.h"
#include "TimerManager.h"
#include "Movement/MS_PathfindingSubsystem.h" 
#include "MS_AIManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestAvailable, const FQuest&, NewQuest);

USTRUCT()
struct FBidInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AMS_AICharacter> Bidder;

	UPROPERTY()
	float BidValue = 0.0f;
	
	UPROPERTY()
	float BidTimestamp = 0.0f;
};


class AMS_StorageBuilding;
UCLASS()
class AG_AIMEDIEVALSIM_API AMS_AIManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AMS_AIManager();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void SelectQuestWinner_Internal(FGuid QuestID);

	void StartBidTimer(const FQuest& Quest);

public:	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AA |Game Speed")
	float gameSpeed = 3.0f;  

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Dependencies")
	TWeakObjectPtr<AMS_StorageBuildingPool> StorageBuldingsPool_;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Dependencies")
	TWeakObjectPtr<AMS_BulletingBoardPool> BulletingBoardPool_;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Manager|Dependencies")
	TWeakObjectPtr<AMS_StorageBuilding> CentralStorageBuilding;

	UFUNCTION(BlueprintPure, Category="AI Manager|Inventory")
	UInventoryComponent* GetCentralStorageInventory() const;
	//  Quest Management 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Manager|Quests")
	TArray<FQuest> AvailableQuests_;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Manager|Quests")
	TMap<FGuid, TWeakObjectPtr<AMS_AICharacter>> AssignedQuests_;
	
	//UPROPERTY(VisibleAnywhere, Category = "AI Manager|Bidding")
	TMap<FGuid, TArray<FBidInfo>> CurrentBids;
	
    UPROPERTY()
    TMap<FGuid, FTimerHandle> BidTimers;
	
	UPROPERTY(BlueprintAssignable, Category = "AI Manager|Events")
	FOnQuestAvailable OnQuestAvailable;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Bidding")
    float BidDuration = 3.0f;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Quests")
    int32 LowResourceThreshold = 50;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Quests")
    int32 MaxResourcePerQuest = 15;
	
	UFUNCTION(BlueprintCallable, Category = "AI Manager|Bidding")
	void ReceiveBid(AMS_AICharacter* Bidder, FQuest Quest, float BidValue);
	
    UFUNCTION(BlueprintCallable, Category = "AI Manager|Quests")
    void RequestQuestCompletion(AMS_AICharacter* Character, FGuid QuestID);

	void InitializeFieldListeners();

protected:

	// Generates resource gathering quests for a specific resource type if needed. 
	void GenerateQuestsForResourceType(ResourceType ResourceTypeToCheck);

	// Checks if an identical quest already exists (available, bidding, or assigned). 
	bool DoesIdenticalQuestExist(ResourceType Type, int32 Amount, AActor* Target ) const;

	// Checks conditions and potentially initiates a construction project.
	void CheckAndInitiateConstruction();

	// Spawns a construction site and generates the initial delivery quest.
	void StartBuildingProject(TSubclassOf<AActor> BuildingClassToSpawn, const FVector& Location, ResourceType RequiredResource, int32 ResourceCost, const TArray<FIntPoint>& OccupiedNodes);

	bool FindSuitableBuildLocation(int32 SizeX, int32 SizeY, FVector& OutCenterLocation, TArray<FIntPoint>& OutOccupiedNodes);
	
	void UpdateHousingState();
	
	void InitializeCentralStorage();
	
	bool ShouldBuildWheatField() const;
	
	UFUNCTION()
	void OnWheatFieldNeedsPlanting(AMS_WheatField* Field);
	UFUNCTION()
	void OnWheatFieldNeedsWatering(AMS_WheatField* Field);
	UFUNCTION()
	void OnWheatFieldReadyToHarvest(AMS_WheatField* Field); // Renamed from OnWheatFieldReady

public:

	// Resource types the AIManager actively manages and generates quests for. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Quests")
	TArray<ResourceType> ManagedResourceTypes = {
		ResourceType::WOOD, ResourceType::BERRIES, ResourceType::WATER, ResourceType::WHEAT
	};

	// Minimum amount of wood needed before considering starting construction. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Construction")
	int32 ConstructionWoodThreshold = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Construction")
	int32 HouseWoodCost = 50;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Construction")
	TSubclassOf<AActor> HouseBuildingClass;

	// Class reference for the Construction Site Actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Construction")
	TSubclassOf<class AMS_ConstructionSite> ConstructionSiteClass; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Construction")
	int32 MaxConcurrentConstruction = 3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI Manager|Population") 
	TSubclassOf<AMS_AICharacter> AICharacterClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Population")
	int32 HouseCapacity = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Construction")
	float HousingCheckInterval = 5.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Quests")
	int32 DeliveryCarryCapacity = 15;

	UFUNCTION(BlueprintCallable, Category="AI Manager|Construction") // Called by ConstructionSite maybe?
	void NotifyConstructionProgress(AMS_ConstructionSite* Site, int32 AmountDelivered);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Construction")
	int32 WheatFieldWoodCost = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Construction")
	int32 MaxWheatField  = 5;
	
	/** Class reference for the Wheat Field building. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Construction")
	TSubclassOf<AMS_WheatField> WheatFieldClass;

private:

	int32 CurrentPopulation = 0;
	int32 TotalHousingCapacity = 0;
	FTimerHandle HousingCheckTimerHandle;
	
	UPROPERTY()
	TObjectPtr<UMS_PathfindingSubsystem> PathfindingSubsystemCache;
	
	TMap<TWeakObjectPtr<AMS_ConstructionSite>, TArray<FGuid>> ActiveConstructionDeliveryQuests;
};
