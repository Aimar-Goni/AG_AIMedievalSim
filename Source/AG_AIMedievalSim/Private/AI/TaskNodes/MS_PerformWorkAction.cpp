// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNodes/MS_PerformWorkAction.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Characters/MS_AICharacter.h" 
#include "Placeables/Interactables/MS_BaseWorkPlace.h" 
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Systems/MS_ResourceSystem.h"
#include "Systems/MS_InventoryComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h" 


UMS_PerformWorkAction::UMS_PerformWorkAction()
{
	NodeName = "Perform Work Action";
	bNotifyTick = true; 


	BlackboardKey_WorkplaceTarget.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_PerformWorkAction, BlackboardKey_WorkplaceTarget), AActor::StaticClass());
	BlackboardKey_IsAtLocation.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_PerformWorkAction, BlackboardKey_IsAtLocation));
	
}

uint16 UMS_PerformWorkAction::GetInstanceMemorySize() const
{
	return sizeof(FMSPerformWorkActionMemory);
}

void UMS_PerformWorkAction::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
    // Initialize the struct in the allocated node memory
	FMSPerformWorkActionMemory* MyMemory = CastInstanceNodeMemory<FMSPerformWorkActionMemory>(NodeMemory);
	MyMemory->StartTime = -1.0f;
	MyMemory->WorkplaceActor = nullptr;
	MyMemory->bAnimationStarted = false;
}

void UMS_PerformWorkAction::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
    FMSPerformWorkActionMemory* MyMemory = CastInstanceNodeMemory<FMSPerformWorkActionMemory>(NodeMemory);
	MyMemory->WorkplaceActor = nullptr;
}

EBTNodeResult::Type UMS_PerformWorkAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!AIController || !Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(AIController->GetPawn());
	if (!AICharacter)
	{
        UE_LOG(LogTemp, Warning, TEXT("PerformWorkAction: Failed to get AI Character."));
		return EBTNodeResult::Failed;
	}

	// *** Get Quest Type ***
	ResourceType CurrentQuestType = AICharacter->AssignedQuest.Type;
	if (CurrentQuestType == ResourceType::ERROR) // Make sure the AI has a valid quest assigned
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformWorkAction: AI Character '%s' has no valid Quest Type (ERROR). Cannot determine animation."), *AICharacter->GetName());
		// Decide how to handle this: fail, or use default animation? Let's fail for now.
		return EBTNodeResult::Failed;
	}

	// Get the target workplace from the blackboard
	AMS_BaseWorkPlace* Workplace = Cast<AMS_BaseWorkPlace>(Blackboard->GetValueAsObject(BlackboardKey_WorkplaceTarget.SelectedKeyName));
	if (!IsValid(Workplace) || !Workplace->ResourceAvaliable_)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformWorkAction: Invalid or unavailable Workplace target '%s'."), *GetNameSafe(Workplace));
        CleanupTask(OwnerComp, NodeMemory);
		return EBTNodeResult::Failed;
	}

	// Get Node Memory
	FMSPerformWorkActionMemory* MyMemory = CastInstanceNodeMemory<FMSPerformWorkActionMemory>(NodeMemory);

	// Store start time and workplace reference
	MyMemory->StartTime = GetWorld()->GetTimeSeconds();
	MyMemory->WorkplaceActor = Workplace;
    MyMemory->bAnimationStarted = false;
    MyMemory->ActiveWorkMontage = nullptr; // Reset active montage

	// *** Select Montage based on Quest Type ***
	TObjectPtr<UAnimMontage> MontageToPlay = nullptr;
	if (TObjectPtr<UAnimMontage>* FoundMontage = WorkMontages.Find(CurrentQuestType))
	{
		MontageToPlay = *FoundMontage; // Use the montage found in the map
	}

    // If no specific montage found, try the default one
    if (!MontageToPlay && DefaultWorkMontage)
    {
        MontageToPlay = DefaultWorkMontage;
        UE_LOG(LogTemp, Log, TEXT("PerformWorkAction: No specific montage found for ResourceType %s. Using default montage '%s'."), *UEnum::GetValueAsString(CurrentQuestType), *GetNameSafe(MontageToPlay));
    }

	// Play starting animation montage if found
	UAnimInstance* AnimInstance = AICharacter->GetMesh() ? AICharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (MontageToPlay && AnimInstance)
	{
		float PlayRate = 1.0f;
		AnimInstance->Montage_Play(MontageToPlay, PlayRate);
        MyMemory->bAnimationStarted = true;
        MyMemory->ActiveWorkMontage = MontageToPlay; // Store which montage is playing
        UE_LOG(LogTemp, Log, TEXT("PerformWorkAction: Started montage '%s' for ResourceType %s on %s."),
               *GetNameSafe(MontageToPlay), *UEnum::GetValueAsString(CurrentQuestType), *AICharacter->GetName());
	}
	else
	{
        UE_LOG(LogTemp, Warning, TEXT("PerformWorkAction: No valid montage found or AnimInstance invalid for ResourceType %s on %s."),
               *UEnum::GetValueAsString(CurrentQuestType), *AICharacter->GetName());
        // Proceed without animation? Or fail? Let's proceed without animation for now.
        MyMemory->bAnimationStarted = false;
        MyMemory->ActiveWorkMontage = nullptr;
    }

	return EBTNodeResult::InProgress;
}

