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
			int i = 0;
			TArray<AActor*> Pool = Cast<AMS_BulletingBoardPool>(AICharacter->BulletingBoardPool_)->BulletingBoards_;
			AActor* Closest = Pool[i];
			float Distance = AICharacter->GetDistanceTo(Pool[i]);
			for (AActor* actor : Cast<AMS_BulletingBoardPool>(AICharacter->BulletingBoardPool_)->BulletingBoards_) 
			{
				if (AICharacter->GetDistanceTo(actor) < Distance) {
					Distance = AICharacter->GetDistanceTo(actor);
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