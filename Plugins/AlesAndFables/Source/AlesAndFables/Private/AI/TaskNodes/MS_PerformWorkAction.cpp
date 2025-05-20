
#include "AI/TaskNodes/MS_PerformWorkAction.h" 
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Characters/MS_AICharacter.h" 
#include "Systems/MS_ResourceSystem.h"   
#include "Placeables/Interactables/MS_BaseWorkPlace.h" 
#include "Placeables/Buildings//MS_WheatField.h"  
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"

UMS_PerformWorkAction::UMS_PerformWorkAction()
{
	NodeName = "Perform Work Action At Target";
	bNotifyTick = true;

	BlackboardKey_WorkplaceTarget.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_PerformWorkAction, BlackboardKey_WorkplaceTarget), AActor::StaticClass());
	BlackboardKey_IsAtTargetWorkplace.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_PerformWorkAction, BlackboardKey_IsAtTargetWorkplace));
    BlackboardKey_QuestType.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UMS_PerformWorkAction, BlackboardKey_QuestType), StaticEnum<ResourceType>());
    BlackboardKey_QuestAmount.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UMS_PerformWorkAction, BlackboardKey_QuestAmount));

	INIT_TASK_NODE_NOTIFY_FLAGS();
}

uint16 UMS_PerformWorkAction::GetInstanceMemorySize() const
{
	return sizeof(FMSPerformWorkActionMemory);
}

void UMS_PerformWorkAction::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	FMSPerformWorkActionMemory* MyMemory = CastInstanceNodeMemory<FMSPerformWorkActionMemory>(NodeMemory);
	MyMemory->StartTime = -1.0f;
	MyMemory->WorkplaceActor = nullptr;
	MyMemory->bAnimationStarted = false;
    MyMemory->ActiveWorkMontage = nullptr;
}

void UMS_PerformWorkAction::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
    FMSPerformWorkActionMemory* MyMemory = CastInstanceNodeMemory<FMSPerformWorkActionMemory>(NodeMemory);
	MyMemory->WorkplaceActor = nullptr;
    MyMemory->ActiveWorkMontage = nullptr;
}


EBTNodeResult::Type UMS_PerformWorkAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!AIController || !Blackboard) return EBTNodeResult::Failed;

	AMS_AICharacter* AIChar = Cast<AMS_AICharacter>(AIController->GetPawn());
	if (!AIChar) return EBTNodeResult::Failed;

	AMS_BaseWorkPlace* TargetWorkplace = Cast<AMS_BaseWorkPlace>(Blackboard->GetValueAsObject(BlackboardKey_WorkplaceTarget.SelectedKeyName));
	if (!IsValid(TargetWorkplace))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Invalid Workplace target for %s."), *GetNodeName(), *AIChar->GetName());
        CleanupTaskState(OwnerComp, NodeMemory);
		return EBTNodeResult::Failed;
	}

    // Specific check for generic workplaces if they have limited resources
    AMS_WheatField* WheatFieldTarget = Cast<AMS_WheatField>(TargetWorkplace);
    if (!WheatFieldTarget && !TargetWorkplace->ResourceAvaliable_) // If generic workplace AND resource not available
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Generic Workplace %s resource not available for %s."), *GetNodeName(), *TargetWorkplace->GetName(), *AIChar->GetName());
        CleanupTaskState(OwnerComp, NodeMemory);
		return EBTNodeResult::Failed;
    }


	FMSPerformWorkActionMemory* MyMemory = CastInstanceNodeMemory<FMSPerformWorkActionMemory>(NodeMemory);
	MyMemory->StartTime = GetWorld()->GetTimeSeconds();
	MyMemory->WorkplaceActor = TargetWorkplace;
    MyMemory->bAnimationStarted = false;
    MyMemory->ActiveWorkMontage = nullptr;

	//  Select Montage 
	TObjectPtr<UAnimMontage> MontageToPlay = nullptr;
    ResourceType QuestTypeBB = static_cast<ResourceType>(Blackboard->GetValueAsEnum(BlackboardKey_QuestType.SelectedKeyName));
    int32 QuestAmountBB = Blackboard->GetValueAsInt(BlackboardKey_QuestAmount.SelectedKeyName);

    if (WheatFieldTarget) // If it's a wheat field, prioritize specific field action montages
    {
        EFieldState FieldState = WheatFieldTarget->GetCurrentFieldState();
        if (QuestAmountBB == -1 && FieldState == EFieldState::Constructed && PlantingMontage) // Planting convention
        {
            MontageToPlay = PlantingMontage;
        }
        else if (QuestTypeBB == ResourceType::WATER && FieldState == EFieldState::Planted && WateringMontage)
        {
            MontageToPlay = WateringMontage;
        }
        else if (QuestTypeBB == ResourceType::WHEAT && QuestAmountBB > 0 && FieldState == EFieldState::ReadyToHarvest && HarvestingMontage)
        {
            MontageToPlay = HarvestingMontage;
        }
    }

    if (!MontageToPlay) // Fallback to ResourceType map or default
    {
	    if (TObjectPtr<UAnimMontage>* FoundMontage = WorkMontages.Find(QuestTypeBB))
	    {
		    MontageToPlay = *FoundMontage;
	    }
        else if (DefaultWorkMontage)
        {
            MontageToPlay = DefaultWorkMontage;
        }
    }

	//  Play Animation 
	UAnimInstance* AnimInstance = AIChar->GetMesh() ? AIChar->GetMesh()->GetAnimInstance() : nullptr;
	if (MontageToPlay && AnimInstance)
	{
		AnimInstance->Montage_Play(MontageToPlay, 1.0f);
        MyMemory->bAnimationStarted = true;
        MyMemory->ActiveWorkMontage = MontageToPlay;
        UE_LOG(LogTemp, Log, TEXT("%s: %s started montage '%s' at '%s'."),
               *GetNodeName(), *AIChar->GetName(), *GetNameSafe(MontageToPlay), *TargetWorkplace->GetName());
	}
	else { UE_LOG(LogTemp, Warning, TEXT("%s: %s - No valid montage or AnimInstance for action at '%s'."), *GetNodeName(), *AIChar->GetName(), *TargetWorkplace->GetName()); }

	return EBTNodeResult::InProgress;
}

