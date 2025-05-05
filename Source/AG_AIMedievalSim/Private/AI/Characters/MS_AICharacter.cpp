// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Characters/MS_AICharacter.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Manager/MS_AIManager.h"
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "Placeables/Buildings/MS_ConstructionSite.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Placeables/Buildings/MS_StorageBuilding.h"
#include "Components/BoxComponent.h"
#include "Systems/MS_PawnStatComponent.h"
#include "Movement/MS_PathfindingSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

AMS_AICharacter::AMS_AICharacter()
{
	PrimaryActorTick.bCanEverTick = false; 


    ShopCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ShopCollision"));
	ShopCollision->SetupAttachment(RootComponent);
	ShopCollision->SetCollisionProfileName(TEXT("Trigger"));
	ShopCollision->OnComponentBeginOverlap.AddDynamic(this, &AMS_AICharacter::OnOverlapBegin);
    ShopCollision->OnComponentEndOverlap.AddDynamic(this, &AMS_AICharacter::OnOverlapEnd); // Add end overlap if needed

	PawnStats_ = CreateDefaultSubobject<UMS_PawnStatComponent>(TEXT("StatsComponent"));
	Inventory_ = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	WidgetComponent_ = CreateDefaultSubobject<UWidgetComponent>(TEXT("Stats"));
	WidgetComponent_->SetupAttachment(RootComponent);
	WidgetComponent_->SetWidgetSpace(EWidgetSpace::World);
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClass{ TEXT("/Game/Dynamic/UI/UI_PawnStats") };
	if (WidgetClass.Succeeded()) WidgetComponent_->SetWidgetClass((WidgetClass.Class));
	if(PawnStats_) PawnStats_->OnStateChanged.AddDynamic(this, &AMS_AICharacter::CheckIfHungry); // Connect stat delegate
}

void AMS_AICharacter::BeginPlay()
{
	Super::BeginPlay();

	UWorld* world = GetWorld();
	if (world) {

		AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_StorageBuildingPool::StaticClass());
		StorageBuldingsPool_ = FoundActor; 

		FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_WorkpPlacePool::StaticClass());
		WorkPlacesPool_ = FoundActor; 


		FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_AIManager::StaticClass());
		AIManager = Cast<AMS_AIManager>(FoundActor);

        // Get Pathfinding Subsystem
        PathfindingSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
        if (PathfindingSubsystem)
        {
            PathfindingSubsystem->OnPathUpdated.AddDynamic(this, &AMS_AICharacter::OnPathUpdated);
        }
        else UE_LOG(LogTemp, Error, TEXT("AICharacter %s: Pathfinding Subsystem not found!"), *GetName());


		// Bind to AIManager's quest announcement
		if (AIManager.IsValid())
		{
			AIManager->OnQuestAvailable.AddDynamic(this, &AMS_AICharacter::OnNewQuestReceived);
            UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Bound to AIManager quest announcements."), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AICharacter %s: AI Manager not found in world! Quest system inactive."), *GetName());
		}
	}

    // Initialize AssignedQuest ID to something invalid
    AssignedQuest.QuestID.Invalidate();
}

void AMS_AICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AMS_AICharacter::IsIdle() const
{
    AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(GetController());
    if (AIController && AIController->GetBlackboardComponent())
    {
        // Check relevant blackboard keys to determine idleness
        const bool bHasQuest = AIController->GetBlackboardComponent()->GetValueAsBool(FName("bHasQuest")); // Need this key
        const bool bIsSleeping = false; // TODO: Add Blackboard key bIsSleeping
        const bool bIsCriticallyNeedy = PawnStats_ ? (PawnStats_->IsHungry() || PawnStats_->IsThirsty()) : false; // Example check

        // Idle if no quest, not sleeping, and needs aren't critical
        return !bHasQuest && !bIsSleeping && !bIsCriticallyNeedy;
    }
    return false; 
}

