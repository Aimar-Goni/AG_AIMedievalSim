
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MS_FindNearestWorkSite.generated.h"

/**
 * 
 */
UCLASS()
class AG_AIMEDIEVALSIM_API UMS_FindNearestWorkSite : public UBTTaskNode
{
	GENERATED_BODY()
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
