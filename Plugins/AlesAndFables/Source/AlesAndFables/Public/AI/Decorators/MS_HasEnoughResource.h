
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Systems/MS_InventoryComponent.h"
#include "MS_HasEnoughResource.generated.h"

/**
 * Checks if the AI Character's inventory has at least a specified amount of a particular resource.
 * The resource type and amount can be specified directly or read from Blackboard keys.
 */
UCLASS()
class ALESANDFABLES_API UMS_HasEnoughResource : public UBTDecorator
{
	GENERATED_BODY()

public:
	UMS_HasEnoughResource();

protected:

	UPROPERTY(EditAnywhere, Category = Condition)
	ResourceType ResourceTypeToCheck = ResourceType::ERROR;


	UPROPERTY(EditAnywhere, Category = Blackboard, meta = (DisplayName = "Resource Type (Key)"))
	FBlackboardKeySelector BlackboardKey_ResourceTypeToCheck;


	UPROPERTY(EditAnywhere, Category = Condition, meta = (ClampMin = "0"))
	int32 AmountNeeded = 1;


	UPROPERTY(EditAnywhere, Category = Blackboard, meta = (DisplayName = "Amount Needed (Key)"))
	FBlackboardKeySelector BlackboardKey_AmountNeeded;


	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};