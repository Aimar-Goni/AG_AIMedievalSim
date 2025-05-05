#include "AI/TaskNodes/MS_CopyBlackboardValue.h" 
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Actor.h" 

UMS_CopyBlackboardValue::UMS_CopyBlackboardValue()
{
	NodeName = "Copy Blackboard Value";
	
	SourceBlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_CopyBlackboardValue, SourceBlackboardKey), UObject::StaticClass());
	DestinationBlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_CopyBlackboardValue, DestinationBlackboardKey), UObject::StaticClass());
}

void UMS_CopyBlackboardValue::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    // Ensure the keys actually resolve within the associated Blackboard asset
    UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset)) // ensure() logs an error in non-shipping builds if false
	{
		SourceBlackboardKey.ResolveSelectedKey(*BBAsset);
        DestinationBlackboardKey.ResolveSelectedKey(*BBAsset);
	}
}


EBTNodeResult::Type UMS_CopyBlackboardValue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const AAIController* Controller = OwnerComp.GetAIOwner();


	if (!Blackboard || !Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task '%s': Invalid Blackboard or Controller."), *GetNodeName());
		return EBTNodeResult::Failed;
	}

	// Check if the keys are actually set in the editor instance of this task
	if (!SourceBlackboardKey.IsSet() || SourceBlackboardKey.SelectedKeyName == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task '%s': Source Blackboard Key is not set."), *GetNodeName());
		return EBTNodeResult::Failed;
	}
    if (!DestinationBlackboardKey.IsSet() || DestinationBlackboardKey.SelectedKeyName == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task '%s': Destination Blackboard Key is not set."), *GetNodeName());
		return EBTNodeResult::Failed;
	}


    UObject* SourceObject = Blackboard->GetValueAsObject(SourceBlackboardKey.SelectedKeyName);


    // Set the value in the destination key
    Blackboard->SetValueAsObject(DestinationBlackboardKey.SelectedKeyName, SourceObject);

    UE_LOG(LogTemp, Verbose, TEXT("Task '%s': Copied value from '%s' (%s) to '%s'."),
        *GetNodeName(),
        *SourceBlackboardKey.SelectedKeyName.ToString(),
        *GetNameSafe(SourceObject), // Log the name of the object being copied
        *DestinationBlackboardKey.SelectedKeyName.ToString());

	return EBTNodeResult::Succeeded;
}

FString UMS_CopyBlackboardValue::GetStaticDescription() const
{
	// Provide a helpful description shown under the node name in the editor
	return FString::Printf(TEXT("Copy Value: %s -> %s"),
		*SourceBlackboardKey.SelectedKeyName.ToString(),
		*DestinationBlackboardKey.SelectedKeyName.ToString());
}