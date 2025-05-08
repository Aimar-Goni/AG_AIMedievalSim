// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNodes/MS_FollowNodePath.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Movement/MS_PathfindingSubsystem.h"

UMS_FollowNodePath::UMS_FollowNodePath()
{
    NodeName = "Follow Path";
    bNotifyTick = true; // Allows TickTask to be called
}

EBTNodeResult::Type UMS_FollowNodePath::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    // Check if a path is avaliable
    AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn());
    if (!AICharacter || AICharacter->Path_.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No path found on AICharacter."));
        return EBTNodeResult::Failed;
    }

    AICharacter->CurrentNodeIndex = 0;

    // Begin movement
    MoveToNextNode(OwnerComp, AIController, AICharacter);

    return EBTNodeResult::InProgress; // Task is ongoing
}

void UMS_FollowNodePath::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn());
    if (!AIController || !AICharacter)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Check if the AI has reached the current target location
    if (FVector::Dist(AICharacter->GetActorLocation(), AICharacter->CurrentTargetLocation) < 100.0f) 
    {
        MoveToNextNode(OwnerComp, AIController, AICharacter);
    }
}

void UMS_FollowNodePath::MoveToNextNode(UBehaviorTreeComponent& OwnerComp, AAIController* AIController, AMS_AICharacter* AICharacter)
{
    if (AICharacter->CurrentNodeIndex >= AICharacter->Path_.Num())
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded); // Task is complete
        return;
    }
    if (AICharacter->CurrentNodeIndex == -1)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    FIntPoint CurrentNode = AICharacter->Path_[AICharacter->CurrentNodeIndex];
    
    UMS_PathfindingSubsystem* PathfindingSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
    if (PathfindingSubsystem)
    {

        AICharacter->CurrentTargetLocation = PathfindingSubsystem->FindNodeByGridPosition(CurrentNode)->Position;
        AIController->MoveToLocation(AICharacter->CurrentTargetLocation);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Folow Node: No valid PathfindingSubsystem found!"));
    }

    AICharacter->CurrentNodeIndex++;
}