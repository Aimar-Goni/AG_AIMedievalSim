// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Manager/MS_AIManager.h"
#include "Systems/MS_InventoryComponent.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Movement/MS_PathfindingSubsystem.h"


AMS_AIManager::AMS_AIManager()
{
	PrimaryActorTick.bCanEverTick = true; 
	Inventory_ = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void AMS_AIManager::BeginPlay()
{
    Super::BeginPlay();

	UWorld* world = GetWorld();


	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_StorageBuildingPool::StaticClass());
	StorageBuldingsPool_ = FoundActor ? Cast<AMS_StorageBuildingPool>(FoundActor) : world->SpawnActor<AMS_StorageBuildingPool>();

	
	FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_BulletingBoardPool::StaticClass());
	BulletingBoardPool_ = FoundActor ? Cast<AMS_BulletingBoardPool>(FoundActor) : world->SpawnActor<AMS_BulletingBoardPool>();
	
    UE_LOG(LogTemp, Warning, TEXT("AIManager BeginPlay: Inventory represents CENTRAL storage. AI deposit logic required elsewhere."));

	PathfindingSubsystemCache = GetWorld()->GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
	if (!PathfindingSubsystemCache)
	{
		UE_LOG(LogTemp, Error, TEXT("AIManager: Failed to cache Pathfinding Subsystem!"));
	}
    // if (BulletingBoardPool_.IsValid())
    // {
    //     // BulletingBoardPool_->OnBulletingBoardPoolInitialized.AddDynamic(this, &AMS_AIManager::OnBulletingBoardPoolReady);
    // }

	GetWorldTimerManager().SetTimer(HousingCheckTimerHandle, this, &AMS_AIManager::UpdateHousingState, HousingCheckInterval, true, 5.0f);
}

void AMS_AIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Inventory_)
	{
		for (ResourceType type : ManagedResourceTypes)
		{
			GenerateQuestsForResourceType(type);
		}
	}
	
	//CheckAndInitiateConstruction();
}
static int32 CalculateGoldReward(ResourceType Resource, int32 Amount)
{
	int32 GoldPerUnit = 0;

	switch (Resource)
	{
	case ResourceType::BERRIES:
		GoldPerUnit = 2;
		break;
	case ResourceType::WOOD:
		GoldPerUnit = 3;
		break;
	case ResourceType::WATER:
		GoldPerUnit = 1;
		break;
	case ResourceType::ERROR:
	default:
		GoldPerUnit = 0;
		break;
	}

	return GoldPerUnit * Amount;
}
void AMS_AIManager::GenerateQuestsForResourceType(ResourceType ResourceTypeToCheck)
{
	if (!Inventory_ || ResourceTypeToCheck == ResourceType::ERROR) return;

	int32 currentAmount = Inventory_->GetResourceAmount(ResourceTypeToCheck);

	if (currentAmount < LowResourceThreshold)
	{
		int32 neededResources = LowResourceThreshold - currentAmount;
		// UE_LOG(LogTemp, Log, TEXT("AIManager: Low on %s (%d/%d). Need %d."), *UEnum::GetValueAsString(ResourceTypeToCheck), currentAmount, LowResourceThreshold, neededResources);

		while (neededResources > 0)
		{
			int32 questAmount = FMath::Min(neededResources, MaxResourcePerQuest);
			if (questAmount <= 0) break; // Safety break

			// --- Check for Duplicates ---
			if (!DoesIdenticalQuestExist(ResourceTypeToCheck, questAmount))
			{
				// Calculate reward
				int32 reward = CalculateGoldReward(ResourceTypeToCheck, questAmount);

				// Create the quest
				FQuest newQuest(ResourceTypeToCheck, questAmount, reward);

				// Add to available list
				AvailableQuests_.Add(newQuest);

				UE_LOG(LogTemp, Log, TEXT("AIManager: Generated Quest ID %s - Gather %d %s for %d reward."), *newQuest.QuestID.ToString(), questAmount, *UEnum::GetValueAsString(ResourceTypeToCheck), reward);

				// Start the bidding timer immediately
				StartBidTimer(newQuest);

				// Broadcast AFTER adding to list and starting timer
				OnQuestAvailable.Broadcast(newQuest);
			}
			else
			{
				// UE_LOG(LogTemp, Verbose, TEXT("AIManager: Identical quest for %d %s already exists. Skipping generation."), questAmount, *UEnum::GetValueAsString(ResourceTypeToCheck));
			}

			neededResources -= questAmount;
		}
	}
}

