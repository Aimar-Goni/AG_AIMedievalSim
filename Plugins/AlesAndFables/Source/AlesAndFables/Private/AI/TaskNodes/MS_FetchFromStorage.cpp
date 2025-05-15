// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNodes/MS_FetchFromStorage.h"


#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AI/Characters/MS_AICharacter.h"
#include "Placeables/Buildings/MS_StorageBuilding.h" 
#include "Systems/MS_InventoryComponent.h" 

UMS_FetchFromStorage::UMS_FetchFromStorage()
{
	NodeName = "Fetch From Storage";

    BlackboardKey_TargetStorage.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FetchFromStorage, BlackboardKey_TargetStorage), AActor::StaticClass());
    BlackboardKey_QuestType.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FetchFromStorage, BlackboardKey_QuestType), StaticEnum<ResourceType>());
    BlackboardKey_QuestAmount.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FetchFromStorage, BlackboardKey_QuestAmount));
    BlackboardKey_QuestTargetDestination.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FetchFromStorage, BlackboardKey_QuestTargetDestination), AActor::StaticClass());
    BlackboardKey_IsFetchingMaterials.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FetchFromStorage, BlackboardKey_IsFetchingMaterials));
    BlackboardKey_IsDeliveringMaterials.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FetchFromStorage, BlackboardKey_IsDeliveringMaterials));
    BlackboardKey_NewMovementTarget.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FetchFromStorage, BlackboardKey_NewMovementTarget), AActor::StaticClass());
}

void UMS_FetchFromStorage::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);
    UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		BlackboardKey_QuestType.ResolveSelectedKey(*BBAsset); // Ensure enum key is resolved
	}
}

EBTNodeResult::Type UMS_FetchFromStorage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	AMS_AICharacter* AIChar = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;

	if (!Blackboard || !AIChar || !AIChar->Inventory_ || !AIChar->PawnStats_) return EBTNodeResult::Failed;

    AMS_StorageBuilding* Storage = Cast<AMS_StorageBuilding>(Blackboard->GetValueAsObject(BlackboardKey_TargetStorage.SelectedKeyName));
    if (!Storage || !Storage->Inventory_)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Target Storage or its inventory is invalid for %s."), *GetNodeName(), *AIChar->GetName());
        Blackboard->ClearValue(BlackboardKey_IsFetchingMaterials.SelectedKeyName); // Stop trying
        return EBTNodeResult::Failed;
    }

    ResourceType TypeNeeded = static_cast<ResourceType>(Blackboard->GetValueAsEnum(BlackboardKey_QuestType.SelectedKeyName));
    int32 AmountNeededForTrip = Blackboard->GetValueAsInt(BlackboardKey_QuestAmount.SelectedKeyName);

    if (TypeNeeded == ResourceType::ERROR || AmountNeededForTrip <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Invalid QuestType or QuestAmount for %s."), *GetNodeName(), *AIChar->GetName());
        Blackboard->ClearValue(BlackboardKey_IsFetchingMaterials.SelectedKeyName);
        return EBTNodeResult::Failed;
    }

    UE_LOG(LogTemp, Log, TEXT("%s: %s attempting to fetch %d %s from %s."),
        *GetNodeName(), *AIChar->GetName(), AmountNeededForTrip, *UEnum::GetValueAsString(TypeNeeded), *Storage->GetName());

    if (Storage->Inventory_->GetResourceAmount(TypeNeeded) >= AmountNeededForTrip)
    {
        int32 ExtractedAmount = Storage->Inventory_->ExtractFromResources(TypeNeeded, AmountNeededForTrip);
        if (ExtractedAmount > 0)
        {
            AIChar->Inventory_->AddToResources(TypeNeeded, ExtractedAmount);
            UE_LOG(LogTemp, Log, TEXT("%s: %s successfully fetched %d %s."), *GetNodeName(), *AIChar->GetName(), ExtractedAmount, *UEnum::GetValueAsString(TypeNeeded));

            // Transition state: Done fetching, now delivering
            Blackboard->ClearValue(BlackboardKey_IsFetchingMaterials.SelectedKeyName); // Set to false
            Blackboard->SetValueAsBool(BlackboardKey_IsDeliveringMaterials.SelectedKeyName, true);

            // Set new movement target to the construction site
            AActor* Destination = Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKey_QuestTargetDestination.SelectedKeyName));
            if (Destination)
            {
                Blackboard->SetValueAsObject(BlackboardKey_NewMovementTarget.SelectedKeyName, Destination);
       
                AIChar->Path_ =  AIChar->CreateMovementPath(Destination); 
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("%s: %s fetched items but QuestTargetDestination is invalid! Quest stuck."), *GetNodeName(), *AIChar->GetName());
                // Clear delivery state as well if destination is bad
                Blackboard->ClearValue(BlackboardKey_IsDeliveringMaterials.SelectedKeyName);
                return EBTNodeResult::Failed; // Critical error
            }
            return EBTNodeResult::Succeeded;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("%s: %s - Storage ExtractFromResources failed (returned %d) for %s."), *GetNodeName(), *AIChar->GetName(), ExtractedAmount, *UEnum::GetValueAsString(TypeNeeded));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %s - Not enough %s in storage %s to fetch %d."), *GetNodeName(), *AIChar->GetName(), *UEnum::GetValueAsString(TypeNeeded), *Storage->GetName(), AmountNeededForTrip);
    }

    // If any failure above, clear fetching state and fail task
    Blackboard->ClearValue(BlackboardKey_IsFetchingMaterials.SelectedKeyName);
	return EBTNodeResult::Failed;
}

FString UMS_FetchFromStorage::GetStaticDescription() const
{
    return FString::Printf(TEXT("%s: Fetches '%s' from '%s', sets new target to '%s'"),
        *Super::GetStaticDescription(),
        *BlackboardKey_QuestType.SelectedKeyName.ToString(),
        *BlackboardKey_TargetStorage.SelectedKeyName.ToString(),
        *BlackboardKey_QuestTargetDestination.SelectedKeyName.ToString());
}