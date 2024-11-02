// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TaskNodes/MS_FindNearestWorkSite.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"

EBTNodeResult::Type UMS_FindNearestWorkSite::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get the AI controller and pawn
	auto* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner());
	auto* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
	if (!AICharacter) return EBTNodeResult::Failed;

	// Retrieve the pool of workplaces and previous target
	auto* WorkplacePool = Cast<AMS_WorkpPlacePool>(AICharacter->WorkPlacesPool_);
	if (!WorkplacePool || WorkplacePool->Workplaces_.IsEmpty()) return EBTNodeResult::Failed;

	auto* PreviousTarget = Cast<AMS_BaseWorkPlace>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("Target"));

	// Find the closest matching workplace
	AMS_BaseWorkPlace* ClosestWorkplace = nullptr;
	float ClosestDistance = FLT_MAX;

	for (auto* Workplace : WorkplacePool->Workplaces_)
	{
		if (Workplace->ResourceType_ == AICharacter->Quest_.Type && Workplace->ResourceAvaliable_ && Workplace != PreviousTarget)
		{
			const float CurrentDistance = AICharacter->GetDistanceTo(Workplace);
			if (CurrentDistance < ClosestDistance)
			{
				ClosestDistance = CurrentDistance;
				ClosestWorkplace = Workplace;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Closest: %f"), ClosestDistance);

	// Update the Blackboard if a workplace was found
	if (ClosestWorkplace)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsObject("Target", ClosestWorkplace);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}