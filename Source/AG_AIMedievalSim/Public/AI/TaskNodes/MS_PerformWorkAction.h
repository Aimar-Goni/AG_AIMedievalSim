// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Systems/MS_InventoryComponent.h" 
#include "MS_PerformWorkAction.generated.h"

class UAnimMontage;
class AMS_BaseWorkPlace; 
class AMS_WheatField;  

USTRUCT()
struct FMSPerformWorkActionMemory
{
	GENERATED_BODY()

	float StartTime = -1.0f;
	TWeakObjectPtr<AMS_BaseWorkPlace> WorkplaceActor; // Can be BaseWorkPlace or derived like WheatField
	bool bAnimationStarted = false;
    UPROPERTY()
    TObjectPtr<UAnimMontage> ActiveWorkMontage = nullptr;
};


UCLASS()
class AG_AIMEDIEVALSIM_API UMS_PerformWorkAction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMS_PerformWorkAction();

	UPROPERTY(EditAnywhere, Category = "Task")
	float WorkDuration = 3.0f;

	//  Blackboard Keys 
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BlackboardKey_WorkplaceTarget; // The Workplace, WheatField, etc.

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BlackboardKey_IsAtTargetWorkplace; // Proximity flag

    UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BlackboardKey_QuestType; // To determine action type for fields

    UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BlackboardKey_QuestAmount; // For specific actions like watering amount

	//  Animation Mapping 
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Work Montages By Resource/Action"))
	TMap<ResourceType, TObjectPtr<UAnimMontage>> WorkMontages;

    // Specific montages for field actions if ResourceType map isn't sufficient
    UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> PlantingMontage;
    UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> WateringMontage;
    UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> HarvestingMontage;

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (DisplayName = "Finish Montages By Resource/Action"))
	TMap<ResourceType, TObjectPtr<UAnimMontage>> FinishWorkMontages;

    UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> DefaultWorkMontage;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
    virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;

	void CleanupTaskState(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
};