void AMS_AICharacter::OnNewQuestReceived(const FQuest& NewQuest)
{
    UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Received announcement for Quest ID %s."), *GetName(), *NewQuest.QuestID.ToString());

	if (IsIdle()) // Only consider bidding if idle
	{
        UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Is Idle. Evaluating Quest ID %s."), *GetName(), *NewQuest.QuestID.ToString());
		EvaluateQuestAndBid(NewQuest);
	}
    else
    {
        UE_LOG(LogTemp, Verbose, TEXT("AICharacter %s: Not Idle. Ignoring Quest ID %s."), *GetName(), *NewQuest.QuestID.ToString());
    }
}

void AMS_AICharacter::EvaluateQuestAndBid(const FQuest& Quest)
{
	if (!AIManager.IsValid() || !PathfindingSubsystem || !PawnStats_)
	{
		UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Missing dependencies for quest evaluation."), *GetName());
		return;
	}

	float bidValue = CalculateBidValue(Quest);

    // Define a minimum acceptable bid threshold (don't bid if value is too low)
    const float MinBidThreshold = 1.0f;

	if (bidValue > MinBidThreshold)
	{
		AIManager->ReceiveBid(this, Quest, bidValue);
	}
    else
    {
         UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Calculated bid value %.2f for Quest ID %s is below threshold %.2f. Not bidding."), *GetName(), bidValue, *Quest.QuestID.ToString(), MinBidThreshold);
    }
}

