// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MS_FindNearestBulletingBoard.generated.h"

/**
 * 
 */
UCLASS()
class AG_AIMEDIEVALSIM_API UMS_FindNearestBulletingBoard : public UBTTaskNode
{
	GENERATED_BODY()
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};