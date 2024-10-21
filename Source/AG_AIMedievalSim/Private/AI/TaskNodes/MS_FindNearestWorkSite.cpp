// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "AI/TaskNodes/MS_FindNearestWorkSite.h"

EBTNodeResult::Type UMS_FindNearestWorkSite::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	// Get the AI controller
	if (AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner()))
	{
		if (AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn()))
		{
			TArray<AMS_BaseWorkPlace*> Pool = Cast<AMS_WorkpPlacePool>(AICharacter->WorkPlacesPool_)->Workplaces_;

			if (Pool.Num() == 0)
			{
				return EBTNodeResult::Failed;
			}

			AActor* Closest = NULL;
			float ClosestDistance = 999999999;

			for (AMS_BaseWorkPlace* Workplace : Pool)
			{
				
				if (!Workplace->IsPlaceOccupied()) {
					if (Workplace->ResourceAvaliable_ == true) {
						float CurrentDistance = AICharacter->GetDistanceTo(Workplace);
						if (CurrentDistance < ClosestDistance)
						{
							ClosestDistance = CurrentDistance;
							Closest = Workplace;
						}
					}
				}

				

				
			}

			OwnerComp.GetBlackboardComponent()->SetValueAsObject("Target", Closest);
			return EBTNodeResult::Succeeded;

		}
	}

	return EBTNodeResult::Failed;
}