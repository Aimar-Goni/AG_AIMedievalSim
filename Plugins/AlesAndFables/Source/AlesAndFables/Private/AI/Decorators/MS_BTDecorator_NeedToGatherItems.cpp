// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Decorators/MS_BTDecorator_NeedToGatherItems.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Characters/MS_AICharacter.h" 
#include "Systems/MS_InventoryComponent.h" 
#include "AIController.h"

UMS_BTDecorator_NeedToGatherItems::UMS_BTDecorator_NeedToGatherItems()
{
	NodeName = "Need To Gather Items";

	// Initialize Blackboard Keys
	BlackboardKey_HasQuest.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_BTDecorator_NeedToGatherItems, BlackboardKey_HasQuest));
	BlackboardKey_QuestType.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UMS_BTDecorator_NeedToGatherItems, BlackboardKey_QuestType), StaticEnum<ResourceType>()); // Assumes ResourceType is registered UENUM
	BlackboardKey_QuestAmount.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UMS_BTDecorator_NeedToGatherItems, BlackboardKey_QuestAmount));

    // Default flow control: condition must be met to execute branch
    FlowAbortMode = EBTFlowAbortMode::Self; // Abort this branch if condition changes
}

void UMS_BTDecorator_NeedToGatherItems::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    // Ensure ResourceType enum is used for the QuestType key
    UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		BlackboardKey_QuestType.ResolveSelectedKey(*BBAsset);
	}
}


bool UMS_BTDecorator_NeedToGatherItems::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Blackboard || !Controller) return false;

	const AMS_AICharacter* AIChar = Cast<AMS_AICharacter>(Controller->GetPawn());
	if (!AIChar || !AIChar->Inventory_) return false;

	// 1. Check if a quest is assigned
	if (!Blackboard->GetValueAsBool(BlackboardKey_HasQuest.SelectedKeyName))
	{
		return false; 
	}

	// 2. Get quest details
	const ResourceType QuestType = static_cast<ResourceType>(Blackboard->GetValueAsEnum(BlackboardKey_QuestType.SelectedKeyName));
	const int32 QuestAmount = Blackboard->GetValueAsInt(BlackboardKey_QuestAmount.SelectedKeyName);

	if (QuestType == ResourceType::ERROR || QuestAmount <= 0)
	{
		return false;
	}

	// 3. Check inventory
	const int32 CurrentAmount = AIChar->Inventory_->GetResourceAmount(QuestType);

	// 4. Condition: Need more items than currently held
	const bool bConditionMet = CurrentAmount < QuestAmount;
	
	return bConditionMet;
}

FString UMS_BTDecorator_NeedToGatherItems::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: Checks if '%s' < '%s'"), *Super::GetStaticDescription(),
		*BlackboardKey_QuestType.SelectedKeyName.ToString(),
		*BlackboardKey_QuestAmount.SelectedKeyName.ToString());
}