
#include "AI/Decorators/MS_BTDecorator_IsNightTime.h" 
#include "Systems/MS_TimeSubsystem.h"
#include "AIController.h"
#include "GameFramework/Actor.h" 
#include "Subsystems/SubsystemBlueprintLibrary.h" 

UMS_BTDecorator_IsNightTime::UMS_BTDecorator_IsNightTime()
{
    NodeName = "Is Night Time";

    bAllowAbortChildNodes = true; 
    FlowAbortMode = EBTFlowAbortMode::Self;
}

bool UMS_BTDecorator_IsNightTime::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    // Try using cached pointer first
    UMS_TimeSubsystem* TimeSubsystem = CachedTimeSubsystem.Get();

    if (!TimeSubsystem)
    {
        // Find Time Subsystem if not cached or invalid
        AActor* OwnerActor = OwnerComp.GetOwner(); // Can be Controller or Pawn
        UWorld* World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
        UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;

        if (GameInstance)
        {
            TimeSubsystem = GameInstance->GetSubsystem<UMS_TimeSubsystem>();
            CachedTimeSubsystem = TimeSubsystem; // Cache it
        }
    }

	if (TimeSubsystem)
	{
		return TimeSubsystem->IsNightTime();
	}

	// Return false if subsystem not found (safer default)
	return false;
}

void UMS_BTDecorator_IsNightTime::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    // Check condition on tick to allow for conditional aborts
	const bool bCondition = CalculateRawConditionValue(OwnerComp, NodeMemory);
	if(this->bIsInversed ? bCondition : !bCondition) 
    {
         OwnerComp.RequestExecution(this); 
    }
}


FString UMS_BTDecorator_IsNightTime::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s"), *Super::GetStaticDescription());
}