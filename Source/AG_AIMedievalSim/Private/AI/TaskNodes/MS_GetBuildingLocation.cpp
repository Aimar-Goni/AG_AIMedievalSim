// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNodes/MS_GetBuildingLocation.h"
#include "Placeables/Buildings/MS_ConstructionSite.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"

EBTNodeResult::Type UMS_GetBuildingLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner());
	auto* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
    auto* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AICharacter || !Blackboard) return EBTNodeResult::Failed;
	auto* target= Blackboard->GetValueAsObject(FName("Target"));
	auto* Site = Cast<AActor>(target); 

	
	if (!Site) return EBTNodeResult::Failed;

	AICharacter->CreateMovementPath(Site);

	if(AICharacter->Path_.Num() > 0)
	{
		return EBTNodeResult::Succeeded;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetBuildingLocation: Found location %s for %s, but failed to generate path."), *Site->GetName(), *AICharacter->GetName());
		Blackboard->ClearValue(FName("Target"));
		return EBTNodeResult::Failed;
	}

}