bool AMS_AIManager::DoesIdenticalQuestExist(ResourceType Type, int32 Amount) const
{
	// 1. Check Available Quests (not yet bid on / assigned)
	for (const FQuest& availableQuest : AvailableQuests_)
	{
		// Only check quests not currently being bid on (timer map check)
		// And ensure it's not a delivery quest (TargetDestination == null)
		if (!BidTimers.Contains(availableQuest.QuestID) && !AssignedQuests_.Contains(availableQuest.QuestID) &&
			availableQuest.Type == Type && availableQuest.Amount == Amount && !availableQuest.TargetDestination.IsValid())
		{
			return true;
		}
	}

	// 2. Check Quests Currently Being Bid On
	for (const FQuest& availableQuest : AvailableQuests_)
	{
		if (BidTimers.Contains(availableQuest.QuestID) &&
		   availableQuest.Type == Type && availableQuest.Amount == Amount && !availableQuest.TargetDestination.IsValid())
		{
			return true;
		}
	}


	// 3. Check Assigned Quests
	for (const auto& pair : AssignedQuests_)
	{
		AMS_AICharacter* character = pair.Value.Get();
		if (character && character->AssignedQuest.Type == Type && character->AssignedQuest.Amount == Amount && !character->AssignedQuest.TargetDestination.IsValid())
		{
			return true;
		}
	}
	

	return false;
}

void AMS_AIManager::StartBidTimer(const FQuest& Quest)
{
    // Check if a timer for this quest ID already exists (shouldn't happen with current flow, but good practice)
    if (!BidTimers.Contains(Quest.QuestID))
    {
        FTimerHandle bidTimerHandle;
        // Use CreateUObject to bind a function that takes parameters
        FTimerDelegate timerDelegate;
        // We bind SelectQuestWinner_Internal which takes the GUID
        timerDelegate.BindUFunction(this, FName("SelectQuestWinner_Internal"), Quest.QuestID);

        GetWorldTimerManager().SetTimer(bidTimerHandle, timerDelegate, BidDuration, false);
        BidTimers.Add(Quest.QuestID, bidTimerHandle);
        UE_LOG(LogTemp, Log, TEXT("AIManager: Started bid timer for Quest ID %s. Duration: %.1f s"), *Quest.QuestID.ToString(), BidDuration);
    }
     else
    {
         UE_LOG(LogTemp, Warning, TEXT("AIManager: Tried to start bid timer for Quest ID %s, but one already exists."), *Quest.QuestID.ToString());
    }
}


void AMS_AIManager::ReceiveBid(AMS_AICharacter* Bidder, FQuest Quest, float BidValue)
{
	if (!Bidder) return;

	// Check if the quest is actually available for bidding
    bool bQuestIsAvailable = false;
    for(const FQuest& AvailableQuest : AvailableQuests_)
    {
        if(AvailableQuest.QuestID == Quest.QuestID)
        {
            bQuestIsAvailable = true;
            break;
        }
    }

	// Also ensure bidding hasn't already closed (timer finished)
    if (bQuestIsAvailable && BidTimers.Contains(Quest.QuestID))
	{
		TArray<FBidInfo>& bids = CurrentBids.FindOrAdd(Quest.QuestID);
		FBidInfo newBid;
		newBid.Bidder = Bidder;
		newBid.BidValue = BidValue;
        newBid.BidTimestamp = GetWorld()->GetTimeSeconds(); // Record time for tie-breaking
		bids.Add(newBid);

		UE_LOG(LogTemp, Log, TEXT("AIManager: Received bid from %s for Quest ID %s. Value: %.2f"), *Bidder->GetName(), *Quest.QuestID.ToString(), BidValue);
	}
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("AIManager: Received bid from %s for invalid/closed Quest ID %s."), *Bidder->GetName(), *Quest.QuestID.ToString());
    }
}

