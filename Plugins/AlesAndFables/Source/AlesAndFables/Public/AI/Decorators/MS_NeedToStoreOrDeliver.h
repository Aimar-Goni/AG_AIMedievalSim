// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MS_NeedToStoreOrDeliver.generated.h"

/**
 * Checks if the AI has an assigned quest and has gathered enough resources (Inventory >= QuestAmount).
 * Can be configured to check for either storage (TargetDestination is null) or delivery (TargetDestination is set).
 */
UCLASS()
class ALESANDFABLES_API UMS_NeedToStoreOrDeliver : public UBTDecorator
{
	GENERATED_BODY()

public:
	UMS_NeedToStoreOrDeliver();

protected:
	UPROPERTY(EditAnywhere, Category = Condition)
	bool bCheckForDelivery = false; 

	/** Blackboard key selector for the boolean indicating if a quest is assigned. */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_HasQuest;

	/** Blackboard key selector for the Enum QuestType (ResourceType). */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_QuestType;

	/** Blackboard key selector for the Int QuestAmount. */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_QuestAmount;

	/** Blackboard key selector for the Object QuestTargetDestination. */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_QuestTargetDestination;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};