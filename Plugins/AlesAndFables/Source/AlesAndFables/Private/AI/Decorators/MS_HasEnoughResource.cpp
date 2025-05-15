// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorators/MS_HasEnoughResource.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Systems/MS_InventoryComponent.h" 
#include "AIController.h"

UMS_HasEnoughResource::UMS_HasEnoughResource()
{
	NodeName = "Has Enough Resource";

	// Initialize Blackboard Keys
	BlackboardKey_ResourceTypeToCheck.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UMS_HasEnoughResource, BlackboardKey_ResourceTypeToCheck), StaticEnum<ResourceType>());
	BlackboardKey_AmountNeeded.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UMS_HasEnoughResource, BlackboardKey_AmountNeeded));

    // Default flow control
    FlowAbortMode = EBTFlowAbortMode::None; // Or Self if you want it to re-evaluate if inventory changes
}

void UMS_HasEnoughResource::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);
    UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		BlackboardKey_ResourceTypeToCheck.ResolveSelectedKey(*BBAsset);
        BlackboardKey_AmountNeeded.ResolveSelectedKey(*BBAsset);
	}
}

bool UMS_HasEnoughResource::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Blackboard || !Controller) return false;

	const AMS_AICharacter* AIChar = Cast<AMS_AICharacter>(Controller->GetPawn());
	if (!AIChar || !AIChar->Inventory_)
    {
        // UE_LOG(LogTemp, Verbose, TEXT("Decorator '%s': AIChar or Inventory invalid."), *GetNodeName());
        return false;
    }

	//  Determine ResourceType to Check 
	ResourceType ActualResourceType = ResourceTypeToCheck; // Use direct property as default
	if (BlackboardKey_ResourceTypeToCheck.IsSet() && BlackboardKey_ResourceTypeToCheck.SelectedKeyName != NAME_None)
	{
        // Check if the key actually exists on this BB instance before trying to get its value
        if(Blackboard->GetKeyID(BlackboardKey_ResourceTypeToCheck.SelectedKeyName) != FBlackboard::InvalidKey)
        {
		    ActualResourceType = static_cast<ResourceType>(Blackboard->GetValueAsEnum(BlackboardKey_ResourceTypeToCheck.SelectedKeyName));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Decorator '%s': BlackboardKey_ResourceTypeToCheck '%s' is selected but does not exist on the blackboard."),
                *GetNodeName(), *BlackboardKey_ResourceTypeToCheck.SelectedKeyName.ToString());
            return false; // Fail if key is selected but not found
        }
	}

	//  Determine Amount Needed 
	int32 ActualAmountNeeded = AmountNeeded; // Use direct property as default
	if (BlackboardKey_AmountNeeded.IsSet() && BlackboardKey_AmountNeeded.SelectedKeyName != NAME_None)
	{
         if(Blackboard->GetKeyID(BlackboardKey_AmountNeeded.SelectedKeyName) != FBlackboard::InvalidKey)
        {
		    ActualAmountNeeded = Blackboard->GetValueAsInt(BlackboardKey_AmountNeeded.SelectedKeyName);
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("Decorator '%s': BlackboardKey_AmountNeeded '%s' is selected but does not exist on the blackboard."),
                *GetNodeName(), *BlackboardKey_AmountNeeded.SelectedKeyName.ToString());
            return false; // Fail if key is selected but not found
        }
	}

	if (ActualResourceType == ResourceType::ERROR || ActualAmountNeeded < 0) // Amount can be 0 if checking "has any"
	{
		return false;
	}

	//  Check Inventory 
	const int32 CurrentAmountInInventory = AIChar->Inventory_->GetResourceAmount(ActualResourceType);
	const bool bConditionMet = CurrentAmountInInventory >= ActualAmountNeeded;
	
	return bConditionMet;
}

FString UMS_HasEnoughResource::GetStaticDescription() const
{
    FString ResourceDesc = (BlackboardKey_ResourceTypeToCheck.IsSet() && BlackboardKey_ResourceTypeToCheck.SelectedKeyName != NAME_None) ?
        BlackboardKey_ResourceTypeToCheck.SelectedKeyName.ToString() : UEnum::GetValueAsString(ResourceTypeToCheck);

    FString AmountDesc = (BlackboardKey_AmountNeeded.IsSet() && BlackboardKey_AmountNeeded.SelectedKeyName != NAME_None) ?
        BlackboardKey_AmountNeeded.SelectedKeyName.ToString() : FString::FromInt(AmountNeeded);

	return FString::Printf(TEXT("%s: Inventory[%s] >= %s"),
		*Super::GetStaticDescription(),
        *ResourceDesc,
        *AmountDesc);
}