void UMS_PerformWorkAction::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FMSPerformWorkActionMemory* MyMemory = CastInstanceNodeMemory<FMSPerformWorkActionMemory>(NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AMS_AICharacter* AIChar = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;

	if (!AIChar || !MyMemory || !MyMemory->WorkplaceActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Invalid AIChar, Memory, or WorkplaceActor in Tick. Aborting."), *GetNodeName());
		CleanupTaskState(OwnerComp, NodeMemory);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AMS_BaseWorkPlace* Workplace = MyMemory->WorkplaceActor.Get();

	// Check if work duration has elapsed
	if (GetWorld()->GetTimeSeconds() - MyMemory->StartTime >= WorkDuration)
	{
        ResourceType QuestTypeBB = static_cast<ResourceType>(Blackboard->GetValueAsEnum(BlackboardKey_QuestType.SelectedKeyName));
        int32 QuestAmountBB = Blackboard->GetValueAsInt(BlackboardKey_QuestAmount.SelectedKeyName);
        bool bActionSuccessful = false;
        FResource ReceivedResource = {ResourceType::ERROR, 0};

        AMS_WheatField* WheatFieldTarget = Cast<AMS_WheatField>(Workplace);

        UE_LOG(LogTemp, Log, TEXT("%s: %s work duration complete at %s."), *GetNodeName(), *AIChar->GetName(), *Workplace->GetName());

        if (WheatFieldTarget)
        {
            EFieldState FieldState = WheatFieldTarget->GetCurrentFieldState();
            UE_LOG(LogTemp, Verbose, TEXT("%s: Interacting with WheatField %s (State: %s, QuestTypeBB: %s, QuestAmountBB: %d)"),
                *GetNodeName(), *WheatFieldTarget->GetName(), *UEnum::GetValueAsString(FieldState), *UEnum::GetValueAsString(QuestTypeBB), QuestAmountBB);

            if (QuestAmountBB == -1 && FieldState == EFieldState::Constructed) // PLANTING
            {
                if (WheatFieldTarget->PerformPlanting()) bActionSuccessful = true;
            }
            else if (QuestTypeBB == ResourceType::WATER && FieldState == EFieldState::Planted) // WATERING
            {
                if (AIChar->Inventory_ && AIChar->Inventory_->GetResourceAmount(ResourceType::WATER) >= QuestAmountBB)
                {
                    if (WheatFieldTarget->PerformWatering())
                    {
                        AIChar->Inventory_->ExtractFromResources(ResourceType::WATER, QuestAmountBB);
                        bActionSuccessful = true;
                    }
                } else { UE_LOG(LogTemp, Warning, TEXT("%s: %s at field to water, but no/not enough water! Has: %d, Needs for trip: %d"), *GetNodeName(), *AIChar->GetName(), AIChar->Inventory_ ? AIChar->Inventory_->GetResourceAmount(ResourceType::WATER) : -1, QuestAmountBB); }
            }
            else if (QuestTypeBB == ResourceType::WHEAT && QuestAmountBB > 0 && FieldState == EFieldState::ReadyToHarvest) // HARVESTING
            {
                ReceivedResource = WheatFieldTarget->PerformHarvesting();
                if (ReceivedResource.Type != ResourceType::ERROR) bActionSuccessful = true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("%s: %s action/state mismatch for WheatField %s (QuestType: %s, QAmount: %d, FieldState: %s)."),
                    *GetNodeName(), *AIChar->GetName(), *WheatFieldTarget->GetName(), *UEnum::GetValueAsString(QuestTypeBB), QuestAmountBB, *UEnum::GetValueAsString(FieldState));
            }
        }
        else // Generic Workplace
        {
            if (Workplace->ResourceAvaliable_)
            {
                ReceivedResource = Workplace->TakeResources();
                if (ReceivedResource.Type != ResourceType::ERROR) bActionSuccessful = true;
            }
            else { UE_LOG(LogTemp, Warning, TEXT("%s: Generic workplace %s resource became unavailable for %s."), *GetNodeName(), *Workplace->GetName(), *AIChar->GetName()); }
        }


        //  Post-Action Logic 
        if (bActionSuccessful)
        {
            // Add gathered resources to inventory
            if (ReceivedResource.Type != ResourceType::ERROR && ReceivedResource.Amount > 0 && AIChar->Inventory_)
            {
                AIChar->Inventory_->AddToResources(ReceivedResource.Type, ReceivedResource.Amount);
                 UE_LOG(LogTemp, Log, TEXT("%s: %s added %d of %s to inventory."), *GetNodeName(), *AIChar->GetName(), ReceivedResource.Amount, *UEnum::GetValueAsString(ReceivedResource.Type));
            }

    
            if (WheatFieldTarget &&
                ( (QuestAmountBB == -1 && QuestTypeBB == ResourceType::WHEAT) || // Planted
                  (QuestTypeBB == ResourceType::WATER) ) )                      // Watered
            {
                if (AIChar->AssignedQuest.QuestID.IsValid()) AIChar->CompleteCurrentQuest();
            }

            // Remove generic workplace from pool (NOT Wheat Fields)
            if (!WheatFieldTarget)
            {
                AMS_WorkpPlacePool* WorkplacePool = Cast<AMS_WorkpPlacePool>(UGameplayStatics::GetActorOfClass(GetWorld(), AMS_WorkpPlacePool::StaticClass()));
                if (WorkplacePool) WorkplacePool->RemoveWorkplaceAndFreeNode(Workplace);
                else UE_LOG(LogTemp, Warning, TEXT("%s: Could not find Workplace Pool manager to remove %s!"), *GetNodeName(), *Workplace->GetName());
            }

            // Play finish montage (optional)
            UAnimInstance* AnimInstance = AIChar->GetMesh() ? AIChar->GetMesh()->GetAnimInstance() : nullptr;
            if (AnimInstance)
            {
                if (MyMemory->bAnimationStarted && MyMemory->ActiveWorkMontage)
                {
                    AnimInstance->Montage_Stop(0.1f, MyMemory->ActiveWorkMontage);
                }
                TObjectPtr<UAnimMontage> FinishMontage = nullptr;
                if (TObjectPtr<UAnimMontage>* FoundFinish = FinishWorkMontages.Find(QuestTypeBB)) FinishMontage = *FoundFinish;
                if (FinishMontage) AnimInstance->Montage_Play(FinishMontage, 1.0f);
            }

            CleanupTaskState(OwnerComp, NodeMemory);
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
        else // Action failed at the workplace
        {
            UE_LOG(LogTemp, Warning, TEXT("%s: %s failed to perform action at %s."), *GetNodeName(), *AIChar->GetName(), *Workplace->GetName());
            CleanupTaskState(OwnerComp, NodeMemory);
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        }
	}
}

EBTNodeResult::Type UMS_PerformWorkAction::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogTemp, Log, TEXT("%s: Task aborted."), *GetNodeName());
	CleanupTaskState(OwnerComp, NodeMemory);
	return EBTNodeResult::Aborted;
}

void UMS_PerformWorkAction::CleanupTaskState(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FMSPerformWorkActionMemory* MyMemory = CastInstanceNodeMemory<FMSPerformWorkActionMemory>(NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();

	if (MyMemory->bAnimationStarted && MyMemory->ActiveWorkMontage)
	{
		if (AMS_AICharacter* AIChar = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr)
		{
			UAnimInstance* AnimInstance = AIChar->GetMesh() ? AIChar->GetMesh()->GetAnimInstance() : nullptr;
			if (AnimInstance && AnimInstance->Montage_IsPlaying(MyMemory->ActiveWorkMontage))
			{
				AnimInstance->Montage_Stop(0.2f, MyMemory->ActiveWorkMontage);
			}
		}
	}
    MyMemory->bAnimationStarted = false;
    MyMemory->ActiveWorkMontage = nullptr;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard && BlackboardKey_IsAtTargetWorkplace.IsSet() && BlackboardKey_IsAtTargetWorkplace.SelectedKeyName != NAME_None)
	{
		Blackboard->ClearValue(BlackboardKey_IsAtTargetWorkplace.SelectedKeyName);
	}
    MyMemory->WorkplaceActor = nullptr;
}