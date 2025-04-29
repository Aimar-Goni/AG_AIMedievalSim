#include "AI/Decorators/MS_NeedToStoreOrDeliver.h" 
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Systems/MS_InventoryComponent.h" 
#include "AIController.h"

UMS_NeedToStoreOrDeliver::UMS_NeedToStoreOrDeliver()
{
	NodeName = "Need To Store Or Deliver Items";

	// Initialize Blackboard Keys
	BlackboardKey_HasQuest.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_NeedToStoreOrDeliver, BlackboardKey_HasQuest));
	BlackboardKey_QuestType.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UMS_NeedToStoreOrDeliver, BlackboardKey_QuestType), StaticEnum<ResourceType>());
	BlackboardKey_QuestAmount.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UMS_NeedToStoreOrDeliver, BlackboardKey_QuestAmount));
    BlackboardKey_QuestTargetDestination.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_NeedToStoreOrDeliver, BlackboardKey_QuestTargetDestination), AActor::StaticClass());

    FlowAbortMode = EBTFlowAbortMode::Self;
}

void UMS_NeedToStoreOrDeliver::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);
    UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		BlackboardKey_QuestType.ResolveSelectedKey(*BBAsset);
        BlackboardKey_QuestTargetDestination.ResolveSelectedKey(*BBAsset);
	}
}

bool UMS_NeedToStoreOrDeliver::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
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
    const UObject* TargetDestination = Blackboard->GetValueAsObject(BlackboardKey_QuestTargetDestination.SelectedKeyName);

    if (QuestType == ResourceType::ERROR || QuestAmount <= 0)
	{
		return false;
	}

    // 3. Check inventory
	const int32 CurrentAmount = AIChar->Inventory_->GetResourceAmount(QuestType);

    // 4. Condition: Have enough items?
    const bool bHasEnoughItems = CurrentAmount >= QuestAmount;
    if(!bHasEnoughItems) return false; 

    // 5. Condition: Check destination based on configuration
    const bool bIsDeliveryQuest = (TargetDestination != nullptr);

    bool bConditionMet = false;
    if(bCheckForDelivery)
    {
        // Need delivery: Has enough items AND destination is set
        bConditionMet = bHasEnoughItems && bIsDeliveryQuest;
    }
    else // Checking for Storage
    {
        // Need storage: Has enough items AND destination is NOT set
        bConditionMet = bHasEnoughItems && !bIsDeliveryQuest;
    }


	return bConditionMet;
}

FString UMS_NeedToStoreOrDeliver::GetStaticDescription() const
{
    if(bCheckForDelivery)
    {
        return FString::Printf(TEXT("%s: Checks if Inventory >= '%s' AND '%s' is Set"),
            *Super::GetStaticDescription(),
            *BlackboardKey_QuestAmount.SelectedKeyName.ToString(),
            *BlackboardKey_QuestTargetDestination.SelectedKeyName.ToString());
    }
    else
    {
         return FString::Printf(TEXT("%s: Checks if Inventory >= '%s' AND '%s' is None"),
            *Super::GetStaticDescription(),
            *BlackboardKey_QuestAmount.SelectedKeyName.ToString(),
            *BlackboardKey_QuestTargetDestination.SelectedKeyName.ToString());
    }
}

    
