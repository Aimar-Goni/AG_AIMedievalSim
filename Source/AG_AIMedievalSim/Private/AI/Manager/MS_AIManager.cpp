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


    BulletingBoardPool_->OnBulletingBoardPoolInitialized.AddDynamic(this, &AMS_AIManager::OnBulletingBoardPoolReady);
    


}

void AMS_AIManager::OnBulletingBoardPoolReady()
{
    for (AMS_BulletingBoard* BulletinBoard : BulletingBoardPool_->BulletingBoards_)
    {
        BulletinBoard->OnQuestObtained.AddDynamic(this, &AMS_AIManager::RemoveQuest);
    }
}

// Called every frame
void AMS_AIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    const int32 LowResourceThreshold = 50;
    int32 MaxResourcePerQuest = 15;
    if (SendingQuests)
    {
        for (const auto& ResourcePair : Inventory_->Resources_)
        {
            ResourceType Type = ResourcePair.Key;
            int32 Amount = ResourcePair.Value;

            if (Amount < LowResourceThreshold)
            {
                int32 NeededResources = LowResourceThreshold - Amount;

                while (NeededResources > 0)
                {
                    int32 QuestAmount = FMath::Min(NeededResources, MaxResourcePerQuest);

                    bool QuestExists = false;
                    for (const FQuest& ActiveQuest : ActiveQuests_)
                    {
                        if (ActiveQuest.Type == Type && ActiveQuest.Amount == QuestAmount)
                        {
                            QuestExists = true;
                            break; 
                        }
                    }

                    if (!QuestExists)
                    {
                        FQuest NewQuest;
                        NewQuest.Type = Type;
                        NewQuest.Amount = QuestAmount;

                        BulletingBoardPool_->BulletingBoards_[FMath::RandRange(0, BulletingBoardPool_->BulletingBoards_.Num()-1)]->AddQuest(NewQuest);
                        ActiveQuests_.Add(NewQuest);            
                    
                    }
                    NeededResources -= QuestAmount;
                }
            }
        }
    }
}


void AMS_AIManager::UpdateResources(ResourceType type, int32 amount) {
	Inventory_->SetResource(type, amount);
}

void AMS_AIManager::RemoveQuest(FQuest Quest) {
    int i = 0;
    for (FQuest ActiveQuest : ActiveQuests_)
    {
        if (ActiveQuest.Type == Quest.Type && ActiveQuest.Amount == Quest.Amount) {
            ActiveQuests_.RemoveAt(i);
            break;
        }
        i++;
    }
}
