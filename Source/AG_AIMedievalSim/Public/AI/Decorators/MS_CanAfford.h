#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "MS_CanAfford.generated.h"

UCLASS()
class AG_AIMEDIEVALSIM_API UMS_CanAfford : public UBTDecorator
{
    GENERATED_BODY()
public:
    UMS_CanAfford();


    UPROPERTY(EditAnywhere, Category=Condition)
    int32 LiteralCost = 15;
	
    UPROPERTY(EditAnywhere, Category=Blackboard, meta=(DisplayName="Cost (Key)"))
    FBlackboardKeySelector BlackboardKey_Cost;

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
    virtual FString GetStaticDescription() const override;
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
};


