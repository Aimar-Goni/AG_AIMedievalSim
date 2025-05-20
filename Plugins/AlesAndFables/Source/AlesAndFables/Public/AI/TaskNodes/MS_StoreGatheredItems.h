#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Systems/MS_InventoryComponent.h" // For ResourceType
#include "MS_StoreGatheredItems.generated.h"

UCLASS()
class ALESANDFABLES_API UMS_StoreGatheredItems : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMS_StoreGatheredItems();

	// Input Keys
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_TargetStorage;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_QuestType; 

	// State Key to Update
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_IsStoringItems;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};