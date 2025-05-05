#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h" 
#include "MS_CopyBlackboardValue.generated.h"

/**
 * Copies the value from one Blackboard key to another.
 * Currently implemented specifically for UObject* values (like Actors).
 */
UCLASS()
class AG_AIMEDIEVALSIM_API UMS_CopyBlackboardValue : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMS_CopyBlackboardValue();

	/** Blackboard key to read the source value FROM. */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector SourceBlackboardKey;

	/** Blackboard key to write the copied value TO. */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector DestinationBlackboardKey;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};
