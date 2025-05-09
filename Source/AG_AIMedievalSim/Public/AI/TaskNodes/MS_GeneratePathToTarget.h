// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MS_GeneratePathToTarget.generated.h"


UCLASS()
class AG_AIMEDIEVALSIM_API UMS_GeneratePathToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMS_GeneratePathToTarget();

	/** Blackboard key for the Target Actor to path to. */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_TargetActor;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};