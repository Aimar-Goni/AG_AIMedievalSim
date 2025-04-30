// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/MS_ResourceSystem.h"        
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Placeables/Buildings/MS_BulletingBoardPool.h"
#include "Systems/MS_InventoryComponent.h"
#include "TimerManager.h"   
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Dependencies")
	TWeakObjectPtr<AMS_StorageBuildingPool> StorageBuldingsPool_;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Dependencies")
	TWeakObjectPtr<AMS_BulletingBoardPool> BulletingBoardPool_;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Inventory")
	TObjectPtr<UInventoryComponent> Inventory_;

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

protected:

	// Generates resource gathering quests for a specific resource type if needed. 
	void GenerateQuestsForResourceType(ResourceType ResourceTypeToCheck);

	// Checks if an identical quest already exists (available, bidding, or assigned). 
	bool DoesIdenticalQuestExist(ResourceType Type, int32 Amount) const;
	

public:

	// Resource types the AIManager actively manages and generates quests for. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Manager|Quests")
	TArray<ResourceType> ManagedResourceTypes = {
		ResourceType::WOOD, ResourceType::BERRIES, ResourceType::WATER, ResourceType::WHEAT
	};

};
