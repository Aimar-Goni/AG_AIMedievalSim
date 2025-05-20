// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Manager/MS_AIManager.h"
#include "Systems/MS_InventoryComponent.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Placeables/Buildings/MS_House.h"
#include "Placeables/Buildings/MS_WheatField.h"
#include "Movement/MS_PathfindingSubsystem.h"


AMS_AIManager::AMS_AIManager()
{
	PrimaryActorTick.bCanEverTick = true; 
}

void AMS_AIManager::BeginPlay()
{
    Super::BeginPlay();

	UWorld* world = GetWorld();
	
	InitializeCentralStorage();

	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_StorageBuildingPool::StaticClass());
	StorageBuldingsPool_ = FoundActor ? Cast<AMS_StorageBuildingPool>(FoundActor) : world->SpawnActor<AMS_StorageBuildingPool>();
	
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
	InitializeFieldListeners();
	GetWorldTimerManager().SetTimer(HousingCheckTimerHandle, this, &AMS_AIManager::UpdateHousingState, HousingCheckInterval, true, 5.0f);

	GetWorldTimerManager().SetTimer(TavernCheckTimerHandle, this, &AMS_AIManager::CheckAndInitiateConstruction, TavernCheckInterval, true); 
}

void AMS_AIManager::InitializeCentralStorage()
{
	TArray<AActor*> FoundStorages;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMS_StorageBuilding::StaticClass(), FoundStorages);

	if (FoundStorages.Num() == 1)
	{
		CentralStorageBuilding = Cast<AMS_StorageBuilding>(FoundStorages[0]);
		if (CentralStorageBuilding.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("AIManager: Found Central Storage Building: %s"), *CentralStorageBuilding->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AIManager: Found actor of StorageBuilding class, but failed to cast or it's invalid!"));
		}
	}
	else if (FoundStorages.Num() > 1)
	{
		UE_LOG(LogTemp, Error, TEXT("AIManager: Found %d Storage Buildings! Only ONE central storage is supported in this configuration. Using the first one found: %s"), FoundStorages.Num(), *GetNameSafe(FoundStorages[0]));
		CentralStorageBuilding = Cast<AMS_StorageBuilding>(FoundStorages[0]);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AIManager: CRITICAL - No Storage Building found in the level! Resource checks will fail."));
	}
}
UInventoryComponent* AMS_AIManager::GetCentralStorageInventory() const
{
	if (CentralStorageBuilding.IsValid())
	{
		return CentralStorageBuilding->Inventory_;
	}
	return nullptr;
}

