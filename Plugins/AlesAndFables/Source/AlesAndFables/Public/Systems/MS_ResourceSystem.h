// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MS_InventoryComponent.h"
#include "MS_ResourceSystem.generated.h"

USTRUCT(BlueprintType)
struct ALESANDFABLES_API FQuest
{
	GENERATED_BODY()

	// Unique identifier for this specific quest instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGuid QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Resources")
	ResourceType Type = ResourceType::ERROR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Resources")
	int32 Amount = 0;

	// Monetary reward for completing the quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
	int32 Reward = 0;

	// Optional: Target destination for delivery quests (like construction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Targeting")
	TWeakObjectPtr<AActor> TargetDestination = nullptr;

	FQuest()
		: QuestID(FGuid::NewGuid()), // Assign a new ID by default
		Type(ResourceType::ERROR),
		Amount(0),
		Reward(0),
		TargetDestination(nullptr)
	{}

	FQuest(ResourceType InType, int32 InAmount, int32 InReward = 0, AActor* InTarget = nullptr)
		: QuestID(FGuid::NewGuid()), // Assign a new ID
		Type(InType),
		Amount(InAmount),
		Reward(InReward),
		TargetDestination(InTarget)
	{}

	bool operator<(const FQuest& Other) const
	{
		return QuestID < Other.QuestID;
	}
};


UCLASS()
class ALESANDFABLES_API AMS_ResourceSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_ResourceSystem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


};
