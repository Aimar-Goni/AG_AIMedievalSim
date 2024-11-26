// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MS_FollowNodePath.generated.h"

/**
 * 
 */
UCLASS()
class AG_AIMEDIEVALSIM_API UMS_FollowNodePath : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UMS_FollowNodePath();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Pathfinding")
    FBlackboardKeySelector PathKey;

    UPROPERTY(EditAnywhere, Category = "Pathfinding")
    FBlackboardKeySelector CurrentNodeIndexKey; 
};