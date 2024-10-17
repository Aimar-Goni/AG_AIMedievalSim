// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Placeables/Buildings/MS_BulletingBoardPool.h"
#include "AI/TaskNodes/MS_FindNearestBulletingBoard.h"

EBTNodeResult::Type UMS_FindNearestBulletingBoard::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	// Get the AI controller
	if (AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner()))
	{
		if (AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn())) 
		{
			TArray<AActor*> Pool = Cast<AMS_BulletingBoardPool>(AICharacter->BulletingBoardPool_)->BulletingBoards_;
			AActor* Closest = Pool[0];
			float Distance = AICharacter->GetDistanceTo(Pool[0]);
			for (size_t i = 1; i < Pool.Num(); i++)
			{
				if (AICharacter->GetDistanceTo(Pool[i]) < Distance) {
					Distance = AICharacter->GetDistanceTo(Pool[i]);
					Closest = Pool[i];
					i++;
				}
			}
			OwnerComp.GetBlackboardComponent()->SetValueAsObject("Target", Closest);
			return EBTNodeResult::Succeeded;

		}
	}

	return EBTNodeResult::Failed;
}