// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TaskNodes/MS_ExecuteWork.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "Systems/MS_InventoryComponent.h"
#include "AI/Characters/MS_AICharacter.h"


EBTNodeResult::Type UMS_ExecuteWork::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	// Get the AI controller
	if (AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner()))
	{
		if (AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn()))
		{

			AMS_BaseWorkPlace* Workplace = Cast<AMS_BaseWorkPlace>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("Target"));
			
			ResourceType ResourceTypeNeeded = AICharacter->AssignedQuest.Type;
			int32 ResourceAmountNeeded = AICharacter->AssignedQuest.Amount;
			auto a  = Cast<UInventoryComponent>(AICharacter->Inventory_);

			// Get resources
			if (a->GetResourceAmount(ResourceTypeNeeded) < ResourceAmountNeeded)
			{
				return EBTNodeResult::Failed;
			}
	
			//Modify BB states
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("DoingTask", false);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("GettingTask", false);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("StoringItems", true);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}