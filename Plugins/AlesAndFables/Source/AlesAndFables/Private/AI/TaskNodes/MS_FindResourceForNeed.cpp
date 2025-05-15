// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TaskNodes/MS_FindResourceForNeed.h" 
#include "AI/Characters/MS_AICharacterController.h" 
#include "AI/Characters/MS_AICharacter.h" 
#include "Placeables/Interactables/MS_WorkpPlacePool.h" 
#include "Placeables/Interactables/MS_BaseWorkPlace.h" 
#include "BehaviorTree/BlackboardComponent.h"

UMS_FindResourceForNeed::UMS_FindResourceForNeed()
{
    NodeName = "Find Resource For Need";
    BlackboardKey_Target.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FindResourceForNeed, BlackboardKey_Target), AActor::StaticClass());
}

EBTNodeResult::Type UMS_FindResourceForNeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner());
	auto* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
    auto* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AICharacter || !Blackboard) return EBTNodeResult::Failed;

    if (NeededResourceType == ResourceType::ERROR)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindResourceForNeed: Task configured with ERROR ResourceType for %s."), *AICharacter->GetName());
        return EBTNodeResult::Failed;
    }

	auto* WorkplacePool = Cast<AMS_WorkpPlacePool>(AICharacter->WorkPlacesPool_.Get());
	if (!WorkplacePool || WorkplacePool->ActiveWorkplaces_.IsEmpty())
    {
         UE_LOG(LogTemp, Warning, TEXT("FindResourceForNeed: WorkplacePool invalid or empty for %s."), *AICharacter->GetName());
        return EBTNodeResult::Failed;
    }

	AMS_BaseWorkPlace* ClosestWorkplace = nullptr;
	float ClosestDistanceSq = FLT_MAX;

     UE_LOG(LogTemp, Verbose, TEXT("FindResourceForNeed: %s searching for need: %s."), *AICharacter->GetName(), *UEnum::GetValueAsString(NeededResourceType));

	for (TWeakObjectPtr<AMS_BaseWorkPlace> WorkplacePtr : WorkplacePool->ActiveWorkplaces_)
	{
		if (WorkplacePtr.IsValid())
        {
            AMS_BaseWorkPlace* Workplace = WorkplacePtr.Get();
            // Check type and availability
            if (Workplace->ResourceType_ == NeededResourceType && Workplace->ResourceAvaliable_)
            {
                const float CurrentDistanceSq = FVector::DistSquared(AICharacter->GetActorLocation(), Workplace->GetActorLocation());
                if (CurrentDistanceSq < ClosestDistanceSq)
                {
                    ClosestDistanceSq = CurrentDistanceSq;
                    ClosestWorkplace = Workplace;
                }
            }
        }
	}

	if (ClosestWorkplace)
	{
         UE_LOG(LogTemp, Log, TEXT("FindResourceForNeed: %s found %s at %s (DistSq: %.0f)."),
            *AICharacter->GetName(), *UEnum::GetValueAsString(NeededResourceType), *ClosestWorkplace->GetName(), ClosestDistanceSq);

        AICharacter->CreateMovementPath(ClosestWorkplace);
         if(AICharacter->Path_.Num() > 0)
        {
            Blackboard->SetValueAsObject(BlackboardKey_Target.SelectedKeyName, ClosestWorkplace); // Set specified key
		    return EBTNodeResult::Succeeded;
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("FindResourceForNeed: Found workplace %s for %s, but failed to generate path."), *ClosestWorkplace->GetName(), *AICharacter->GetName());
             return EBTNodeResult::Failed;
        }
	}
     else
    {
        UE_LOG(LogTemp, Warning, TEXT("FindResourceForNeed: %s could not find any available workplace for need: %s."), *AICharacter->GetName(), *UEnum::GetValueAsString(NeededResourceType));
    }

	return EBTNodeResult::Failed;
}