float AMS_AICharacter::CalculateBidValue(const FQuest& Quest)
{
    if (!PathfindingSubsystem || !PawnStats_ || !WorkPlacesPool_.IsValid()) return 0.0f;

    // --- Factors ---
    float DistanceFactor = 1.0f;
    float NeedsPenalty = 1.0f;
    float RewardFactor = static_cast<float>(Quest.Reward);

    // 1. Distance Calculation 
    AMS_WorkpPlacePool* WPPool = Cast<AMS_WorkpPlacePool>(WorkPlacesPool_.Get());
    AMS_StorageBuildingPool* StoragePool = Cast<AMS_StorageBuildingPool>(StorageBuldingsPool_.Get());
    if (!WPPool || !StoragePool) return 0.0f; 

    // Find closest available resource node
    AMS_BaseWorkPlace* ClosestResource = nullptr;
	float MinResourceDistSq = FLT_MAX;
    FVector ResourceLocation = FVector::ZeroVector;

    for (TWeakObjectPtr<AMS_BaseWorkPlace> WorkplacePtr : WPPool->ActiveWorkplaces_)
	{
        if(WorkplacePtr.IsValid())
        {
            AMS_BaseWorkPlace* Workplace = WorkplacePtr.Get();
            if (Workplace->ResourceType_ == Quest.Type && Workplace->ResourceAvaliable_)
            {
                float DistSq = FVector::DistSquared(GetActorLocation(), Workplace->GetActorLocation());
                if (DistSq < MinResourceDistSq)
                {
                    MinResourceDistSq = DistSq;
                    ClosestResource = Workplace;
                    ResourceLocation = Workplace->GetActorLocation();
                }
            }
        }
	}

    if(!ClosestResource) {
        UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Cannot find available resource %s for bid calculation."), *GetName(), *UEnum::GetValueAsString(Quest.Type));
        return 0.0f; 
    }

    // Find closest Storage (assuming resource quests deliver to storage)
    AActor* DestinationActor = nullptr;
    if(Quest.TargetDestination.IsValid())
    {
        DestinationActor = Quest.TargetDestination.Get(); // For specific delivery quests
    }
    else // Default to nearest storage
    {
        AMS_StorageBuilding* ClosestStorage = nullptr;
        float MinStorageDistSq = FLT_MAX;
         for (TWeakObjectPtr<AMS_StorageBuilding> StoragePtr : StoragePool->StorageBuldings_) // Assumes StorageBuldings_ has active ones
        {
             if(StoragePtr.IsValid() && StoragePtr->placeActive_) // Check if storage is active
             {
                // Calculate distance from the RESOURCE location to the storage
                float DistSq = FVector::DistSquared(ResourceLocation, StoragePtr->GetActorLocation());
                 if (DistSq < MinStorageDistSq)
                 {
                     MinStorageDistSq = DistSq;
                     ClosestStorage = StoragePtr.Get();
                 }
             }
        }
        DestinationActor = ClosestStorage;
    }


    if(!DestinationActor) {
         UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Cannot find valid destination for bid calculation."), *GetName());
        return 0.0f; // Cannot calculate path if no destination
    }

	// TODO: Change the calculations so it checks the nav system
    float EstimateDistToResource = FMath::Abs(GetActorLocation().X - ResourceLocation.X) + FMath::Abs(GetActorLocation().Y - ResourceLocation.Y);
    float EstimateDistResourceToDest = FMath::Abs(ResourceLocation.X - DestinationActor->GetActorLocation().X) + FMath::Abs(ResourceLocation.Y - DestinationActor->GetActorLocation().Y);
    float TotalEstimatedDistance = EstimateDistToResource + EstimateDistResourceToDest;

    // Penalize longer distances 
    DistanceFactor = 1.0f / FMath::Max(1.0f, TotalEstimatedDistance / 1000.0f); 

    // 2. Needs Penalty 
    float HungerPenalty = FMath::Clamp(1.0f - (PawnStats_->GetHunger() / 100.0f), 0.0f, 1.0f); // 0 if full, 1 if empty
    float ThirstPenalty = FMath::Clamp(1.0f - (PawnStats_->GetThirst() / 100.0f), 0.0f, 1.0f);

    if (PawnStats_->IsHungry()) HungerPenalty *= 3.0f;
    if (PawnStats_->IsThirsty()) ThirstPenalty *= 3.0f;

    // Combine needs penalties 
    NeedsPenalty = 1.0f / FMath::Max(1.0f, 1.0f + HungerPenalty + ThirstPenalty); // Higher penalty = lower multiplier

    // Other Factors 
    // float SkillFactor = 1.0f; // TODO: Add skill system
    // float EnergyFactor = FMath::Clamp(PawnStats_->Energy / 100.0f, 0.1f, 1.0f); // Less likely to bid if tired


    float CalculatedValue = RewardFactor * NeedsPenalty * DistanceFactor; 

    UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Bid Calc for Quest %s: Reward=%.1f, DistFactor=%.2f (EstDist=%.0f), NeedsPenalty=%.2f -> BidValue=%.2f"),
        *GetName(), *Quest.QuestID.ToString(), RewardFactor, DistanceFactor, TotalEstimatedDistance, NeedsPenalty, CalculatedValue);

	return CalculatedValue;
}


void AMS_AICharacter::AssignQuest(const FQuest& Quest)
{
 AssignedQuest = Quest; // Assign the whole struct
	UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Assigned Quest ID %s (Type: %s, Amount: %d, Reward: %d, Target: %s)."), // Log target
		*GetName(),
		*Quest.QuestID.ToString(),
		*UEnum::GetValueAsString(Quest.Type),
		Quest.Amount,
        Quest.Reward,
        *GetNameSafe(Quest.TargetDestination.Get()));

	AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(GetController());
	if (AIController && AIController->GetBlackboardComponent())
	{
		UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
		Blackboard->SetValueAsBool(FName("bHasQuest"), true);
        Blackboard->SetValueAsEnum(FName("QuestType"), static_cast<uint8>(Quest.Type));
        Blackboard->SetValueAsInt(FName("QuestAmount"), Quest.Amount); // This is now the amount for THIS TRIP for fetch quests
        Blackboard->SetValueAsInt(FName("QuestReward"), Quest.Reward);
        Blackboard->SetValueAsString(FName("QuestID"), Quest.QuestID.ToString());
        Blackboard->SetValueAsObject(FName("QuestTargetDestination"), Quest.TargetDestination.Get());
		
        if (Quest.TargetDestination.IsValid() && AIManager.IsValid()) 
        {
          
            Blackboard->SetValueAsObject(FName("QuestResourceSource"), AIManager->CentralStorageBuilding.Get());
             UE_LOG(LogTemp, Verbose, TEXT("AICharacter %s: Setting QuestResourceSource to Central Storage for fetch quest."), *GetName());
        }
        else
        {
            Blackboard->ClearValue(FName("QuestResourceSource")); // Clear for gathering quests
        }

        Blackboard->SetValueAsObject(FName("Target"), nullptr);
        Blackboard->SetValueAsBool(FName("bIsAtWorkLocation"), false);
        Blackboard->ClearValue(FName("bIsFetchingConstructionMaterials"));
        Blackboard->ClearValue(FName("bIsDeliveringConstructionMaterials"));
        Blackboard->ClearValue(FName("bIsStoringGatheredItems"));
        Blackboard->ClearValue(FName("bGettingFood"));
        Blackboard->ClearValue(FName("bGettingWater"));
	}
    else {
         UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Failed to get Blackboard when assigning quest."), *GetName());
    }
}

