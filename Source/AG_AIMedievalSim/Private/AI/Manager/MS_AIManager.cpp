// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Manager/MS_AIManager.h"
#include "Systems/MS_InventoryComponent.h"
#include "AI/Characters/MS_AICharacter.h" 
#include "Kismet/GameplayStatics.h"

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


    // if (BulletingBoardPool_.IsValid())
    // {
    //     // BulletingBoardPool_->OnBulletingBoardPoolInitialized.AddDynamic(this, &AMS_AIManager::OnBulletingBoardPoolReady);
    // }

}

void AMS_AIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    // Periodically check if new quests need to be generated
    GenerateQuests();
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
void AMS_AIManager::GenerateQuests()
{

    ResourceType typeToGenerate = ResourceType::WOOD; // Example
    int32 currentAmount = Inventory_ ? Inventory_->GetResourceAmount(typeToGenerate) : 0;

    if (currentAmount < LowResourceThreshold)
    {
        int32 neededResources = LowResourceThreshold - currentAmount;
       // UE_LOG(LogTemp, Log, TEXT("AIManager: Low on %s (%d/%d). Generating quests."), *UEnum::GetValueAsString(typeToGenerate), currentAmount, LowResourceThreshold);

        while (neededResources > 0)
        {
            int32 questAmount = FMath::Min(neededResources, MaxResourcePerQuest);
        	
            bool bQuestExists = false;
            // Check AvailableQuests
            for (const FQuest& availableQuest : AvailableQuests_)
            {
                if (availableQuest.Type == typeToGenerate && availableQuest.Amount == questAmount)
                {
                    bQuestExists = true;
                    break;
                }
            }
            // Check CurrentBids (quests being bid on)
            if (!bQuestExists)
            {
                 for(const auto& bidPair : CurrentBids)
                 {
                    // Need to find the quest details for the ID being bid on
                    // This requires AvailableQuests_ to hold quests even while bidding is active
                    // Let's adjust the flow slightly: Keep quest in Available until assigned.
                 }
                 // More efficient: Check if a bid is active for a quest matching type/amount.
                 // This requires storing quest details with bids or looking up in Available.
                 // For now, let's skip this check for simplicity, potentially creating duplicate quests if generated quickly.
                 // TODO: Add robust check against CurrentBids if needed.
            }
            // Check AssignedQuests (if we want only one active quest of a type/amount)
            // TODO: Add check against AssignedQuests if needed.


            if (!bQuestExists) // Simplified check for now
            {
                // Calculate reward
                int32 reward = CalculateGoldReward(typeToGenerate, questAmount);

                // Create the quest (assigns new GUID automatically)
                FQuest newQuest(typeToGenerate, questAmount, reward);

                // Add to available list
                AvailableQuests_.Add(newQuest);

                UE_LOG(LogTemp, Log, TEXT("AIManager: Generated Quest ID %s - Gather %d %s for %d reward."), *newQuest.QuestID.ToString(), questAmount, *UEnum::GetValueAsString(typeToGenerate), reward);

            	// Start the bidding timer immediately after announcing
                 StartBidTimer(newQuest);

                // Broadcast to listening AI
                OnQuestAvailable.Broadcast(newQuest);

            }

            neededResources -= questAmount;

            // Safety break to prevent infinite loop if logic is flawed
            if (questAmount <= 0) break;
        }
    }
     // TODO: Add generation logic for other resource types (Berries, Water, Wheat etc.)
     // TODO: Add generation logic for Construction delivery quests
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
	
    if(AssignedQuests_.Contains(QuestID) && AssignedQuests_[QuestID] == Character)
    {

        int32 reward = Character->AssignedQuest.Reward; // Get reward from AI's stored quest

        UE_LOG(LogTemp, Log, TEXT("AIManager: Quest %s completed by %s. Awarding %d money."), *QuestID.ToString(), *Character->GetName(), reward);


        Character->Money += reward;

        AssignedQuests_.Remove(QuestID);
    	
    }
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("AIManager: %s requested completion for Quest %s, but was not assigned or quest ID invalid."), *Character->GetName(), *QuestID.ToString());
    }
}