// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MS_AICharacterController.generated.h"

/**
 * 
 */
UCLASS()
class ALESANDFABLES_API AMS_AICharacterController : public AAIController
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal|AI")
	class UBehaviorTreeComponent* behaviorTree_;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal|AI")
	class UBlackboardComponent* blackboard_;
private:
    virtual void OnPossess(APawn* pawn) override;
    virtual void OnUnPossess() override;
	AMS_AICharacterController();
};


