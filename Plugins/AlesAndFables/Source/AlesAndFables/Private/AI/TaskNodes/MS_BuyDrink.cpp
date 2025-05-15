
#include "AI/TaskNodes/MS_BuyDrink.h" 
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AI/Characters/MS_AICharacter.h" 
#include "Placeables/Buildings/MS_Tavern.h"   
#include "Systems/MS_PawnStatComponent.h"  
#include "Animation/AnimInstance.h"

UMS_BuyDrink::UMS_BuyDrink()
{
	NodeName = "Buy Drink At Tavern";
	bNotifyTick = true; 

	BlackboardKey_TargetTavern.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_BuyDrink, BlackboardKey_TargetTavern), AActor::StaticClass());
    INIT_TASK_NODE_NOTIFY_FLAGS();
}

uint16 UMS_BuyDrink::GetInstanceMemorySize() const
{
	return sizeof(FMSBuyDrinkTaskMemory);
}

void UMS_BuyDrink::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
    FMSBuyDrinkTaskMemory* MyMemory = CastInstanceNodeMemory<FMSBuyDrinkTaskMemory>(NodeMemory);
    MyMemory->StartTime = -1.0f;
    MyMemory->ActualDrinkDuration = 0.0f;
    MyMemory->bAnimationStarted = false;
    MyMemory->ActiveDrinkingMontage = nullptr;
    MyMemory->TargetTavern = nullptr;
}

FString UMS_BuyDrink::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: Interacts with Tavern from '%s'"),
		*Super::GetStaticDescription(), *BlackboardKey_TargetTavern.SelectedKeyName.ToString());
}

EBTNodeResult::Type UMS_BuyDrink::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	AMS_AICharacter* AIChar = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
    FMSBuyDrinkTaskMemory* MyMemory = CastInstanceNodeMemory<FMSBuyDrinkTaskMemory>(NodeMemory);

	if (!Blackboard || !AIChar || !AIChar->PawnStats_ || !MyMemory)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Invalid dependencies for %s."), *GetNodeName(), *GetNameSafe(AIChar));
        return EBTNodeResult::Failed;
    }

	AMS_Tavern* Tavern = Cast<AMS_Tavern>(Blackboard->GetValueAsObject(BlackboardKey_TargetTavern.SelectedKeyName));
	if (!Tavern)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Target Tavern is invalid for %s."), *GetNodeName(), *AIChar->GetName());
		return EBTNodeResult::Failed;
	}

	// Check if AI has enough money
	if (AIChar->Money < Tavern->DrinkCost)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: %s cannot afford drink (Has: %d, Costs: %d)."),
            *GetNodeName(), *AIChar->GetName(), AIChar->Money, Tavern->DrinkCost);
		return EBTNodeResult::Failed; // Cannot afford
	}
	AIChar->SetActorLocation(Tavern->InteractionPoint->GetComponentLocation());
	// Deduct money
	AIChar->Money -= Tavern->DrinkCost;
	UE_LOG(LogTemp, Log, TEXT("%s: %s bought a drink for %d. Remaining money: %d."),
        *GetNodeName(), *AIChar->GetName(), Tavern->DrinkCost, AIChar->Money);

	MyMemory->StartTime = GetWorld()->GetTimeSeconds();
    MyMemory->ActualDrinkDuration = Tavern->DrinkDuration; // Get duration from the pub itself
    MyMemory->bAnimationStarted = false;
    MyMemory->ActiveDrinkingMontage = nullptr;
    MyMemory->TargetTavern = Tavern; // Store weak ref to pub

	// Play drinking animation
	UAnimInstance* AnimInstance = AIChar->GetMesh() ? AIChar->GetMesh()->GetAnimInstance() : nullptr;
	if (DrinkingMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(DrinkingMontage);
        MyMemory->bAnimationStarted = true;
        MyMemory->ActiveDrinkingMontage = DrinkingMontage;
	}

	return EBTNodeResult::InProgress;
}

void UMS_BuyDrink::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FMSBuyDrinkTaskMemory* MyMemory = CastInstanceNodeMemory<FMSBuyDrinkTaskMemory>(NodeMemory);
	AMS_AICharacter* AIChar = OwnerComp.GetAIOwner() ? Cast<AMS_AICharacter>(OwnerComp.GetAIOwner()->GetPawn()) : nullptr;

    if (!AIChar || !AIChar->PawnStats_ || !MyMemory || !MyMemory->TargetTavern.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: Invalid state in TickTask. Aborting."), *GetNodeName());
        CleanupAfterDrinking(OwnerComp, NodeMemory);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

	// Check if drink duration has elapsed
	if (GetWorld()->GetTimeSeconds() - MyMemory->StartTime >= MyMemory->ActualDrinkDuration)
	{
        AMS_Tavern* Tavern = MyMemory->TargetTavern.Get(); // Get pub from cached ref

		AIChar->PawnStats_->ModifyHappiness(Tavern->HappinessGain);
		UE_LOG(LogTemp, Log, TEXT("%s: %s finished drink. Happiness modified by %.1f. Current Happiness: %.1f"),
            *GetNodeName(), *AIChar->GetName(), Tavern->HappinessGain, AIChar->PawnStats_->GetHappiness()); // Assuming Happiness is public for logging

        CleanupAfterDrinking(OwnerComp, NodeMemory);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UMS_BuyDrink::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AMS_AICharacter* AIChar = OwnerComp.GetAIOwner() ? Cast<AMS_AICharacter>(OwnerComp.GetAIOwner()->GetPawn()) : nullptr;
	UE_LOG(LogTemp, Log, TEXT("%s: Task aborted for %s."), *GetNodeName(), *GetNameSafe(AIChar));
    CleanupAfterDrinking(OwnerComp, NodeMemory);
	return EBTNodeResult::Aborted;
}

void UMS_BuyDrink::CleanupAfterDrinking(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FMSBuyDrinkTaskMemory* MyMemory = CastInstanceNodeMemory<FMSBuyDrinkTaskMemory>(NodeMemory);
    AAIController* AIController = OwnerComp.GetAIOwner();
    AMS_AICharacter* AIChar = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;

    if (AIChar && MyMemory->bAnimationStarted && MyMemory->ActiveDrinkingMontage)
    {
        UAnimInstance* AnimInstance = AIChar->GetMesh() ? AIChar->GetMesh()->GetAnimInstance() : nullptr;
        if (AnimInstance && AnimInstance->Montage_IsPlaying(MyMemory->ActiveDrinkingMontage))
        {
            AnimInstance->Montage_Stop(0.2f, MyMemory->ActiveDrinkingMontage);
        }
    }
    MyMemory->bAnimationStarted = false;
    MyMemory->ActiveDrinkingMontage = nullptr;
    MyMemory->TargetTavern = nullptr; // Clear weak ptr
}