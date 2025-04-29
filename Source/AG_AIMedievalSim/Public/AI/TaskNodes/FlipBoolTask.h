// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "FlipBoolTask.generated.h"


UCLASS()
class AG_AIMEDIEVALSIM_API UFlipBoolTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UFlipBoolTask();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BoolKey;
};