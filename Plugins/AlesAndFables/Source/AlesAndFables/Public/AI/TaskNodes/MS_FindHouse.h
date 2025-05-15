// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MS_FindHouse.generated.h"

UCLASS()
class ALESANDFABLES_API UMS_FindHouse : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UMS_FindHouse();

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_Target; // Output: Target Actor (the house)

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};