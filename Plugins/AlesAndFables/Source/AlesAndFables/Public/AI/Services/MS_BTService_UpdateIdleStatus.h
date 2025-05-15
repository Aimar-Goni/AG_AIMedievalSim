// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h" 
#include "MS_BTService_UpdateIdleStatus.generated.h"

/**
 * Updates the 'bIsIdle' blackboard key based on other states like 'bHasQuest', 'bIsSleeping', 'bIsHungry', etc.
 */
UCLASS()
class ALESANDFABLES_API UMS_BTService_UpdateIdleStatus : public UBTService_BlackboardBase // Changed base class
{
	GENERATED_BODY()

public:
	UMS_BTService_UpdateIdleStatus();

protected:
	/** Blackboard key selector for the boolean indicating if a quest is assigned. */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_HasQuest;

	/** Blackboard key selector for the boolean indicating if sleeping. */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_IsSleeping;

	/** Blackboard key selector for the boolean indicating if hungry. */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_IsHungry;

	/** Blackboard key selector for the boolean indicating if thirsty. */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_IsThirsty;


	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

};