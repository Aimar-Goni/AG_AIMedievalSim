
#include "AI/TaskNodes/MS_FindNearestTavern.h" 
#include "AI/Characters/MS_AICharacter.h"      
#include "Placeables/Buildings/MS_Tavern.h"      
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

UMS_FindNearestTavern::UMS_FindNearestTavern()
{
    NodeName = "Find Nearest Pub";
    BlackboardKey_TargetPub.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FindNearestTavern, BlackboardKey_TargetPub), AActor::StaticClass());
}

EBTNodeResult::Type UMS_FindNearestTavern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    AMS_AICharacter* AIChar = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

    if(!AIChar || !Blackboard) return EBTNodeResult::Failed;

    TArray<AActor*> FoundPubs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMS_Tavern::StaticClass(), FoundPubs);

    if(FoundPubs.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: No Pubs found in the world for %s."), *GetNodeName(), *AIChar->GetName());
        AIChar->PawnStats_->ModifyHappiness(100);
        Blackboard->ClearValue(BlackboardKey_TargetPub.SelectedKeyName);
        return EBTNodeResult::Failed;
    }

    AMS_Tavern* ClosestPub = nullptr;
    float MinDistSq = FLT_MAX;

    for(AActor* PubActor : FoundPubs)
    {
        AMS_Tavern* Pub = Cast<AMS_Tavern>(PubActor);
        if(Pub)
        {
            float DistSq = FVector::DistSquared(AIChar->GetActorLocation(), Pub->GetActorLocation());
            if(DistSq < MinDistSq)
            {
                MinDistSq = DistSq;
                ClosestPub = Pub;
            }
        }
    }

    if(ClosestPub)
    {
        Blackboard->SetValueAsObject(BlackboardKey_TargetPub.SelectedKeyName, ClosestPub);
        // Also set the generic "Target" key for FollowNodePath
        Blackboard->SetValueAsObject(FName("Target"), ClosestPub);
        AIChar->CreateMovementPath(ClosestPub); // Generate path immediately

        UE_LOG(LogTemp, Log, TEXT("%s: %s found nearest pub %s."), *GetNodeName(), *AIChar->GetName(), *ClosestPub->GetName());
        return AIChar->Path_.Num() > 0 ? EBTNodeResult::Succeeded : EBTNodeResult::Failed; // Succeed if path found
    }

    Blackboard->ClearValue(BlackboardKey_TargetPub.SelectedKeyName);
    Blackboard->ClearValue(FName("Target"));
    AIChar->PawnStats_->ModifyHappiness(100);
    return EBTNodeResult::Failed;
}

FString UMS_FindNearestTavern::GetStaticDescription() const
{
    return FString::Printf(TEXT("%s: Sets '%s' to nearest Pub"), *Super::GetStaticDescription(), *BlackboardKey_TargetPub.SelectedKeyName.ToString());
}