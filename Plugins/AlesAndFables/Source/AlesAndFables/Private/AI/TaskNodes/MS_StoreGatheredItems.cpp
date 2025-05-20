#include "AI/TaskNodes/MS_StoreGatheredItems.h" 
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AI/Characters/MS_AICharacter.h" 
#include "Placeables/Buildings/MS_StorageBuilding.h" 
#include "Systems/MS_InventoryComponent.h" 

UMS_StoreGatheredItems::UMS_StoreGatheredItems()
{
	NodeName = "Store Gathered Items";

    BlackboardKey_TargetStorage.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_StoreGatheredItems, BlackboardKey_TargetStorage), AActor::StaticClass());
    BlackboardKey_QuestType.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UMS_StoreGatheredItems, BlackboardKey_QuestType), StaticEnum<ResourceType>());
    BlackboardKey_IsStoringItems.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_StoreGatheredItems, BlackboardKey_IsStoringItems));
}

void UMS_StoreGatheredItems::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);
    UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		BlackboardKey_QuestType.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UMS_StoreGatheredItems::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	AMS_AICharacter* AIChar = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;

	if (!Blackboard || !AIChar || !AIChar->Inventory_) return EBTNodeResult::Failed;

    AMS_StorageBuilding* Storage = Cast<AMS_StorageBuilding>(Blackboard->GetValueAsObject(BlackboardKey_TargetStorage.SelectedKeyName));
    if (!Storage || !Storage->Inventory_)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Target Storage or its inventory is invalid for %s."), *GetNodeName(), *AIChar->GetName());
        Blackboard->ClearValue(BlackboardKey_IsStoringItems.SelectedKeyName);
        return EBTNodeResult::Failed;
    }

    ResourceType TypeToStore = static_cast<ResourceType>(Blackboard->GetValueAsEnum(BlackboardKey_QuestType.SelectedKeyName));
    if (TypeToStore == ResourceType::ERROR)
    {
         UE_LOG(LogTemp, Warning, TEXT("%s: QuestType is ERROR for storing by %s."), *GetNodeName(), *AIChar->GetName());
         Blackboard->ClearValue(BlackboardKey_IsStoringItems.SelectedKeyName);
         return EBTNodeResult::Failed;
    }

    int32 AmountInInventory = AIChar->Inventory_->GetResourceAmount(TypeToStore);

    UE_LOG(LogTemp, Log, TEXT("%s: %s attempting to store %d %s in %s."),
        *GetNodeName(), *AIChar->GetName(), AmountInInventory, *UEnum::GetValueAsString(TypeToStore), *Storage->GetName());

    if (AmountInInventory > 0)
    {
        Storage->Inventory_->AddToResources(TypeToStore, AmountInInventory);
        AIChar->Inventory_->ExtractFromResources(TypeToStore, AmountInInventory); // Remove all of this type

        UE_LOG(LogTemp, Log, TEXT("%s: %s successfully stored %d %s."), *GetNodeName(), *AIChar->GetName(), AmountInInventory, *UEnum::GetValueAsString(TypeToStore));

        AIChar->CompleteCurrentQuest(); // This was a gather quest, now complete

        Blackboard->ClearValue(BlackboardKey_IsStoringItems.SelectedKeyName); // Set to false
        return EBTNodeResult::Succeeded;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %s reached storage to store %s, but has none."), *GetNodeName(), *AIChar->GetName(), *UEnum::GetValueAsString(TypeToStore));
        AIChar->CompleteCurrentQuest();
    }

    Blackboard->ClearValue(BlackboardKey_IsStoringItems.SelectedKeyName);
	return EBTNodeResult::Succeeded;
}

FString UMS_StoreGatheredItems::GetStaticDescription() const
{
     return FString::Printf(TEXT("%s: Stores all '%s' items in '%s'"),
        *Super::GetStaticDescription(),
        *BlackboardKey_QuestType.SelectedKeyName.ToString(),
        *BlackboardKey_TargetStorage.SelectedKeyName.ToString());
}