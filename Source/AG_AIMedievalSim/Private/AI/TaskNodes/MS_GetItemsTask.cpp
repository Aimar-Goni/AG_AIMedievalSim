// Fill out your copyright notice in the Description page of Project Settings.




#include "AI/TaskNodes/MS_GetItemsTask.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"

EBTNodeResult::Type UMS_GetItemsTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	// Get the AI controller
	if (AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner()))
	{
		if (AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn()))
		{



			// Update BB states
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("GettingWater", false);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("GettingFood", false);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("Working", true);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("DoingTask", false);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("GettingTask", true);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("StoringItems", false);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}