// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNodes/MS_ConsumeDirectlyTask.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "Systems/MS_InventoryComponent.h"
#include "AI/Characters/MS_AICharacter.h"


EBTNodeResult::Type UMS_ConsumeDirectlyTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	// Get the AI controller
	if (AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner()))
	{
		if (AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn()))
		{

			AMS_BaseWorkPlace* Workplace = Cast<AMS_BaseWorkPlace>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("Target"));

			ResourceType ResourceTypeNeeded = AICharacter->Quest_.Type;
			int32 ResourceAmountNeeded = AICharacter->Quest_.Amount;
			auto a = Cast<UInventoryComponent>(AICharacter->Inventory_);

			// Consume resources
			AICharacter->ConsumeResourceDirectly(Workplace->ResourceType_, Workplace->ResourceAmount_);

			// Update hunger and thirst
			AICharacter->CheckIfHungry();
			

			//Modify BB states
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("Ignoring", false);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("DoingTask", false);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("GettingTask", true);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("Working", true);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool("StoringItems", false);
			
			// Reset Quest
			AICharacter->Quest_ = FQuest(ResourceType::ERROR,0);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
