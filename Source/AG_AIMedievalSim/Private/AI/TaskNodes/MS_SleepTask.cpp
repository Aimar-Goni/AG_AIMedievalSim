// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TaskNodes/MS_SleepTask.h"
#include "AI/Characters/MS_AICharacterController.h" 
#include "AI/Characters/MS_AICharacter.h" 
#include "MS_TimeSubsystem.h" 
#include "Systems/MS_PawnStatComponent.h" 
#include "BehaviorTree/BlackboardComponent.h"

UMS_SleepTask::UMS_SleepTask()
{
    NodeName = "Sleep Until Morning";
    bNotifyTick = true; // Latent task
    BlackboardKey_IsSleeping.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UMS_SleepTask, BlackboardKey_IsSleeping));
}

uint16 UMS_SleepTask::GetInstanceMemorySize() const
{
	return sizeof(FMSSleepTaskMemory);
}

FString UMS_SleepTask::GetStaticDescription() const
{
    return FString::Printf(TEXT("%s\nSets '%s' to True"), *Super::GetStaticDescription(), *BlackboardKey_IsSleeping.SelectedKeyName.ToString());
}

EBTNodeResult::Type UMS_SleepTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* AIController = OwnerComp.GetAIOwner();
	auto* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
    auto* Blackboard = OwnerComp.GetBlackboardComponent();
    FMSSleepTaskMemory* MyMemory = CastInstanceNodeMemory<FMSSleepTaskMemory>(NodeMemory);

	if (!AICharacter || !Blackboard || !MyMemory) return EBTNodeResult::Failed;

    // Cache Time Subsystem
    if (!MyMemory->TimeSubsystemCache)
    {
         UGameInstance* GI = AIController->GetWorld() ? AIController->GetWorld()->GetGameInstance() : nullptr;
         MyMemory->TimeSubsystemCache = GI ? GI->GetSubsystem<UMS_TimeSubsystem>() : nullptr;
    }

    if(!MyMemory->TimeSubsystemCache)
    {
        UE_LOG(LogTemp, Error, TEXT("SleepTask: Failed to get Time Subsystem for %s."), *AICharacter->GetName());
        return EBTNodeResult::Failed;
    }

    // Check if it's already daytime - if so, don't even start sleeping.
    if(MyMemory->TimeSubsystemCache->IsDayTime())
    {
        UE_LOG(LogTemp, Log, TEXT("SleepTask: Attempting to sleep during daytime for %s. Failing task."), *AICharacter->GetName());
        return EBTNodeResult::Failed;
    }

    // Set sleeping flag
    Blackboard->SetValueAsBool(BlackboardKey_IsSleeping.SelectedKeyName, true);
    UE_LOG(LogTemp, Log, TEXT("SleepTask: %s going to sleep."), *AICharacter->GetName());

    // TODO: Play sleep animation/pose?
    // UAnimInstance* AnimInstance = AICharacter->GetMesh() ? AICharacter->GetMesh()->GetAnimInstance() : nullptr;
	// if (SleepMontage && AnimInstance) AnimInstance->Montage_Play(SleepMontage, 1.0f);

    return EBTNodeResult::InProgress;
}

void UMS_SleepTask::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FMSSleepTaskMemory* MyMemory = CastInstanceNodeMemory<FMSSleepTaskMemory>(NodeMemory);
    if(!MyMemory || !MyMemory->TimeSubsystemCache)
    {
        UE_LOG(LogTemp, Error, TEXT("SleepTask: Invalid memory or Time Subsystem cache in TickTask. Aborting."));
        Cleanup(OwnerComp, NodeMemory);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Check if it's daytime now
    if (MyMemory->TimeSubsystemCache->IsDayTime())
    {
        auto* AICharacter = Cast<AMS_AICharacter>(OwnerComp.GetAIOwner()->GetPawn());
        UE_LOG(LogTemp, Log, TEXT("SleepTask: Morning has arrived for %s. Waking up."), *GetNameSafe(AICharacter));
        Cleanup(OwnerComp, NodeMemory);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // Restore Energy while sleeping
    auto* AICharacter = Cast<AMS_AICharacter>(OwnerComp.GetAIOwner()->GetPawn());
    if (AICharacter && AICharacter->PawnStats_ && EnergyRestoredPerSecond > 0.0f)
    {
        AICharacter->PawnStats_->ModifyEnergy(EnergyRestoredPerSecond * DeltaSeconds);
    }
}

EBTNodeResult::Type UMS_SleepTask::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
     auto* AICharacter = Cast<AMS_AICharacter>(OwnerComp.GetAIOwner()->GetPawn());
     UE_LOG(LogTemp, Log, TEXT("SleepTask: Aborted for %s."), *GetNameSafe(AICharacter));
     Cleanup(OwnerComp, NodeMemory);
     return EBTNodeResult::Aborted;
}

void UMS_SleepTask::Cleanup(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
     auto* Blackboard = OwnerComp.GetBlackboardComponent();
     if (Blackboard && BlackboardKey_IsSleeping.IsSet())
     {
         Blackboard->ClearValue(BlackboardKey_IsSleeping.SelectedKeyName);
     }
    
     FMSSleepTaskMemory* MyMemory = CastInstanceNodeMemory<FMSSleepTaskMemory>(NodeMemory);
     if(MyMemory) MyMemory->TimeSubsystemCache = nullptr;
}