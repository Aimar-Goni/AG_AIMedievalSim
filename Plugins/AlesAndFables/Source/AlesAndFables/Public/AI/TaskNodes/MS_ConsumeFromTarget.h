// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MS_ConsumeFromTarget.generated.h"


UCLASS()
class ALESANDFABLES_API UMS_ConsumeFromTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMS_ConsumeFromTarget();
	
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_TargetWorkplace;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};