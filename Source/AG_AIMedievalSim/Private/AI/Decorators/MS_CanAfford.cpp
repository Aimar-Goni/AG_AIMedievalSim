#include "AI/Decorators/MS_CanAfford.h" 
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Characters/MS_AICharacter.h"
#include "AIController.h"

UMS_CanAfford::UMS_CanAfford()
{
    NodeName = "Can Afford";
    BlackboardKey_Cost.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UMS_CanAfford, BlackboardKey_Cost));
    FlowAbortMode = EBTFlowAbortMode::None; // Usually doesn't need to abort on its own change
}

void UMS_CanAfford::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);
    UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		BlackboardKey_Cost.ResolveSelectedKey(*BBAsset);
	}
}

bool UMS_CanAfford::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return false; // No controller, no pawn

	const AMS_AICharacter* AIChar = Cast<AMS_AICharacter>(Controller->GetPawn());
	if (!AIChar) return false; // No AI Character

    int32 ActualCost = LiteralCost;
    if(Blackboard && BlackboardKey_Cost.IsSet() && BlackboardKey_Cost.SelectedKeyName != NAME_None)
    {
        if(Blackboard->GetKeyID(BlackboardKey_Cost.SelectedKeyName) != FBlackboard::InvalidKey)
        {
            ActualCost = Blackboard->GetValueAsInt(BlackboardKey_Cost.SelectedKeyName);
        }
        else // Key selected in decorator details but not found on current BB
        {
            UE_LOG(LogTemp, Warning, TEXT("Decorator '%s': BlackboardKey_Cost '%s' is selected but not found on Blackboard for %s."),
                *GetNodeName(), *BlackboardKey_Cost.SelectedKeyName.ToString(), *AIChar->GetName());
            return false;
        }
    }

    if (ActualCost < 0) ActualCost = 0; // Cost cannot be negative

    const bool bCanAfford = AIChar->Money >= ActualCost;


    return bCanAfford;
}

FString UMS_CanAfford::GetStaticDescription() const
{
    if(BlackboardKey_Cost.IsSet() && BlackboardKey_Cost.SelectedKeyName != NAME_None)
    {
        return FString::Printf(TEXT("%s: Money >= Cost from '%s'"), *Super::GetStaticDescription(), *BlackboardKey_Cost.SelectedKeyName.ToString());
    }
    return FString::Printf(TEXT("%s: Money >= %d"), *Super::GetStaticDescription(), LiteralCost);
}