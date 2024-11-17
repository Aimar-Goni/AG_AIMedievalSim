// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNodes/MS_FindNearestBulletingBoard.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Placeables/Buildings/MS_BulletingBoardPool.h"

EBTNodeResult::Type UMS_FindNearestBulletingBoard::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	// Get the AI controller
	if (AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner()))
	{
		if (AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn())) 
		{
			TArray<AMS_BulletingBoard*> Pool = Cast<AMS_BulletingBoardPool>(AICharacter->BulletingBoardPool_)->BulletingBoards_;

			if (Pool.Num() == 0)
			{
				return EBTNodeResult::Failed;
			}

			AActor* Closest = nullptr;
			float ClosestDistance = FLT_MAX;

			bool found = false;
			for (AMS_BulletingBoard* Workplace : Pool)
			{
				if (Workplace->Quests_.Num()>0) {

					float CurrentDistance = AICharacter->GetDistanceTo(Workplace);
					if (CurrentDistance < ClosestDistance)
					{
						ClosestDistance = CurrentDistance;
						Closest = Workplace;
						found = true;
					}
				}
			}

			if (Closest) {
				AICharacter->CreateMovementPath(Closest);
				OwnerComp.GetBlackboardComponent()->SetValueAsObject("Target", Closest);
			}
			else AIController->GetBlackboardComponent()->SetValueAsBool("Working", false);

			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}