// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNodes/MS_FollowNodePath.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Movement/MS_PathfindingSubsyste.h"

UMS_FollowNodePath::UMS_FollowNodePath()
{
    NodeName = "Follow Path";
}

EBTNodeResult::Type UMS_FollowNodePath::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }


    auto* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner());
    auto* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
    if (!AICharacter) return EBTNodeResult::Failed;


    AICharacter->Path_;

    if (0 == AICharacter->Path_.Num())
    {
        return EBTNodeResult::Succeeded;
    }
    auto* a = AICharacter->Path_[AICharacter->Path_.Num()-1];


    FVector TargetLocation = a->Position;
    AIController->MoveToLocation(TargetLocation);
    AICharacter->Path_.RemoveAt(AICharacter->Path_.Num() - 1);

    return EBTNodeResult::InProgress; 
}

void UMS_FollowNodePath::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AAIController* AIController = OwnerComp.GetAIOwner();
    auto* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
    if (!AIController || !AIController->GetPawn())
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }


    if (AICharacter->Path_.Num() == 0)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Get the current node index
    int32 CurrentNodeIndex = BlackboardComp->GetValueAsInt(CurrentNodeIndexKey.SelectedKeyName);

   
        // Increment the node index
        CurrentNodeIndex++;

        // Check if we've reached the end of the path
        if (CurrentNodeIndex >= AICharacter->Path_.Num())
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            return;
        }

        auto* a = AICharacter->Path_[AICharacter->Path_.Num() - 1];


        FVector TargetLocation = a->Position;
        AIController->MoveToLocation(TargetLocation);
        AICharacter->Path_.RemoveAt(AICharacter->Path_.Num() - 1);
    
}