void AMS_AICharacter::CompleteCurrentQuest()
{
    if (AssignedQuest.QuestID.IsValid() && AIManager.IsValid()) 
    {
        UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Requesting completion for Quest ID %s."), *GetName(), *AssignedQuest.QuestID.ToString());
        AIManager->RequestQuestCompletion(this, AssignedQuest.QuestID);
    	
         AssignedQuest.QuestID.Invalidate(); // Mark quest as invalid/done internally
         AssignedQuest.Type = ResourceType::ERROR;

         // Update blackboard
         AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(GetController());
         if (AIController && AIController->GetBlackboardComponent())
         {
             UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
             Blackboard->SetValueAsBool(FName("bHasQuest"), false);
             Blackboard->ClearValue(FName("QuestType")); // Clear details
             Blackboard->ClearValue(FName("QuestAmount"));
             Blackboard->ClearValue(FName("QuestReward"));
             Blackboard->ClearValue(FName("QuestID"));
             Blackboard->ClearValue(FName("QuestTargetDestination"));
              Blackboard->SetValueAsObject(FName("Target"), nullptr); // Clear target
         }
    }
     else {
         UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Tried to complete quest, but no valid quest assigned or AIManager invalid."), *GetName());
    }
}


void AMS_AICharacter::CheckIfHungry() {

	AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(this->GetController());
	if (AIController && AIController->GetBlackboardComponent() && PawnStats_)
    {
        UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
        bool bIsHungry = PawnStats_->IsHungry();
        bool bIsThirsty = PawnStats_->IsThirsty();

        Blackboard->SetValueAsBool(FName("bIsHungry"), bIsHungry); // Use dedicated keys
        Blackboard->SetValueAsBool(FName("bIsThirsty"), bIsThirsty);
		
	}
}

