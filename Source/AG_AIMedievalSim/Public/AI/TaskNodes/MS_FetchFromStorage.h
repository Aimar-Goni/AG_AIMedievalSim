#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MS_FetchFromStorage.generated.h"

/**
 * AI attempts to fetch resources needed for its current (delivery) quest from the Target Storage.
 * Reads: Target (Storage), QuestType, QuestAmount (for trip), QuestTargetDestination (Construction Site).
 * Writes: bIsFetchingConstructionMaterials (false), bIsDeliveringConstructionMaterials (true), Target (to Site).
 */
UCLASS()
class AG_AIMEDIEVALSIM_API UMS_FetchFromStorage : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UMS_FetchFromStorage();

	// Input Keys
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_TargetStorage; 

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_QuestType;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_QuestAmount; 

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_QuestTargetDestination; 

	// State Keys to Update
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_IsFetchingMaterials; 

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_IsDeliveringMaterials; 

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_NewMovementTarget; 


protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};