void AMS_AIManager::SelectQuestWinner_Internal(FGuid QuestID)
{
    UE_LOG(LogTemp, Log, TEXT("AIManager: Selecting winner for Quest ID %s."), *QuestID.ToString());

    // Find the original quest details (needed for assignment)
    FQuest originalQuest;
    bool bFoundQuest = false;
    for(const FQuest& q : AvailableQuests_) {
        if(q.QuestID == QuestID) {
            originalQuest = q;
            bFoundQuest = true;
            break;
        }
    }

    if(!bFoundQuest) {
        UE_LOG(LogTemp, Warning, TEXT("AIManager: Quest ID %s not found in AvailableQuests during winner selection."), *QuestID.ToString());
        CurrentBids.Remove(QuestID);
        BidTimers.Remove(QuestID);
        return;
    }


	if (CurrentBids.Contains(QuestID))
	{
		TArray<FBidInfo>& bids = CurrentBids[QuestID];
		AMS_AICharacter* winner = nullptr;
		float highestBid = -1.0f; // Assuming bids are non-negative
        float winningBidTimestamp = FLT_MAX;

		for (const FBidInfo& bid : bids)
		{
            // Ensure the bidder is still valid before considering the bid
			if (bid.Bidder.IsValid())
			{
				if (bid.BidValue > highestBid)
				{
					highestBid = bid.BidValue;
					winner = bid.Bidder.Get();
                    winningBidTimestamp = bid.BidTimestamp;
				}
                // Medieval Tie-breaking: First come, first served (lowest timestamp wins)
                else if (FMath::IsNearlyEqual(bid.BidValue, highestBid) && bid.BidTimestamp < winningBidTimestamp)
                {
                    winner = bid.Bidder.Get();
                    winningBidTimestamp = bid.BidTimestamp;
                }
			}
            else
            {
                 UE_LOG(LogTemp, Warning, TEXT("AIManager: Bidder became invalid during winner selection for Quest ID %s."), *QuestID.ToString());
            }
		}

		if (winner)
		{
			UE_LOG(LogTemp, Log, TEXT("AIManager: Assigning Quest ID %s to %s (Bid: %.2f)."), *QuestID.ToString(), *winner->GetName(), highestBid);

            // Move quest from Available to Assigned
            AvailableQuests_.RemoveAll([QuestID](const FQuest& q){ return q.QuestID == QuestID; }); // Remove by ID
			AssignedQuests_.Add(QuestID, winner);

			// Notify the winner
			winner->AssignQuest(originalQuest);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AIManager: No valid winner found for Quest ID %s. Quest remains available?"), *QuestID.ToString());
             AvailableQuests_.RemoveAll([QuestID](const FQuest& q){ return q.QuestID == QuestID; });
		}

		// Clean up bids and timer for this quest
		CurrentBids.Remove(QuestID);
		BidTimers.Remove(QuestID);
	}
     else
    {
         UE_LOG(LogTemp, Warning, TEXT("AIManager: No bids found for Quest ID %s during winner selection."), *QuestID.ToString());
         AvailableQuests_.RemoveAll([QuestID](const FQuest& q){ return q.QuestID == QuestID; });
         BidTimers.Remove(QuestID);
    }
}

