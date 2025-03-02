// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MS_MovementNodeMeshStarter.h"
#include "MS_PathfindingInterface.h"
#include "MS_PathfindingSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMMOVEMENTPLUGIN_API UMS_PathfindingSubsystem : public UGameInstanceSubsystem, public IMS_PathfindingInterface
{
	GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    TArray<FNode*> FindPath(FNode* StartNode, FNode* GoalNode);

    FNode* FindClosestNodeToActor(AActor* TargetActor);
    FNode* FindClosestNodeToPosition(FVector position);

    void SetNodeMap(TMap<FIntPoint, FNode*> newNodeMap);

    FIntPoint AddNodeAtPosition(const FVector& Position);

    void SetNodeSeparation(int32 newSeparation);

    virtual TArray<FVector> FindPath_Implementation(FVector StartPosition, FVector GoalPosition) override;
    virtual void AddNode_Implementation(FVector Position) override;

private:
    TMap<FIntPoint, FNode*> NodeMap;

    int32 NodeSeparation_;

};
