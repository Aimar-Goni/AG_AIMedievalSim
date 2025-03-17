// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MS_MovementNodeMeshStarter.h"
#include "MS_PathfindingSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPathUpdated, FIntPoint, ChangedNodePos);

UCLASS()
class CUSTOMMOVEMENTPLUGIN_API UMS_PathfindingSubsystem : public UGameInstanceSubsystem
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
    UFUNCTION()
    bool PerformRaycastToPosition(const FVector& Start, const FVector& End);

    UFUNCTION()
    void BlockNode(FVector Position);

    UFUNCTION()
    void UnblockNode(FVector Position);

    FOnPathUpdated OnPathUpdated; 

private:
    TMap<FIntPoint, FNode*> NodeMap;

    int32 NodeSeparation_;

};
