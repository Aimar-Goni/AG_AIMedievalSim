// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/MS_InventoryComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MS_PerformWorkAction.generated.h"

class UAnimMontage;
class AMS_BaseWorkPlace;

// Structure to hold memory for this node
USTRUCT()
struct FMSPerformWorkActionMemory
{
	GENERATED_BODY()

	/** Time when the task started */
	float StartTime = -1.0f;

	/** Cached pointer to the workplace actor */
	TWeakObjectPtr<AActor> WorkplaceActor;

	/** Whether the animation started playing */
	bool bAnimationStarted = false;

    /** Keep track of which montage actually started playing for cleanup */
    UPROPERTY() // UPROPERTY prevents GC
    TObjectPtr<UAnimMontage> ActiveWorkMontage = nullptr;
};


UCLASS()
class AG_AIMEDIEVALSIM_API UMS_PerformWorkAction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMS_PerformWorkAction();

	/** Duration of the work action in seconds. */
	UPROPERTY(EditAnywhere, Category = "Task")
	float WorkDuration = 3.0f;

	/** Blackboard key selector for the Target Workplace actor. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BlackboardKey_WorkplaceTarget;

	/** Blackboard key selector for the boolean indicating the AI is at the location. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BlackboardKey_IsAtLocation;

	// --- Animation Mapping ---

	/** Maps ResourceType (from AI's Quest) to the Animation Montage to play while working. */
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Work Montages By Resource"))
	TMap<ResourceType, TObjectPtr<UAnimMontage>> WorkMontages;

    /** Default Montage if no specific one is found for the ResourceType */
    UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> DefaultWorkMontage;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
    virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;

	/** Helper function to stop montage and clean up blackboard key */
	void CleanupTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
};