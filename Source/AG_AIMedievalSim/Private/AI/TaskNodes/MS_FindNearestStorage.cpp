// Fill out your copyright notice in the Description page of Project Settings.



#include "AI/TaskNodes/MS_FindNearestStorage.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Movement/MS_PathfindingSubsystem.h"

UMS_FindNearestStorage::UMS_FindNearestStorage()
{
	NodeName = "Find Nearest Storage";
	BlackboardKey_TargetStorage.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FindNearestStorage, BlackboardKey_TargetStorage), AActor::StaticClass());
}

EBTNodeResult::Type UMS_FindNearestStorage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner());
	auto* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
    auto* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!AICharacter || !Blackboard) return EBTNodeResult::Failed;


	AMS_StorageBuildingPool* StoragePool = Cast<AMS_StorageBuildingPool>(AICharacter->StorageBuldingsPool_.Get());
	if (!StoragePool || StoragePool->StorageBuldings_.IsEmpty()) 
    {
         UE_LOG(LogTemp, Warning, TEXT("FindNearestStorage: Storage Pool invalid or empty for %s."), *AICharacter->GetName());
        return EBTNodeResult::Failed;
    }


	AMS_StorageBuilding* ClosestStorage = nullptr;
	float ClosestDistanceSq = FLT_MAX;

    UE_LOG(LogTemp, Verbose, TEXT("FindNearestStorage: %s searching for storage."), *AICharacter->GetName());

	for (TWeakObjectPtr<AMS_StorageBuilding> StoragePtr : StoragePool->StorageBuldings_)
	{
		if (StoragePtr.IsValid() && StoragePtr->placeActive_) 
		{
            AMS_StorageBuilding* Storage = StoragePtr.Get();
			float CurrentDistanceSq = FVector::DistSquared(AICharacter->GetActorLocation(), Storage->GetActorLocation());
			if (CurrentDistanceSq < ClosestDistanceSq)
			{
				ClosestDistanceSq = CurrentDistanceSq;
				ClosestStorage = Storage;
			}
		}
	}

	if (ClosestStorage)
	{
        UE_LOG(LogTemp, Log, TEXT("FindNearestStorage: %s found storage %s (DistSq: %.0f)."),
            *AICharacter->GetName(), *ClosestStorage->GetName(), ClosestDistanceSq);
		
        Blackboard->SetValueAsObject(BlackboardKey_TargetStorage.SelectedKeyName, ClosestStorage);
        Blackboard->SetValueAsObject(FName("Target"), ClosestStorage);

        AICharacter->CreateMovementPath(ClosestStorage);
         if(AICharacter->Path_.Num() > 0)
        {
		    return EBTNodeResult::Succeeded;
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("FindNearestStorage: Found storage %s for %s, but failed to generate path."), *ClosestStorage->GetName(), *AICharacter->GetName());
             Blackboard->ClearValue(BlackboardKey_TargetStorage.SelectedKeyName);
             Blackboard->ClearValue(FName("Target"));
             return EBTNodeResult::Failed;
        }
	}
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FindNearestStorage: %s could not find any active storage."), *AICharacter->GetName());
    }

	return EBTNodeResult::Failed;
}