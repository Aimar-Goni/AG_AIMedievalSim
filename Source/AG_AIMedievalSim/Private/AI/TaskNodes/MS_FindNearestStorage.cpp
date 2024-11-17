// Fill out your copyright notice in the Description page of Project Settings.



#include "AI/TaskNodes/MS_FindNearestStorage.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Movement/MS_PathfindingSubsyste.h"


EBTNodeResult::Type UMS_FindNearestStorage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	// Get the AI controller and pawn
	auto* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner());
	auto* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
	if (!AICharacter) return EBTNodeResult::Failed;

	// Retrieve the pool of workplaces and previous target
	AMS_StorageBuildingPool* StoragePool = Cast<AMS_StorageBuildingPool>(AICharacter->StorageBuldingsPool_);
	if (!StoragePool) return EBTNodeResult::Failed;

	auto* PreviousTarget = Cast<AMS_StorageBuilding>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("Target"));

	// Find the closest matching workplace
	AMS_StorageBuilding* Closest = nullptr;
	float ClosestDistance = FLT_MAX;


	for (AMS_StorageBuilding* Storage : StoragePool->StorageBuldings_)
	{
		float CurrentDistance = AICharacter->GetDistanceTo(Storage);
		if (CurrentDistance < ClosestDistance)
		{
			ClosestDistance = CurrentDistance;
			Closest = Storage;
		}
	}

	if (Closest)
	{
		AICharacter->CreateMovementPath(Closest);
		OwnerComp.GetBlackboardComponent()->SetValueAsObject("Target", Closest);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;


}