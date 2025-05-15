
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MS_FindNearestTavern.generated.h"

UCLASS()
class ALESANDFABLES_API UMS_FindNearestTavern : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UMS_FindNearestTavern();

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector BlackboardKey_TargetPub; // Output: The found Pub Actor

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};