void UMS_PerformWorkAction::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FMSPerformWorkActionMemory* MyMemory = CastInstanceNodeMemory<FMSPerformWorkActionMemory>(NodeMemory);

	// Check if workplace became invalid during the task
	if (!MyMemory->WorkplaceActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformWorkAction: Workplace became invalid during task. Aborting."));
		CleanupTask(OwnerComp, NodeMemory);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check if work duration has elapsed
	if (GetWorld()->GetTimeSeconds() - MyMemory->StartTime >= WorkDuration)
	{
		AAIController* AIController = OwnerComp.GetAIOwner();
		AMS_AICharacter* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;

		if (AICharacter && MyMemory->WorkplaceActor.IsValid())
		{
			AMS_BaseWorkPlace* Workplace = MyMemory->WorkplaceActor.Get();
			ResourceType CurrentQuestType = AICharacter->AssignedQuest.Type; // Get type for finish anim

            if(Workplace->ResourceAvaliable_)
            {
                UE_LOG(LogTemp, Log, TEXT("PerformWorkAction: Work duration complete for %s at %s."), *AICharacter->GetName(), *Workplace->GetName());

                FResource ReceivedResource = Workplace->TakeResources();
                if (AICharacter->Inventory_)
                {
                    AICharacter->Inventory_->AddToResources(ReceivedResource.Type, ReceivedResource.Amount);
                    UE_LOG(LogTemp, Log, TEXT("PerformWorkAction: %s received %d of %s."), *AICharacter->GetName(), ReceivedResource.Amount, *UEnum::GetValueAsString(ReceivedResource.Type));
                }

                AMS_WorkpPlacePool* WorkplacePool = Cast<AMS_WorkpPlacePool>(UGameplayStatics::GetActorOfClass(GetWorld(), AMS_WorkpPlacePool::StaticClass()));
                if (WorkplacePool)
                {
                     WorkplacePool->RemoveWorkplaceAndFreeNode(Workplace);
                     UE_LOG(LogTemp, Log, TEXT("PerformWorkAction: Removed workplace %s via pool."), *Workplace->GetName());
                }
                else UE_LOG(LogTemp, Warning, TEXT("PerformWorkAction: Could not find Workplace Pool manager!"));

                // Stop work montage, play finish montage (optional)
                UAnimInstance* AnimInstance = AICharacter->GetMesh() ? AICharacter->GetMesh()->GetAnimInstance() : nullptr;
                if (AnimInstance)
                {
                    // Stop the specific work montage that was started
                    if (MyMemory->bAnimationStarted && MyMemory->ActiveWorkMontage)
                    {
                        AnimInstance->Montage_Stop(0.1f, MyMemory->ActiveWorkMontage);
                        UE_LOG(LogTemp, Log, TEXT("PerformWorkAction: Stopped montage '%s' for %s."), *GetNameSafe(MyMemory->ActiveWorkMontage), *AICharacter->GetName());
                    }

                }

                CleanupTask(OwnerComp, NodeMemory); // Clears BB key
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("PerformWorkAction: Resource became unavailable before work finished for %s at %s."), *AICharacter->GetName(), *Workplace->GetName());
                CleanupTask(OwnerComp, NodeMemory);
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            }
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PerformWorkAction: AI Character or Workplace became invalid just before finishing. Aborting."));
			CleanupTask(OwnerComp, NodeMemory);
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
	}
}

EBTNodeResult::Type UMS_PerformWorkAction::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogTemp, Log, TEXT("PerformWorkAction: Task aborted."));
	CleanupTask(OwnerComp, NodeMemory);
	return EBTNodeResult::Aborted;
}

void UMS_PerformWorkAction::CleanupTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FMSPerformWorkActionMemory* MyMemory = CastInstanceNodeMemory<FMSPerformWorkActionMemory>(NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();

	// Stop the specific animation that was started, if any
	if (MyMemory->bAnimationStarted && MyMemory->ActiveWorkMontage)
	{
		if (AMS_AICharacter* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr)
		{
			UAnimInstance* AnimInstance = AICharacter->GetMesh() ? AICharacter->GetMesh()->GetAnimInstance() : nullptr;
			// Check if the specific montage is still playing before stopping
			if (AnimInstance && AnimInstance->Montage_IsPlaying(MyMemory->ActiveWorkMontage))
			{
				AnimInstance->Montage_Stop(0.2f, MyMemory->ActiveWorkMontage);
                UE_LOG(LogTemp, Log, TEXT("PerformWorkAction: Cleaned up montage '%s' for %s."), *GetNameSafe(MyMemory->ActiveWorkMontage), *AICharacter->GetName());
			}
		}
	}
    // Reset memory state regardless
    MyMemory->bAnimationStarted = false;
    MyMemory->ActiveWorkMontage = nullptr;


	// Clear the blackboard key
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	// Ensure the key selector is valid and the blackboard exists before clearing
	if (Blackboard && BlackboardKey_IsAtLocation.IsSet() && BlackboardKey_IsAtLocation.SelectedKeyName != NAME_None)
	{
        // Directly clear the value. If the key doesn't exist on this specific Blackboard instance,
        // this won't cause a crash, though it might log a warning internally in the engine if verbose.
        // A more robust check would involve getting the key ID first, but direct clearing is common.
		Blackboard->ClearValue(BlackboardKey_IsAtLocation.SelectedKeyName); // Use ClearValue instead of SetValueAsBool(false)
        UE_LOG(LogTemp, Log, TEXT("PerformWorkAction: Cleared BB Key '%s'."), *BlackboardKey_IsAtLocation.SelectedKeyName.ToString());
	}
    else if (BlackboardKey_IsAtLocation.SelectedKeyName != NAME_None)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformWorkAction: Blackboard key '%s' was not set or Blackboard invalid during cleanup."), *BlackboardKey_IsAtLocation.SelectedKeyName.ToString());
    }


    // Clear weak pointer in memory
    MyMemory->WorkplaceActor = nullptr;
}