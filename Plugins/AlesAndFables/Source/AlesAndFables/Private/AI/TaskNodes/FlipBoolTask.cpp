#include "AI/TaskNodes/FlipBoolTask.h"
#include "BehaviorTree/BlackboardComponent.h"

UFlipBoolTask::UFlipBoolTask()
{
	NodeName = "Flip Bool";
	bNotifyTick = false;
}

EBTNodeResult::Type UFlipBoolTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp || !BoolKey.SelectedKeyName.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	const bool bCurrentValue = BlackboardComp->GetValueAsBool(BoolKey.SelectedKeyName);
	BlackboardComp->SetValueAsBool(BoolKey.SelectedKeyName, !bCurrentValue);

	return EBTNodeResult::Succeeded;
}