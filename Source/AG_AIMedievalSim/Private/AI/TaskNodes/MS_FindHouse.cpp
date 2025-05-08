// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TaskNodes/MS_FindHouse.h" // Adjust path
#include "AI/Characters/MS_AICharacter.h"           // Adjust path
#include "Placeables/Buildings/MS_House.h"              // Adjust path
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UMS_FindHouse::UMS_FindHouse()
{
    NodeName = "Find And Target My House";
    BlackboardKey_Target.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FindHouse, BlackboardKey_Target), AActor::StaticClass());
}

EBTNodeResult::Type UMS_FindHouse::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    AMS_AICharacter* AIChar = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

    if (!AIChar || !Blackboard) return EBTNodeResult::Failed;
    
    AMS_House* MyHouse = AIChar->GetAssignedHouse();
    if (MyHouse && MyHouse->HasSpace()) // Check if house exists AND has space
    {
        Blackboard->SetValueAsObject(BlackboardKey_Target.SelectedKeyName, MyHouse);
        Blackboard->SetValueAsBool("bHasFoundHouse", true);
        AIChar->CreateMovementPath(MyHouse);

        UE_LOG(LogTemp, Log, TEXT("FindAndTargetHouse: %s targeting house %s."), *AIChar->GetName(), *MyHouse->GetName());
        return EBTNodeResult::Succeeded;
    }
    else if(MyHouse)
    {
         UE_LOG(LogTemp, Warning, TEXT("FindAndTargetHouse: %s has house %s, but it's full."), *AIChar->GetName(), *MyHouse->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FindAndTargetHouse: %s has no assigned house."), *AIChar->GetName());
    }
    Blackboard->SetValueAsBool("bHasFoundHouse", false);

    Blackboard->ClearValue(BlackboardKey_Target.SelectedKeyName); // Clear target if no valid house
    return EBTNodeResult::Failed;
}

FString UMS_FindHouse::GetStaticDescription() const
{
    return FString::Printf(TEXT("%s: Sets '%s' to AI's assigned house (if space available)"),
        *Super::GetStaticDescription(), *BlackboardKey_Target.SelectedKeyName.ToString());
}