void AMS_AICharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{

	    AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(GetController());
    if (!AIController || !AIController->GetBlackboardComponent()) return;
    UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
    AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(FName("Target"))); // Get current movement target

    // Interaction with CENTRAL STORAGE BUILDING 
	AMS_StorageBuilding* StorageBuilding = Cast<AMS_StorageBuilding>(OtherActor);
	if (StorageBuilding && CurrentTarget == StorageBuilding) // Only interact if it was the destination
	{
        UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Overlapped with Target Storage %s."), *GetName(), *StorageBuilding->GetName());

        // Fetching for Construction/Delivery 
        if (Blackboard->GetValueAsBool(FName("bIsFetchingConstructionMaterials")) && AssignedQuest.QuestID.IsValid())
        {
            ResourceType typeNeeded = AssignedQuest.Type;
            int32 amountNeededForTrip = AssignedQuest.Amount; // Amount for this specific trip

            UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Attempting to fetch %d %s for Quest %s."),
                *GetName(), amountNeededForTrip, *UEnum::GetValueAsString(typeNeeded), *AssignedQuest.QuestID.ToString());

            UInventoryComponent* StorageInventory = StorageBuilding->Inventory_;
            if (StorageInventory && StorageInventory->GetResourceAmount(typeNeeded) >= amountNeededForTrip)
            {
                // Extract from storage
                int32 extractedAmount = StorageInventory->ExtractFromResources(typeNeeded, amountNeededForTrip);
                if(extractedAmount > 0)
                {
                    // Add to AI inventory
                    Inventory_->AddToResources(typeNeeded, extractedAmount);
                    UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Successfully fetched %d %s from storage."), *GetName(), extractedAmount, *UEnum::GetValueAsString(typeNeeded));

                    // Update State: Now delivering
                    Blackboard->SetValueAsBool(FName("bIsFetchingConstructionMaterials"), false);
                    Blackboard->SetValueAsBool(FName("bIsDeliveringConstructionMaterials"), true);

                    // Set new Target: The actual construction site
                    AActor* Destination = AssignedQuest.TargetDestination.Get();
                    if(Destination)
                    {
                        Blackboard->SetValueAsObject(FName("Target"), Destination);
                        CreateMovementPath(Destination); 
                    }
                    else {
                         UE_LOG(LogTemp, Error, TEXT("AICharacter %s: Fetched items but QuestTargetDestination is invalid! Quest stuck."), *GetName());
                         // Handle error state in BT? Maybe drop items? Complete quest with failure?
                    }
                }
                else {
                    UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Storage %s ExtractFromResources failed (returned %d) even though check passed?"), *GetName(), *StorageBuilding->GetName(), extractedAmount);
                    // BT needs to handle failure (wait and retry?)
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Reached storage %s to fetch %d %s, but not enough available!"),
                    *GetName(), *StorageBuilding->GetName(), amountNeededForTrip, *UEnum::GetValueAsString(typeNeeded));
                // BT needs to handle failure (wait for resources, abandon quest?)
                 Blackboard->SetValueAsBool(FName("bIsFetchingConstructionMaterials"), false); // Stop trying for now?
                 // Maybe clear quest? Or just let AI idle until resources appear?
            }
        }

        // Storing GATHERED resources
        else if (Blackboard->GetValueAsBool(FName("bIsStoringGatheredItems")) && AssignedQuest.QuestID.IsValid())
        {
             ResourceType typeToStore = AssignedQuest.Type;
             int32 amountToStore = Inventory_->GetResourceAmount(typeToStore);

             if (amountToStore > 0)
             {
                 StorageBuilding->Inventory_->AddToResources(typeToStore, amountToStore);
                 Inventory_->ExtractFromResources(typeToStore, amountToStore); // Remove from AI
                 UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Stored %d %s (gathered) at %s."), *GetName(), amountToStore, *UEnum::GetValueAsString(typeToStore), *StorageBuilding->GetName());

                 // Complete the GATHER quest
                 CompleteCurrentQuest();

                 Blackboard->SetValueAsBool(FName("bIsStoringGatheredItems"), false); // Reset state
             }
             else {
                  UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Reached storage to store %s, but has none."), *GetName(), *UEnum::GetValueAsString(typeToStore));
                  CompleteCurrentQuest(); 
                  Blackboard->SetValueAsBool(FName("bIsStoringGatheredItems"), false);
             }
        }

        // Getting Food/Water for SELF 
        else if (Blackboard->GetValueAsBool(FName("bGettingFood")))
        {
            UInventoryComponent* StorageInventory = StorageBuilding->Inventory_;
            if (StorageInventory && StorageInventory->ExtractFromResources(ResourceType::BERRIES, 20) != -1)
            {
                PawnStats_->ModifyHunger(100);
                UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Took Berries for self from %s."), *GetName(), *StorageBuilding->GetName());
                Blackboard->SetValueAsBool(FName("bGettingFood"), false);
                CheckIfHungry(); // Update need state
            } else
            {
            	Blackboard->SetValueAsBool(FName("bEmergencyFood"), true);
	            UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Failed to take Berries for self from %s."), *GetName(), *StorageBuilding->GetName());
            }
        }
        else if (Blackboard->GetValueAsBool(FName("bGettingWater")))
        {
             UInventoryComponent* StorageInventory = StorageBuilding->Inventory_;
            if (StorageInventory && StorageInventory->ExtractFromResources(ResourceType::WATER, 20) != -1)
            {
                PawnStats_->ModifyThirst(100);
                UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Took Water for self from %s."), *GetName(), *StorageBuilding->GetName());
                Blackboard->SetValueAsBool(FName("bGettingWater"), false);
                CheckIfHungry(); // Update need state
            } else
            {
            	Blackboard->SetValueAsBool(FName("bEmergencyWater"), true);

	            UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Failed to take Water for self from %s."), *GetName(), *StorageBuilding->GetName());
            }
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Reached storage %s but has no clear action state (Fetching/Storing/Needs)."), *GetName(), *StorageBuilding->GetName());
        }
	} 

	// Collision with workplace 
	AMS_BaseWorkPlace* WorkPlace = Cast<AMS_BaseWorkPlace>(OtherActor);
	if (WorkPlace && Blackboard->GetValueAsObject("Target") == WorkPlace)
	{
		Blackboard->SetValueAsBool(FName("AtWorkLocation"), true);
		UE_LOG(LogTemp, Log, TEXT("AI Character '%s' reached target workplace '%s'. Setting AtWorkLocation=true."), *GetNameSafe(this), *GetNameSafe(WorkPlace));
	}

	   AMS_ConstructionSite* Site = Cast<AMS_ConstructionSite>(OtherActor);
    if (Site && CurrentTarget == Site)
    {
         UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Overlapped with Target Construction Site %s."), *GetName(), *Site->GetName());

         bool bIsDelivering = Blackboard->GetValueAsBool(FName("bIsDeliveringConstructionMaterials"));

         if (bIsDelivering && AssignedQuest.QuestID.IsValid() && AssignedQuest.TargetDestination == Site)
         {
             ResourceType neededType = AssignedQuest.Type;
             int32 hasAmount = Inventory_->GetResourceAmount(neededType);
             int32 amountToDeliver = FMath::Min(hasAmount, AssignedQuest.Amount); // Deliver amount for this trip

             if (amountToDeliver > 0)
             {
                 if(Site->AddResource(amountToDeliver)) // Site completed
                 { UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Delivered final %d %s to %s. Construction complete."), *GetName(), amountToDeliver, *UEnum::GetValueAsString(neededType), *Site->GetName()); }
                 else { UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Delivered %d %s to %s. Progress: %d/%d"), *GetName(), amountToDeliver, *UEnum::GetValueAsString(neededType), *Site->GetName(), Site->CurrentAmount, Site->AmountRequired); }

                 Inventory_->ExtractFromResources(neededType, amountToDeliver);
                 CompleteCurrentQuest(); // Complete THIS delivery trip quest
                 Blackboard->SetValueAsBool(FName("bIsDeliveringConstructionMaterials"), false); // Reset state
             }
             else
             {
                 UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Reached site %s for delivery, but has no %s."), *GetName(), *Site->GetName(), *UEnum::GetValueAsString(neededType));
                 // BT needs to handle failure (e.g., go back to storage?)
                 Blackboard->SetValueAsBool(FName("bIsDeliveringConstructionMaterials"), false); // Reset state
             }
         }
          else {
             UE_LOG(LogTemp, Warning, TEXT("AICharacter %s: Reached site %s but wasn't delivering or quest invalid."), *GetName(), *Site->GetName());
         }
    }

	
}

void AMS_AICharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    AMS_BaseWorkPlace* WorkPlace = Cast<AMS_BaseWorkPlace>(OtherActor);
	AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(GetController());
    if (WorkPlace && AIController && AIController->GetBlackboardComponent())
    {
        UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
        // Only clear if they were targeting this specific workplace
        if (Blackboard->GetValueAsObject("Target") == WorkPlace)
        {
             // Check if currently supposed to be working there
             if(Blackboard->GetValueAsBool(FName("AtWorkLocation")))
             {
                 //Blackboard->SetValueAsBool(FName("AtWorkLocation"), false);
                 UE_LOG(LogTemp, Log, TEXT("AI Character '%s' left workplace '%s' trigger. Setting AtWorkLocation=false."), *GetNameSafe(this), *GetNameSafe(WorkPlace));
                 // The Decorator on the PerformWorkAction sequence should abort the task.
             }
        }
    }
}


