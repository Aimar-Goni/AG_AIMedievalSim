// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Manager/MS_AIManager.h"
#include "Systems/MS_InventoryComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AMS_AIManager::AMS_AIManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Inventory Component
	Inventory_ = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));


}

// Called when the game starts or when spawned
void AMS_AIManager::BeginPlay()
{

	UWorld* world = GetWorld();
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_StorageBuildingPool::StaticClass());
	if (FoundActor) {
		AMS_StorageBuildingPool* Pool = Cast<AMS_StorageBuildingPool>(FoundActor);

		StorageBuldingsPool_ = Pool;
		
	}
	else {
		StorageBuldingsPool_ = world->SpawnActor<AMS_StorageBuildingPool>(AMS_StorageBuildingPool::StaticClass());
	}
	for (AMS_StorageBuilding* Storage : StorageBuldingsPool_->StorageBuldings_)
	{
		Storage->Inventory_->OnResourceChanged.AddDynamic(this, &AMS_AIManager::UpdateResources);
	}
	FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_BulletingBoardPool::StaticClass());
	if (FoundActor) {
		AMS_BulletingBoardPool* Pool = Cast<AMS_BulletingBoardPool>(FoundActor);

		BulletingBoardPool_ = Pool;
	}
	else {
		BulletingBoardPool_ = world->SpawnActor<AMS_BulletingBoardPool>(AMS_BulletingBoardPool::StaticClass());
	}



}
// Called every frame
void AMS_AIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    const int32 LowResourceThreshold = 50;
    int32 MaxResourcePerQuest = 15;
    // Iterate over the resource types and their amounts in the inventory
    for (const auto& ResourcePair : Inventory_->Resources_)
    {
        ResourceType Type = ResourcePair.Key;
        int32 Amount = ResourcePair.Value;

        // If the resource is below the threshold, create quests to gather more
        if (Amount < LowResourceThreshold)
        {
            // Calculate how many resources are needed to reach the threshold
            int32 NeededResources = LowResourceThreshold - Amount;

            // Generate multiple quests if needed
            while (NeededResources > 0)
            {
                // Determine the amount for the current quest
                int32 QuestAmount = FMath::Min(NeededResources, MaxResourcePerQuest);

                // Check if a quest with this type and amount already exists
                bool QuestExists = false;
                for (const FQuest& ActiveQuest : ActiveQuests_)
                {
                    if (ActiveQuest.Type == Type && ActiveQuest.Amount == QuestAmount)
                    {
                        QuestExists = true;
                        break; // Break if a duplicate quest is found
                    }
                }

                // If the quest doesn't already exist, create and assign it
                if (!QuestExists)
                {
                    FQuest NewQuest; // Create a new quest directly
                    NewQuest.Type = Type;
                    NewQuest.Amount = QuestAmount;

                    // Assign the quest to a bulletin board
                    bool QuestAssigned = false;
                    for (AMS_BulletingBoard* BulletinBoard : BulletingBoardPool_->BulletingBoards_)
                    {
                        if (BulletinBoard->Quests_.Num() < 5) // Example: limit of 5 quests per board
                        {
                            BulletinBoard->Quests_.Add(NewQuest); // Add directly
                            ActiveQuests_.Add(NewQuest); // Track the new active quest
                            UE_LOG(LogTemp, Log, TEXT("Assigned quest to gather %d of %s to bulletin board"),
                                NewQuest.Amount, *UEnum::GetValueAsString(Type));
                            QuestAssigned = true;
                            break; // Stop after assigning the quest to the first available board
                        }
                    }

                    if (!QuestAssigned)
                    {
                        // You don't need to clean up memory for NewQuest since it is not a pointer
                    }
                }

                // Decrease the amount of needed resources
                NeededResources -= QuestAmount;
            }
        }
    }
}


void AMS_AIManager::UpdateResources(ResourceType type, int32 amount) {
	Inventory_->SetResource(type, amount);
}

