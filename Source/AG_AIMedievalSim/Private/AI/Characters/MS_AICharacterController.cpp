// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Characters/MS_AICharacterController.h"
#include "AIController.h"
#include "AI/Characters/MS_AICharacter.h"

AMS_AICharacterController::AMS_AICharacterController() {
    behaviorTree_ = CreateDefaultSubobject<UBehaviorTreeComponent>("behaviorComp_");
    blackboard_ = CreateDefaultSubobject<UBlackboardComponent>("blackboardComp_");


}

void AMS_AICharacterController::OnPossess(APawn* pawn) {
    Super::OnPossess(pawn);

    AMS_AICharacter* ch = Cast<AMS_AICharacter>(pawn);

    if (ch) {
        if (ch->behaviorTree_) {
            if (ch->behaviorTree_->BlackboardAsset) {
                blackboard_->InitializeBlackboard(*ch->behaviorTree_->BlackboardAsset);
                blackboard_->SetValueAsObject("SelfActor", ch);
                blackboard_->SetValueAsObject("Objective", ch->target_);
            }
        }
        behaviorTree_->StartTree(*ch->behaviorTree_);
    }
}

void AMS_AICharacterController::OnUnPossess() {
    behaviorTree_->StopTree();
}