TArray<FIntPoint> AMS_AICharacter::CreateMovementPath(AActor* TargetActor) {
	Path_.Empty(); // Clear previous path
    CurrentNodeIndex = -1; // Reset index

	if (!PathfindingSubsystem || !TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateMovementPath: PathfindingSubsystem or TargetActor invalid for %s."), *GetName());
		return TArray<FIntPoint>();
	}

	TSharedPtr<FMoveNode> Begin = PathfindingSubsystem->FindClosestNodeToActor(this);
	TSharedPtr<FMoveNode> End = PathfindingSubsystem->FindClosestNodeToActor(TargetActor);

    if(!Begin.IsValid() || !End.IsValid())
    {
       UE_LOG(LogTemp, Warning, TEXT("CreateMovementPath: Could not find valid start or end node for path between %s and %s."), *GetName(), *TargetActor->GetName());
       return TArray<FIntPoint>();
    }

	Path_ = PathfindingSubsystem->FindPathPoints(Begin, End);

    if(Path_.Num() > 0)
    {
         UE_LOG(LogTemp, Verbose, TEXT("CreateMovementPath: Found path with %d nodes for %s to %s."), Path_.Num(), *GetName(), *TargetActor->GetName());
         CurrentNodeIndex = 0; // Start at the beginning
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateMovementPath: Pathfinding returned empty path for %s to %s."), *GetName(), *TargetActor->GetName());
    }

	return Path_; // Return the member variable path
}


