// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Characters/MS_AICharacterController.h"
#include "AIController.h"
#include "AI/Characters/MS_AICharacter.h"

AMS_AICharacterController::AMS_AICharacterController() {
    behaviorTree_ = CreateDefaultSubobject<UBehaviorTreeComponent>("behaviorComp_");
    blackboard_ = CreateDefaultSubobject<UBlackboardComponent>("blackboardComp_");
    PrimaryActorTick.bCanEverTick = true;


}

void AMS_AICharacterController::OnPossess(APawn* pawn) {
    Super::OnPossess(pawn);

    AMS_AICharacter* ch = Cast<AMS_AICharacter>(pawn);

    // Initialize BB values
    if (ch) {
        if (ch->behaviorTree_) {
            if (ch->behaviorTree_->BlackboardAsset) {
                blackboard_->InitializeBlackboard(*ch->behaviorTree_->BlackboardAsset);
                blackboard_->SetValueAsObject("SelfActor", ch);
               // blackboard_->SetValueAsObject("Target", ch->target_.Get());
                blackboard_->SetValueAsBool("GettingTask", true);
                blackboard_->SetValueAsBool("Working", true);
                blackboard_->SetValueAsBool("DoingTask", false);
                blackboard_->SetValueAsBool("StoringItems", false);
                blackboard_->SetValueAsBool("GettingFood", false);
                blackboard_->SetValueAsBool("GettingWater", false);
                blackboard_->SetValueAsBool("Ignoring", false);

            }
        }
        behaviorTree_->StartTree(*ch->behaviorTree_);
    }
}

void AMS_AICharacterController::OnUnPossess() {
    behaviorTree_->StopTree();
}