void AMS_AIManager::RequestQuestCompletion(AMS_AICharacter* Character, FGuid QuestID)
{
	if(!Character) return;

	// Verify assignment
	if(AssignedQuests_.Contains(QuestID) && AssignedQuests_[QuestID] == Character)
	{
		int32 reward = Character->AssignedQuest.Reward;
		AActor* target = Character->AssignedQuest.TargetDestination.Get(); // Get target

		UE_LOG(LogTemp, Log, TEXT("AIManager: Quest %s completed by %s (Target: %s). Awarding %d money."),
			   *QuestID.ToString(), *Character->GetName(), *GetNameSafe(target), reward);

		// Award money
		Character->Money += reward;

		// Remove from assigned list
		AssignedQuests_.Remove(QuestID);

		// Remove from the specific site's tracking list if it was a delivery quest
		for (auto& Pair : ActiveConstructionDeliveryQuests)
		{
			if(Pair.Key.IsValid()) // Check if site still exists
			{
				Pair.Value.Remove(QuestID); // Remove the ID from the site's list
			}
		}
		// Optional: Clean up empty entries in ActiveConstructionDeliveryQuests map?

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AIManager: %s requested completion for Quest %s, but was not assigned or quest ID invalid."), *Character->GetName(), *QuestID.ToString());
	}
}


void AMS_AIManager::CheckAndInitiateConstruction() // Added parameter
{
    if (!Inventory_ || !ConstructionSiteClass) return;

    // --- Count Active Construction Projects ---
    int32 ActiveConstructionCount = 0;
    TArray<TWeakObjectPtr<AMS_ConstructionSite>> ActiveSites; // Keep track of sites being worked on
    for (const auto& Pair : ActiveConstructionDeliveryQuests)
    {
        if (!Pair.Value.IsEmpty() && Pair.Key.IsValid())
        {
            ActiveConstructionCount++;
            ActiveSites.AddUnique(Pair.Key); // Add site associated with active quests
        }
    }
     // Also count sites that exist but might have no *active* quests right now
     TArray<AActor*> FoundSites;
     UGameplayStatics::GetAllActorsOfClass(GetWorld(), ConstructionSiteClass, FoundSites);
     ActiveConstructionCount = FoundSites.Num(); // Simpler: Just count existing sites


    if (ActiveConstructionCount >= MaxConcurrentConstruction)
    {
        // UE_LOG(LogTemp, Verbose, TEXT("AIManager: Max concurrent constructions (%d) reached."), MaxConcurrentConstruction);
        return; // Limit reached
    }

    // --- Determine What to Build ---
    TSubclassOf<AActor> BuildingToSpawn = nullptr;
    ResourceType RequiredResource = ResourceType::ERROR;
    int32 ResourceCost = 0;

    if (HouseBuildingClass && Inventory_->GetResourceAmount(ResourceType::WOOD) >= HouseWoodCost)
    {
        BuildingToSpawn = HouseBuildingClass;
        RequiredResource = ResourceType::WOOD;
        ResourceCost = HouseWoodCost;
        UE_LOG(LogTemp, Log, TEXT("AIManager: Prioritizing House Construction."));
    }
    // TODO: Add logic for other building types here based on different conditions

    else if (HouseBuildingClass && Inventory_->GetResourceAmount(ResourceType::WOOD) >= HouseWoodCost) // Fallback to house if not prioritizing but possible
    {
         BuildingToSpawn = HouseBuildingClass;
         RequiredResource = ResourceType::WOOD;
         ResourceCost = HouseWoodCost;
         UE_LOG(LogTemp, Log, TEXT("AIManager: Considering non-priority House Construction."));
    }


    // --- Proceed if a building type was selected ---
    if (BuildingToSpawn && RequiredResource != ResourceType::ERROR && ResourceCost > 0)
    {
        // --- Find Build Location (2x2 Check) ---
        FVector BuildLocation = FVector::ZeroVector;
        TArray<FIntPoint> OccupiedNodes; // Store the nodes to block
        const int32 BuildSizeX = 2; // Example size
        const int32 BuildSizeY = 2;

        if (FindSuitableBuildLocation(BuildSizeX, BuildSizeY, BuildLocation, OccupiedNodes))
        {
            UE_LOG(LogTemp, Log, TEXT("AIManager: Initiating construction for %s at location %s."), *BuildingToSpawn->GetName(), *BuildLocation.ToString());
            StartBuildingProject(BuildingToSpawn, BuildLocation, RequiredResource, ResourceCost, OccupiedNodes); 
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AIManager: Met resource threshold for construction, but failed to find a suitable %dx%d build location."), BuildSizeX, BuildSizeY);
        }
    }
}