void AMS_AICharacter::OnPathUpdated(FIntPoint ChangedNodePos)
{
	if (Path_.Num() == 0 || CurrentNodeIndex < 0) return; // No active path or already finished

    bool bPathAffected = false;
    // Check if the changed node is part of the *remaining* path
	for (int32 i = CurrentNodeIndex; i < Path_.Num(); ++i)
	{
		if (ChangedNodePos == Path_[i])
		{
            bPathAffected = true;
			break;
		}
	}

    if(bPathAffected)
    {
        UE_LOG(LogTemp, Warning, TEXT("AI Path for %s affected! Recalculating..."), *GetName());

        // Get the final destination actor from the blackboard 'Target' key
        AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(GetController());
        if (!AIController || !AIController->GetBlackboardComponent()) return;
        AActor* TargetActor = Cast<AActor>(AIController->GetBlackboardComponent()->GetValueAsObject("Target"));

        if(TargetActor)
        {
            // Recalculate the whole path
            CreateMovementPath(TargetActor);
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("OnPathUpdated: Cannot recalculate path, Target actor not found in Blackboard for %s."), *GetName());
             Path_.Empty(); // Invalidate current path
             CurrentNodeIndex = -1;
             // BT needs to handle path failure
        }
    }
}


void AMS_AICharacter::ConsumeResourceDirectly(ResourceType type, int32 ammount) {
	switch (type)
	{
	case ResourceType::BERRIES:
        if(PawnStats_) PawnStats_->ModifyHunger(100); // Example value
		break;
	case ResourceType::WATER:
        if(PawnStats_) PawnStats_->ModifyThirst(100); // Example value
		break;
	default: 
		break;
	}
     UE_LOG(LogTemp, Log, TEXT("AICharacter %s: Consumed %s directly."), *GetName(), *UEnum::GetValueAsString(type));
}

// SetupPlayerInputComponent - Keep as is or remove if AI never possessed by player
void AMS_AICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}