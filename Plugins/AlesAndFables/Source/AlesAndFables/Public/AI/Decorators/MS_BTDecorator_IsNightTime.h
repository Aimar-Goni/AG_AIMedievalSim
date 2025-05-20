#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "MS_BTDecorator_IsNightTime.generated.h"

class UMS_TimeSubsystem;

/**
 * Checks if it is currently night time according to the Time Subsystem.
 */
UCLASS()
class ALESANDFABLES_API UMS_BTDecorator_IsNightTime : public UBTDecorator
{
	GENERATED_BODY()

public:
	UMS_BTDecorator_IsNightTime();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override; // Needed for conditional abort
	virtual FString GetStaticDescription() const override;
	
 UPROPERTY(EditInstanceOnly, Category=Decorator)
    uint8 bIsInversed : 1;
	
private:
	// Cache the subsystem pointer for performance
	mutable TWeakObjectPtr<UMS_TimeSubsystem> CachedTimeSubsystem;
};