bool AMS_AIManager::FindSuitableBuildLocation(int32 SizeX, int32 SizeY, FVector& OutCenterLocation, TArray<FIntPoint>& OutOccupiedNodes)
{
    if (!PathfindingSubsystemCache) return false;
	
    const int MaxAttempts = 1050; 
    for (int Attempt = 0; Attempt < MaxAttempts; ++Attempt)
    {
        FVector RandomNodeLocation;
        FIntPoint StartNodeGridPos;
        if (PathfindingSubsystemCache->GetRandomFreeNode(RandomNodeLocation, StartNodeGridPos))
        {
            OutOccupiedNodes.Empty();
            bool bAreaIsFree = true;
            FVector TotalLocation = FVector::ZeroVector;

            // Check the SizeX * SizeY area starting from the random node
            for (int32 x = 0; x < SizeX; ++x)
            {
                for (int32 y = 0; y < SizeY; ++y)
                {
                    FIntPoint CurrentGridPos(StartNodeGridPos.X + x, StartNodeGridPos.Y + y);
                    TSharedPtr<FMoveNode> CurrentNode = PathfindingSubsystemCache->FindNodeByGridPosition(CurrentGridPos);

                    // Check if node exists and is free (not blocked)
                    if (!CurrentNode.IsValid())
                    {
                        bAreaIsFree = false;
                        break; // This node in the area is bad, stop checking this area
                    }
                    OutOccupiedNodes.Add(CurrentGridPos);
                    TotalLocation += CurrentNode->Position;
                }
                if (!bAreaIsFree) break; // Stop checking this area
            }

            // If the whole area was free, calculate center and return success
            if (bAreaIsFree && OutOccupiedNodes.Num() == (SizeX * SizeY))
            {
                OutCenterLocation = TotalLocation / static_cast<float>(OutOccupiedNodes.Num());
                return true;
            }
        }
    }

    // Failed to find a suitable location after attempts
    return false;
}

