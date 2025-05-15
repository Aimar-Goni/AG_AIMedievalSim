// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TaskNodes/MS_ConsumeFromTarget.h" 
#include "AI/Characters/MS_AICharacterController.h" 
#include "AI/Characters/MS_AICharacter.h" 
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UMS_ConsumeFromTarget::UMS_ConsumeFromTarget()
{
    NodeName = "Consume From Target";
    BlackboardKey_TargetWorkplace.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_ConsumeFromTarget, BlackboardKey_TargetWorkplace), AActor::StaticClass());
}

EBTNodeResult::Type UMS_ConsumeFromTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner());
	auto* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
    auto* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AICharacter || !Blackboard) return EBTNodeResult::Failed;

    AMS_BaseWorkPlace* Workplace = Cast<AMS_BaseWorkPlace>(Blackboard->GetValueAsObject(BlackboardKey_TargetWorkplace.SelectedKeyName));

    if (!Workplace)
    {
        UE_LOG(LogTemp, Warning, TEXT("ConsumeFromTarget: Target Workplace is null for %s."), *AICharacter->GetName());
        return EBTNodeResult::Failed;
    }

    if (!Workplace->ResourceAvaliable_)
    {
        UE_LOG(LogTemp, Warning, TEXT("ConsumeFromTarget: Target Workplace %s has no available resource for %s."), *Workplace->GetName(), *AICharacter->GetName());
        return EBTNodeResult::Failed;
    }

    UE_LOG(LogTemp, Log, TEXT("ConsumeFromTarget: %s consuming from %s."), *AICharacter->GetName(), *Workplace->GetName());
    
    FResource ConsumedResource = Workplace->TakeResources();
    
    AICharacter->ConsumeResourceDirectly(ConsumedResource.Type, ConsumedResource.Amount);
    
    AICharacter->CheckIfHungry();
    
    AMS_WorkpPlacePool* WorkplacePool = Cast<AMS_WorkpPlacePool>(UGameplayStatics::GetActorOfClass(GetWorld(), AMS_WorkpPlacePool::StaticClass()));
    if (WorkplacePool)
    {
         WorkplacePool->RemoveWorkplaceAndFreeNode(Workplace);
    }
    else UE_LOG(LogTemp, Warning, TEXT("ConsumeFromTarget: Could not find Workplace Pool manager to remove node!"));

    // Clear the target from blackboard as the action is complete
    Blackboard->ClearValue(BlackboardKey_TargetWorkplace.SelectedKeyName);

    return EBTNodeResult::Succeeded;
}