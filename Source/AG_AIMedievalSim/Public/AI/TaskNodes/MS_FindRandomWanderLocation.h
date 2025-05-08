// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MS_FindRandomWanderLocation.generated.h"


UCLASS()
class AG_AIMEDIEVALSIM_API UMS_FindRandomWanderLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMS_FindRandomWanderLocation();

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_TargetActor;


	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_TargetLocation;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

private:
	// Cache the subsystem pointer for performance
	mutable TWeakObjectPtr<class UMS_PathfindingSubsystem> CachedPathSubsystem;
};