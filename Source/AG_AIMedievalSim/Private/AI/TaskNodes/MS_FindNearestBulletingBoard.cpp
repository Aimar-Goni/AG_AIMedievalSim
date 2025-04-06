// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNodes/MS_FindNearestBulletingBoard.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Placeables/Buildings/MS_BulletingBoardPool.h"

EBTNodeResult::Type UMS_FindNearestBulletingBoard::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

  //// ESTOY COMPROBANDO QUE PONIENDO LOS POOLS A VER SI FUNCIONA
// ISSUE: THE GAME IS LOADING THE NAV before THE BULLETING POOL SO IT  sends the done broadcast before it linkes it to the pools so it DOESNT GET THE UPDATES
	
	// Get the AI controller
	if (AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner()))
	{
		if (AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn())) 
		{
			TArray< TWeakObjectPtr<AMS_BulletingBoard>> Pool = Cast<AMS_BulletingBoardPool>(AICharacter->BulletingBoardPool_)->ActiveBulletingBoards_;

			if (Pool.Num() == 0)
			{
				return EBTNodeResult::Failed;
			}

			AActor* Closest = nullptr;
			float ClosestDistance = FLT_MAX;

			bool found = false;
			// Check the pool and compare workplaces until find the closest one
			for (TWeakObjectPtr<AMS_BulletingBoard> Workplace : Pool)
			{
				if (Workplace->Quests_.Num()>0) {

					float CurrentDistance = AICharacter->GetDistanceTo(Workplace.Get());
					if (CurrentDistance < ClosestDistance)
					{
						ClosestDistance = CurrentDistance;
						Closest = Workplace.Get();
						found = true;
					}
				}
			}

			//Modify BB variables
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