void AMS_AIManager::StartBuildingProject(TSubclassOf<AActor> BuildingClassToSpawn, const FVector& Location, ResourceType RequiredResource, int32 ResourceCost, const TArray<FIntPoint>& OccupiedNodes)
{
    if (!ConstructionSiteClass || !PathfindingSubsystemCache) return;

    UWorld* World = GetWorld();
    if (!World) return;

    //  Block Nodes Before Spawning 
    for (const FIntPoint& NodePos : OccupiedNodes)
    {
        TSharedPtr<FMoveNode> Node = PathfindingSubsystemCache->FindNodeByGridPosition(NodePos);
        if (Node.IsValid())
        {
             PathfindingSubsystemCache->BlockNode(Node->Position); 
        }
    }
	
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    AMS_ConstructionSite* NewSite = World->SpawnActor<AMS_ConstructionSite>(ConstructionSiteClass, Location, FRotator::ZeroRotator, SpawnParams);

    if (NewSite)
    {
        // Configure the site
        NewSite->BuildingClassToSpawn = BuildingClassToSpawn;
        NewSite->RequiredResource = RequiredResource;
        NewSite->AmountRequired = ResourceCost;
        NewSite->CurrentAmount = 0;
        NewSite->OccupiedNodes = OccupiedNodes; // Store occupied nodes for unblocking later

        UE_LOG(LogTemp, Log, TEXT("AIManager: Spawned Construction Site for %s requiring %d %s."), *BuildingClassToSpawn->GetName(), ResourceCost, *UEnum::GetValueAsString(RequiredResource));

        
        int32 RemainingCost = ResourceCost;
        int32 DeliveryQuestIndex = 0; 
        TArray<FGuid> GeneratedQuestIDs;

        while (RemainingCost > 0)
        {
            int32 deliveryAmount = FMath::Min(RemainingCost, DeliveryCarryCapacity);
            if (deliveryAmount <= 0) break; // Safety break

            // Calculate reward for this specific delivery trip
            int32 deliveryReward = CalculateGoldReward(RequiredResource, deliveryAmount);
            FQuest deliveryQuest(RequiredResource, deliveryAmount, deliveryReward, NewSite);

            AvailableQuests_.Add(deliveryQuest);
            GeneratedQuestIDs.Add(deliveryQuest.QuestID); // Track generated IDs for this site
            StartBidTimer(deliveryQuest);
            OnQuestAvailable.Broadcast(deliveryQuest);

            UE_LOG(LogTemp, Log, TEXT("AIManager: Generated Delivery Quest (Trip %d) ID %s - Deliver %d %s to site for %d reward."),
                DeliveryQuestIndex + 1, *deliveryQuest.QuestID.ToString(), deliveryAmount, *UEnum::GetValueAsString(RequiredResource), deliveryReward);

            RemainingCost -= deliveryAmount;
            DeliveryQuestIndex++;
        }
        // Associate these quests with the site
        ActiveConstructionDeliveryQuests.Add(NewSite, GeneratedQuestIDs);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIManager: Failed to spawn Construction Site Actor at %s."), *Location.ToString());
        // --- Unblock Nodes if Spawning Failed ---
        for (const FIntPoint& NodePos : OccupiedNodes)
        {
            TSharedPtr<FMoveNode> Node = PathfindingSubsystemCache->FindNodeByGridPosition(NodePos);
             if (Node.IsValid()) PathfindingSubsystemCache->UnblockNode(Node->Position); // Assuming UnblockNode exists
        }
    }
}

void AMS_AIManager::NotifyConstructionProgress(AMS_ConstructionSite* Site, int32 AmountDelivered)
{
	if(!Site) return;

	UE_LOG(LogTemp, Log, TEXT("AIManager: Received progress report from Site %s. Total delivered: %d / %d"),
		*Site->GetName(), Site->CurrentAmount, Site->AmountRequired);


}



void AMS_AIManager::UpdateHousingState()
{
	CurrentPopulation = 0;
	TotalHousingCapacity = 0;

	UWorld* World = GetWorld();
	if (!World) return;

	// Count Population
	if (AICharacterClass)
	{
		TArray<AActor*> FoundCharacters;
		UGameplayStatics::GetAllActorsOfClass(World, AICharacterClass, FoundCharacters);
		CurrentPopulation = FoundCharacters.Num();
	}

	// Count Housing Capacity 
	if (HouseBuildingClass) // Check if House Class is set
	{
		TArray<AActor*> FoundHouses;
		UGameplayStatics::GetAllActorsOfClass(World, HouseBuildingClass, FoundHouses);
		TotalHousingCapacity = FoundHouses.Num() * HouseCapacity;
	}

	// UE_LOG(LogTemp, Log, TEXT("AIManager Housing Check: Population=%d, Capacity=%d"), CurrentPopulation, TotalHousingCapacity);

	// Check if new housing is needed AND trigger construction check
	if (CurrentPopulation > TotalHousingCapacity)
	{
		UE_LOG(LogTemp, Log, TEXT("AIManager: Housing needed (Pop %d > Cap %d). Checking construction conditions."), CurrentPopulation, TotalHousingCapacity);
		CheckAndInitiateConstruction();
	}

}