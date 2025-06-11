// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/TaskNodes/MS_FindRandomWanderLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Actor.h" 
#include "Movement/MS_PathfindingSubsystem.h" 
#include "AI/Characters/MS_AICharacter.h" 

UMS_FindRandomWanderLocation::UMS_FindRandomWanderLocation()
{
	NodeName = "Find Random Wander Location";
	
	BlackboardKey_TargetActor.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FindRandomWanderLocation, BlackboardKey_TargetActor), AActor::StaticClass());
	BlackboardKey_TargetLocation.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UMS_FindRandomWanderLocation, BlackboardKey_TargetLocation));
}

void UMS_FindRandomWanderLocation::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);
    UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		BlackboardKey_TargetActor.ResolveSelectedKey(*BBAsset);
        BlackboardKey_TargetLocation.ResolveSelectedKey(*BBAsset);
	}
}


EBTNodeResult::Type UMS_FindRandomWanderLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
    AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(ControlledPawn); // Cast to your character
	
	if (!Blackboard || !AIController || !AICharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Task '%s': Invalid Blackboard, Controller, or AICharacter."), *GetNodeName());
		return EBTNodeResult::Failed;
	}
	
    UMS_PathfindingSubsystem* PathSubsystem = CachedPathSubsystem.Get();
    if (!PathSubsystem)
    {
        UGameInstance* GameInstance = AIController->GetWorld() ? AIController->GetWorld()->GetGameInstance() : nullptr;
        if (GameInstance)
        {
            PathSubsystem = GameInstance->GetSubsystem<UMS_PathfindingSubsystem>();
            CachedPathSubsystem = PathSubsystem; // Cache it
        }
    }

	if (!PathSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Task '%s': Pathfinding Subsystem not found! Cannot find wander location for %s."), *GetNodeName(), *AICharacter->GetName());
		return EBTNodeResult::Failed;
	}
	
	FVector RandomLocation = FVector::ZeroVector;
	FIntPoint RandomGridLocation;
	
	if (PathSubsystem->GetRandomFreeNode(RandomLocation, RandomGridLocation))
	{
		UE_LOG(LogTemp, Log, TEXT("Task '%s': Found random wander location for %s at %s (Grid: %s)."),
			*GetNodeName(), *AICharacter->GetName(), *RandomLocation.ToString(), *RandomGridLocation.ToString());
		
		if (BlackboardKey_TargetLocation.IsSet() && BlackboardKey_TargetLocation.SelectedKeyName != NAME_None)
		{
			Blackboard->SetValueAsVector(BlackboardKey_TargetLocation.SelectedKeyName, RandomLocation);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Task '%s': TargetLocation Blackboard Key is not set. Location not written."), *GetNodeName());
		}
		
		if (BlackboardKey_TargetActor.IsSet() && BlackboardKey_TargetActor.SelectedKeyName != NAME_None)
		{
			Blackboard->ClearValue(BlackboardKey_TargetActor.SelectedKeyName);
		}

        if (AICharacter->PathfindingSubsystem) // Use character's cached subsystem
        {
            TSharedPtr<FMoveNode> StartNode = AICharacter->PathfindingSubsystem->FindClosestNodeToActor(AICharacter);
            TSharedPtr<FMoveNode> EndNode = AICharacter->PathfindingSubsystem->FindNodeByGridPosition(RandomGridLocation).Get()->Neighbors.begin().Key(); // Use the grid pos we got
        	
            if(StartNode.IsValid() && EndNode.IsValid())
            {
                AICharacter->Path_ = AICharacter->PathfindingSubsystem->FindPathPoints(StartNode, EndNode);
            	AICharacter->Path_.Add(EndNode->GridPosition); // Add end position to path
                if (AICharacter->Path_.Num() > 0)
                {
                    AICharacter->CurrentNodeIndex = 0; 

                    UE_LOG(LogTemp, Verbose, TEXT("Task '%s': Path generated for wander to %s for %s."), *GetNodeName(), *RandomLocation.ToString(), *AICharacter->GetName());
                    return EBTNodeResult::Succeeded;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Task '%s': Found random location but failed to generate path to it for %s."), *GetNodeName(), *AICharacter->GetName());
                    return EBTNodeResult::Failed;
                }
            }
            else
            {
                 UE_LOG(LogTemp, Warning, TEXT("Task '%s': Could not find valid start/end node for wander path for %s."), *GetNodeName(), *AICharacter->GetName());
                 return EBTNodeResult::Failed;
            }
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("Task '%s': AICharacter's PathfindingSubsystem is null. Cannot generate wander path."), *GetNodeName());
             return EBTNodeResult::Failed;
        }
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Task '%s': Pathfinding Subsystem failed to return a random free node for %s."), *GetNodeName(), *AICharacter->GetName());
		return EBTNodeResult::Failed;
	}
}

FString UMS_FindRandomWanderLocation::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s: Sets '%s' (Vector) to Random Location"),
		*Super::GetStaticDescription(),
		*BlackboardKey_TargetLocation.SelectedKeyName.ToString());
}