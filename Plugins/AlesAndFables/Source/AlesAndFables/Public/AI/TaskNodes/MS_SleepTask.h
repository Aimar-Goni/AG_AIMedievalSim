// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MS_SleepTask.generated.h"

class UMS_TimeSubsystem;

// Memory Structure
USTRUCT()
struct FMSSleepTaskMemory
{
	GENERATED_BODY()

	UPROPERTY() // Keep reference valid
	TObjectPtr<UMS_TimeSubsystem> TimeSubsystemCache = nullptr;
};

UCLASS()
class ALESANDFABLES_API UMS_SleepTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMS_SleepTask();
	
	UPROPERTY(EditAnywhere, Category=Blackboard)
	FBlackboardKeySelector BlackboardKey_IsSleeping;
	
	UPROPERTY(EditAnywhere, Category=Task)
	float EnergyRestoredPerSecond = 1.0f;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

	void Cleanup(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
};