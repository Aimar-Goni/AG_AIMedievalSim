// Fill out your copyright notice in the Description page of Project Settings.




#include "AI/TaskNodes/MS_StoreItems.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"

EBTNodeResult::Type UMS_StoreItems::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	// Get the AI controller
	if (AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner()))
	{
		if (AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn()))
		{


			// Update BB states
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("DoingTask", false);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("GettingTask", true);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("Working", true);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("StoringItems", false);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}