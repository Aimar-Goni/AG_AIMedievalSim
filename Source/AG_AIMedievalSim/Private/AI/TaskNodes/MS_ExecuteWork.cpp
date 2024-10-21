// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"
#include "AI/TaskNodes/MS_ExecuteWork.h"

EBTNodeResult::Type UMS_ExecuteWork::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	// Get the AI controller
	if (AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner()))
	{
		if (AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn()))
		{
			switch (AICharacter->Quest_.Type)
			{
			case ResourceType::BERRIES:
				if (AICharacter->Inventory_.Berries_ < AICharacter->Quest_.Amount) {
					return EBTNodeResult::Failed;
				}
				break;
			case ResourceType::WOOD:
				if (AICharacter->Inventory_.Wood_ < AICharacter->Quest_.Amount) {
					return EBTNodeResult::Failed;
				}
				break;
			default:
				break;
			}
	
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("DoingTask", false);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("GettingTask", false);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("StoringItems", true);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}