void AMS_AIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (ResourceType type : ManagedResourceTypes)
	{
		GenerateQuestsForResourceType(type);
	}
	
	GetWorld()->GetWorldSettings()->SetTimeDilation(gameSpeed);
}
static int32 CalculateGoldReward(ResourceType Resource, int32 Amount)
{
	int32 GoldPerUnit = 0;

	switch (Resource)
	{
	case ResourceType::WHEAT:
		GoldPerUnit = 4;
		break;
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
	UInventoryComponent* StorageInventory = GetCentralStorageInventory();
	if (!StorageInventory || ResourceTypeToCheck == ResourceType::ERROR) return;

	int32 currentAmount = StorageInventory->GetResourceAmount(ResourceTypeToCheck);

	if (currentAmount < LowResourceThreshold)
	{
		int32 neededResources = LowResourceThreshold - currentAmount;
		// UE_LOG(LogTemp, Log, TEXT("AIManager: Low on %s (%d/%d). Need %d."), *UEnum::GetValueAsString(ResourceTypeToCheck), currentAmount, LowResourceThreshold, neededResources);

		while (neededResources > 0)
		{
			int32 questAmount = FMath::Min(neededResources, MaxResourcePerQuest);
			if (questAmount <= 0) break; // Safety break

			// --- Check for Duplicates ---
			if (!DoesIdenticalQuestExist(ResourceTypeToCheck, questAmount, nullptr))
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

bool AMS_AIManager::DoesIdenticalQuestExist(ResourceType Type, int32 Amount, AActor* Target ) const
{
	for (const FQuest& availableQuest : AvailableQuests_)
	{
		if (availableQuest.Type == Type && availableQuest.Amount == Amount && availableQuest.TargetDestination == Target)
		{

			if (!BidTimers.Contains(availableQuest.QuestID) && !AssignedQuests_.Contains(availableQuest.QuestID))
				return true; // Found identical available quest
			if (BidTimers.Contains(availableQuest.QuestID))
				return true; // Found identical quest being bid on
		}
	}
	for (const auto& pair : AssignedQuests_)
	{
		AMS_AICharacter* character = pair.Value.Get();
		if (character && character->AssignedQuest.Type == Type && character->AssignedQuest.Amount == Amount && character->AssignedQuest.TargetDestination == Target)
		{
			return true; // Found identical assigned quest
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

		if (winner && !winner->AssignedQuest.QuestID.IsValid())
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
			UE_LOG(LogTemp, Warning, TEXT("AIManager: No valid winner found for Quest ID %s. Quest sent again"), *QuestID.ToString());
			AvailableQuests_.RemoveAll([QuestID](const FQuest& q){ return q.QuestID == QuestID; });

			CurrentBids.Remove(QuestID);
			BidTimers.Remove(QuestID);
			
			AvailableQuests_.Add(originalQuest);
			StartBidTimer(originalQuest);
			OnQuestAvailable.Broadcast(originalQuest);
			
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
     						
     	AvailableQuests_.Add(originalQuest);
     	StartBidTimer(originalQuest);
     	OnQuestAvailable.Broadcast(originalQuest);
				
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
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AIManager: %s requested completion for Quest %s, but was not assigned or quest ID invalid."), *Character->GetName(), *QuestID.ToString());
	}
}


void AMS_AIManager::CheckAndInitiateConstruction() // Added parameter
{
	UInventoryComponent* StorageInventory = GetCentralStorageInventory();
    if (!StorageInventory || !ConstructionSiteClass) return;

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
	bool bIsWheatField = false;
	int32 woodAmount = GetCentralStorageInventory() ? GetCentralStorageInventory()->GetResourceAmount(ResourceType::WOOD) : 0;

	if (CurrentPopulation > TotalHousingCapacity || (CurrentPopulation > 0 && TotalHousingCapacity == 0) ) 
	{
		BuildingToSpawn = HouseBuildingClass;
		RequiredResource = ResourceType::WOOD;
		ResourceCost = HouseWoodCost;
		UE_LOG(LogTemp, Log, TEXT("AIManager: Prioritizing House Construction."));
	}
	else if (ShouldBuildTavern() && TavernBuildingClass && woodAmount >= TavernWoodCost)
	{
		BuildingToSpawn = TavernBuildingClass;
		RequiredResource = ResourceType::WOOD; 
		ResourceCost = TavernWoodCost;
		UE_LOG(LogTemp, Log, TEXT("AIManager: Conditions met for Tavern Construction."));
	}
	else if (ShouldBuildWheatField() && WheatFieldClass && woodAmount >= WheatFieldWoodCost)
	{
		BuildingToSpawn = WheatFieldClass;
		RequiredResource = ResourceType::WOOD;
		ResourceCost = WheatFieldWoodCost;
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
bool AMS_AIManager::ShouldBuildWheatField() const
{
	
	const int32 MaxFields = MaxWheatField; 
	TArray<AActor*> FoundFields;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), WheatFieldClass, FoundFields);

	UInventoryComponent* StorageInv = GetCentralStorageInventory();
	int32 CurrentWheat = StorageInv ? StorageInv->GetResourceAmount(ResourceType::WHEAT) : 0;

	return CurrentWheat < (LowResourceThreshold * 2) && FoundFields.Num() < MaxFields;
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
                    if (!CurrentNode.IsValid() )
                    {
                    	bAreaIsFree = false;
                    	break; 
                    }
                	if (PathfindingSubsystemCache->IsNodeBlocked(CurrentGridPos))
                	{
                		bAreaIsFree = false;
                		break; 
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
		PathfindingSubsystemCache->OnPathUpdated.Broadcast(NodePos);
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

    	PathfindingSubsystemCache->AddNodeAtPosition(Location);
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

void AMS_AIManager::InitializeFieldListeners()
{
	TArray<AActor*> FoundFields;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMS_WheatField::StaticClass(), FoundFields);
	UE_LOG(LogTemp, Log, TEXT("AIManager: Found %d Wheat Fields to listen to."), FoundFields.Num());
	for (AActor* FieldActor : FoundFields)
	{
		AMS_WheatField* Field = Cast<AMS_WheatField>(FieldActor);
		if (Field)
		{
			Field->OnFieldNeedsPlanting.AddDynamic(this, &AMS_AIManager::OnWheatFieldNeedsPlanting);
			Field->OnFieldNeedsWatering.AddDynamic(this, &AMS_AIManager::OnWheatFieldNeedsWatering);
			Field->OnFieldReadyToHarvest.AddDynamic(this, &AMS_AIManager::OnWheatFieldReadyToHarvest); 

			// Initial checks
			switch(Field->GetCurrentFieldState())
			{
			case EFieldState::Constructed: 
			case EFieldState::Harvested: OnWheatFieldNeedsPlanting(Field); break;
			case EFieldState::Planted: OnWheatFieldNeedsWatering(Field); break;
			case EFieldState::ReadyToHarvest: OnWheatFieldReadyToHarvest(Field); break;
			default: break;
			}
		}
	}
}

void AMS_AIManager::OnWheatFieldNeedsPlanting(AMS_WheatField* Field)
{
    if (!Field || Field->GetCurrentFieldState() != EFieldState::Constructed) return; // Double check state
    UE_LOG(LogTemp, Log, TEXT("AIManager: Field %s needs planting."), *Field->GetName());

    if (!DoesIdenticalQuestExist(ResourceType::WHEAT, -1, Field))
    {
        FQuest plantQuest(ResourceType::WHEAT, -1, 2, Field); // Small reward for planting
        AvailableQuests_.Add(plantQuest);
        StartBidTimer(plantQuest);
        OnQuestAvailable.Broadcast(plantQuest);
        UE_LOG(LogTemp, Log, TEXT("AIManager: Generated PLANTING Quest ID %s for Field %s."), *plantQuest.QuestID.ToString(), *Field->GetName());
    }
}

void AMS_AIManager::OnWheatFieldNeedsWatering(AMS_WheatField* Field)
{
    if (!Field || Field->GetCurrentFieldState() != EFieldState::Planted) return;
    UE_LOG(LogTemp, Log, TEXT("AIManager: Field %s needs watering."), *Field->GetName());
	
    if (!DoesIdenticalQuestExist(ResourceType::WATER, 1, Field))
    {
        int32 waterAmountNeeded = 1; // For one watering action
        int32 reward = CalculateGoldReward(ResourceType::WATER, waterAmountNeeded) + 1; 
        FQuest waterQuest(ResourceType::WATER, waterAmountNeeded, reward, Field);

        AvailableQuests_.Add(waterQuest);
        StartBidTimer(waterQuest);
        OnQuestAvailable.Broadcast(waterQuest);
        UE_LOG(LogTemp, Log, TEXT("AIManager: Generated WATERING Quest ID %s for Field %s (Fetch %d Water)."), *waterQuest.QuestID.ToString(), *Field->GetName(), waterAmountNeeded);
    }
}

void AMS_AIManager::OnWheatFieldReadyToHarvest(AMS_WheatField* Field)
{
    if (!Field || Field->GetCurrentFieldState() != EFieldState::ReadyToHarvest) return;
    UE_LOG(LogTemp, Log, TEXT("AIManager: Field %s ready for harvest."), *Field->GetName());

    if (!DoesIdenticalQuestExist(ResourceType::WHEAT, Field->HarvestAmount, Field))
    {
        int32 reward = CalculateGoldReward(ResourceType::WHEAT, Field->HarvestAmount);
        FQuest harvestQuest(ResourceType::WHEAT, Field->HarvestAmount, reward, Field);

        AvailableQuests_.Add(harvestQuest);
        StartBidTimer(harvestQuest);
        OnQuestAvailable.Broadcast(harvestQuest);
        UE_LOG(LogTemp, Log, TEXT("AIManager: Generated HARVEST Quest ID %s for Field %s (Yield %d)."), *harvestQuest.QuestID.ToString(), *Field->GetName(), Field->HarvestAmount);
    }
}
void AMS_AIManager::UpdateHousingState()
{
    CurrentPopulation = 0;
    TotalHousingCapacity = 0;
    TArray<AMS_House*> AvailableHouses; 

    UWorld* World = GetWorld();
    if (!World) return;

    if (AICharacterClass)
    {
        TArray<AActor*> FoundCharacters;
        UGameplayStatics::GetAllActorsOfClass(World, AICharacterClass, FoundCharacters);
        CurrentPopulation = FoundCharacters.Num();

        // --- Assign Houses to Homeless AI ---
        TArray<AMS_AICharacter*> HomelessAI;
        for (AActor* CharActor : FoundCharacters)
        {
            AMS_AICharacter* AIChar = Cast<AMS_AICharacter>(CharActor);
            if (AIChar && !AIChar->GetAssignedHouse())
            {
                HomelessAI.Add(AIChar);
            }
        }

        if (HouseBuildingClass)
        {
            TArray<AActor*> FoundHouses;
            UGameplayStatics::GetAllActorsOfClass(World, HouseBuildingClass, FoundHouses);
            for (AActor* HouseActor : FoundHouses)
            {
                AMS_House* House = Cast<AMS_House>(HouseActor);
                if (House)
                {
                    TotalHousingCapacity += House->MaxOccupants;
                    if (House->HasSpace())
                    {
                        AvailableHouses.Add(House);
                    }
                }
            }
        }

        // Assign homeless AI to available houses
        for (AMS_AICharacter* HomelessChar : HomelessAI)
        {
            for (AMS_House* House : AvailableHouses)
            {
                if (House->HasSpace())
                {
                    HomelessChar->SetAssignedHouse(House);
                    AvailableHouses.Remove(House); 
                    if(House->HasSpace()) AvailableHouses.Add(House); 
                    break; 
                }
            }
        }
    }

        UE_LOG(LogTemp, Log, TEXT("AIManager Housing Check: Housing needed (Pop %d > Cap %d). Checking construction conditions."), CurrentPopulation, TotalHousingCapacity);
        CheckAndInitiateConstruction(); // Prioritize house
    
}

bool AMS_AIManager::ShouldBuildTavern() const
{
	if (!AICharacterClass || !TavernBuildingClass) return false; // Need classes to check

	UWorld* World = GetWorld();
	if (!World) return false;

	// 1. Check Max Taverns Limit
	TArray<AActor*> FoundTaverns;
	UGameplayStatics::GetAllActorsOfClass(World, TavernBuildingClass, FoundTaverns);
	if (FoundTaverns.Num() >= MaxTaverns)
	{
		return false;
	}

	// 2. Check Average Population Happiness
	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(World, AICharacterClass, FoundCharacters);

	if (FoundCharacters.IsEmpty()) return false; // No population, no need for a pub

	float TotalHappiness = 0.0f;
	int32 ValidCharacterCount = 0;
	for (AActor* CharActor : FoundCharacters)
	{
		AMS_AICharacter* AIChar = Cast<AMS_AICharacter>(CharActor);
		if (AIChar && AIChar->PawnStats_)
		{
			TotalHappiness += AIChar->PawnStats_->GetHappiness();
			ValidCharacterCount++;
		}
	}

	if (ValidCharacterCount == 0) return false;

	float AverageHappiness = TotalHappiness / static_cast<float>(ValidCharacterCount);

	if (AverageHappiness < MinAverageHappinessForTavernConsideration)
	{
		UE_LOG(LogTemp, Log, TEXT("AIManager: Average happiness (%.1f) is below threshold (%.1f). Considering Tavern."), AverageHappiness, MinAverageHappinessForTavernConsideration);
		return true;
	}

	return false;
}
