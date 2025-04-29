#include "AI/Services/MS_BTService_UpdateIdleStatus.h" // Adjust path
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UMS_BTService_UpdateIdleStatus::UMS_BTService_UpdateIdleStatus()
{
    NodeName = "Update Idle Status";
    BlackboardKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_BTService_UpdateIdleStatus, BlackboardKey));
    
    BlackboardKey_HasQuest.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_BTService_UpdateIdleStatus, BlackboardKey_HasQuest));
    BlackboardKey_IsSleeping.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_BTService_UpdateIdleStatus, BlackboardKey_IsSleeping));
    BlackboardKey_IsHungry.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_BTService_UpdateIdleStatus, BlackboardKey_IsHungry));
    BlackboardKey_IsThirsty.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_BTService_UpdateIdleStatus, BlackboardKey_IsThirsty));


    Interval = 0.5f;
    RandomDeviation = 0.1f; 
}

void UMS_BTService_UpdateIdleStatus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds); // Call base class tick

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;


    const bool bHasQuest = Blackboard->GetValueAsBool(BlackboardKey_HasQuest.SelectedKeyName);
    const bool bIsSleeping = Blackboard->GetValueAsBool(BlackboardKey_IsSleeping.SelectedKeyName);
    const bool bIsHungry = Blackboard->GetValueAsBool(BlackboardKey_IsHungry.SelectedKeyName);
    const bool bIsThirsty = Blackboard->GetValueAsBool(BlackboardKey_IsThirsty.SelectedKeyName);

    const bool bCurrentlyIdle = !bHasQuest && !bIsSleeping && !bIsHungry && !bIsThirsty;


    const bool bBlackboardIdleValue = Blackboard->GetValueAsBool(GetSelectedBlackboardKey());


    if (bCurrentlyIdle != bBlackboardIdleValue)
    {
        // Use SetValue function provided by base class UBTService_BlackboardBase
        Blackboard->SetValueAsBool(GetSelectedBlackboardKey(), bCurrentlyIdle);
    }
}

FString UMS_BTService_UpdateIdleStatus::GetStaticDescription() const
{
    // Use GetSelectedBlackboardKey() from the base class
	return FString::Printf(TEXT("Updates '%s' based on other states (HasQuest, IsSleeping, Needs)"), *GetSelectedBlackboardKey().ToString());
}