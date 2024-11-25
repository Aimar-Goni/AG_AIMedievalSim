// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MS_MovementNodeMeshStarter.h"
#include "MS_PathfindingSubsyste.generated.h"

/**
 * 
 */
UCLASS()
class AG_AIMEDIEVALSIM_API UMS_PathfindingSubsyste : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    TArray<FNode*> FindPath(FNode* StartNode, FNode* GoalNode);

    FNode* FindClosestNodeToActor(AActor* TargetActor);

    void SetNodeMap(TMap<FIntPoint, FNode*> newNodeMap);

    void AddNodeAtPosition(const FVector& Position);

    void SetNodeSeparation(int32 newSeparation);
private:
    TMap<FIntPoint, FNode*> NodeMap;

    int32 NodeSeparation_;

};
