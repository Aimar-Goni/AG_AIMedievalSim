
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MS_BuyDrink.generated.h"

class UAnimMontage;
class AMS_Tavern;

// Memory Structure for this task
USTRUCT()
struct ALESANDFABLES_API FMSBuyDrinkTaskMemory
{
	GENERATED_BODY()

	float StartTime = -1.0f;
	float ActualDrinkDuration = 0.0f;
	bool bAnimationStarted = false;
	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveDrinkingMontage = nullptr;
	UPROPERTY() 
	TWeakObjectPtr<AMS_Tavern> TargetTavern;
};


UCLASS()
class ALESANDFABLES_API UMS_BuyDrink : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMS_BuyDrink();

	/** Blackboard key for the Target Tavern actor. */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_TargetTavern;

	/** Animation Montage to play while "drinking". */
	UPROPERTY(EditAnywhere, Category = Animation)
	TObjectPtr<UAnimMontage> DrinkingMontage;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;

	void CleanupAfterDrinking(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
};