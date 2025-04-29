#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MS_BTDecorator_NeedToGatherItems.generated.h"

/**
 * Checks if the AI has an assigned quest and needs to gather more resources for it.
 * Succeeds if Inventory < QuestAmount.
 */
UCLASS()
class AG_AIMEDIEVALSIM_API UMS_BTDecorator_NeedToGatherItems : public UBTDecorator
{
	GENERATED_BODY()

public:
	UMS_BTDecorator_NeedToGatherItems();

protected:

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_HasQuest;


	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_QuestType;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_QuestAmount;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

};