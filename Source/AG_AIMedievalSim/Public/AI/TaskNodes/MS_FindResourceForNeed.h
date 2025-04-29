// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Systems/MS_InventoryComponent.h" 
#include "MS_FindResourceForNeed.generated.h"


UCLASS()
class AG_AIMEDIEVALSIM_API UMS_FindResourceForNeed : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMS_FindResourceForNeed();


	UPROPERTY(EditAnywhere, Category=Task)
	ResourceType NeededResourceType = ResourceType::ERROR;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_Target;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};