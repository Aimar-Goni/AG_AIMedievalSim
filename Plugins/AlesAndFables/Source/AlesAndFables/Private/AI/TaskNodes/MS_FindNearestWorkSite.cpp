// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TaskNodes/MS_FindNearestWorkSite.h" 
#include "AI/Characters/MS_AICharacterController.h" 
#include "AI/Characters/MS_AICharacter.h" 
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h" 
#include "BehaviorTree/BlackboardComponent.h"      
#include "Systems/MS_InventoryComponent.h"  

EBTNodeResult::Type UMS_FindNearestWorkSite::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner());
	auto* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
    auto* Blackboard = OwnerComp.GetBlackboardComponent(); // Get Blackboard

	if (!AICharacter || !Blackboard) return EBTNodeResult::Failed;


    FName QuestTypeKeyName = FName("QuestType"); 
    ResourceType ResourceTypeNeeded = static_cast<ResourceType>(Blackboard->GetValueAsEnum(QuestTypeKeyName));


    if (ResourceTypeNeeded == ResourceType::ERROR)
    {
        UE_LOG(LogTemp, Warning, TEXT("FindNearestWorkSite: QuestType in Blackboard is ERROR for %s."), *AICharacter->GetName());
        return EBTNodeResult::Failed; // No valid quest type
    }
 


	// Retrieve the pool of workplaces
	auto* WorkplacePool = Cast<AMS_WorkpPlacePool>(AICharacter->WorkPlacesPool_.Get()); 
	if (!WorkplacePool || WorkplacePool->ActiveWorkplaces_.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("FindNearestWorkSite: WorkplacePool invalid or empty for %s."), *AICharacter->GetName());
        return EBTNodeResult::Failed;
    }
	
	auto* PreviousTarget = Cast<AMS_BaseWorkPlace>(Blackboard->GetValueAsObject(FName("Target"))); 
	
	AMS_BaseWorkPlace* ClosestWorkplace = nullptr;
	float ClosestDistanceSq = FLT_MAX; 

    UE_LOG(LogTemp, Verbose, TEXT("FindNearestWorkSite: %s searching for %s."), *AICharacter->GetName(), *UEnum::GetValueAsString(ResourceTypeNeeded));

	for (TWeakObjectPtr<AMS_BaseWorkPlace> WorkplacePtr : WorkplacePool->ActiveWorkplaces_)
	{
		if (WorkplacePtr.IsValid()) 
        {
            AMS_BaseWorkPlace* Workplace = WorkplacePtr.Get();
            if (Workplace->ResourceType_ == ResourceTypeNeeded && Workplace->ResourceAvaliable_ && Workplace != PreviousTarget)
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

	// Update the Blackboard if a workplace was found
	if (ClosestWorkplace)
	{
        UE_LOG(LogTemp, Log, TEXT("FindNearestWorkSite: %s found %s at %s (DistSq: %.0f)."),
            *AICharacter->GetName(), *UEnum::GetValueAsString(ResourceTypeNeeded), *ClosestWorkplace->GetName(), ClosestDistanceSq);

		AICharacter->CreateMovementPath(ClosestWorkplace); // Generate path
        if(AICharacter->Path_.Num() > 0) // Check if path was successful
        {
            Blackboard->SetValueAsObject(FName("Target"), ClosestWorkplace); 
		    return EBTNodeResult::Succeeded;
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("FindNearestWorkSite: Found workplace %s for %s, but failed to generate path."), *ClosestWorkplace->GetName(), *AICharacter->GetName());
             return EBTNodeResult::Failed; // Pathfinding failed
        }
	}
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FindNearestWorkSite: %s could not find any available workplace for %s."), *AICharacter->GetName(), *UEnum::GetValueAsString(ResourceTypeNeeded));
    }

	return EBTNodeResult::Failed;
}