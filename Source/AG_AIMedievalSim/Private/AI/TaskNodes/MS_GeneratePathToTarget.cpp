// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TaskNodes/MS_GeneratePathToTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AI/Characters/MS_AICharacter.h" 
#include "GameFramework/Actor.h"

UMS_GeneratePathToTarget::UMS_GeneratePathToTarget()
{
	NodeName = "Generate Path To Target";

	// Initialize key selector
	BlackboardKey_TargetActor.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_GeneratePathToTarget, BlackboardKey_TargetActor), AActor::StaticClass());
}

void UMS_GeneratePathToTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);
    UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		BlackboardKey_TargetActor.ResolveSelectedKey(*BBAsset);
	}
}


EBTNodeResult::Type UMS_GeneratePathToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
    AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(ControlledPawn);


	if (!Blackboard || !AIController || !AICharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task '%s': Invalid Blackboard, Controller, or AICharacter."), *GetNodeName());
		return EBTNodeResult::Failed;
	}

	// Check if the TargetActor key is set
	if (!BlackboardKey_TargetActor.IsSet() || BlackboardKey_TargetActor.SelectedKeyName == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task '%s': TargetActor Blackboard Key is not set in the task properties."), *GetNodeName());
		return EBTNodeResult::Failed;
	}


	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKey_TargetActor.SelectedKeyName));

	if (!IsValid(TargetActor)) // Use IsValid to check if the actor pointer is good
	{
		UE_LOG(LogTemp, Warning, TEXT("Task '%s': TargetActor in Blackboard key '%s' is null or invalid for %s."),
			*GetNodeName(), *BlackboardKey_TargetActor.SelectedKeyName.ToString(), *AICharacter->GetName());
		return EBTNodeResult::Failed;
	}


	AICharacter->Path_ = AICharacter->CreateMovementPath(TargetActor);

	// Check if a path was successfully generated
	if (AICharacter->Path_.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Task '%s': Successfully generated path with %d nodes for %s to target %s."),
			*GetNodeName(), AICharacter->Path_.Num(), *AICharacter->GetName(), *TargetActor->GetName());
		return EBTNodeResult::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Task '%s': AICharacter %s failed to generate a path to target %s."),
			*GetNodeName(), *AICharacter->GetName(), *TargetActor->GetName());
		return EBTNodeResult::Failed;
	}
}

FString UMS_GeneratePathToTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: Paths to '%s'"),
		*Super::GetStaticDescription(),
		*BlackboardKey_TargetActor.SelectedKeyName.ToString());
}