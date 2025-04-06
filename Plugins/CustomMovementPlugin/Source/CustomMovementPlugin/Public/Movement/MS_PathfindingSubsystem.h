// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MS_MovementNodeMeshStarter.h"
#include "MS_PathfindingSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPathUpdated, FIntPoint, ChangedNodePos);
static bool bShowDebugLinesPathfinding = false;  // Set to false to disable debug lines

UCLASS()
class CUSTOMMOVEMENTPLUGIN_API UMS_PathfindingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    TArray<FIntPoint> FindPathPoints(TSharedPtr<FMoveNode> StartNode, TSharedPtr<FMoveNode> GoalNode);
    TArray<TSharedPtr<FMoveNode>> FindPathNodes(TSharedPtr<FMoveNode> StartNode, TSharedPtr<FMoveNode> GoalNode);

    TSharedPtr<FMoveNode> FindNodeByGridPosition(const FIntPoint& GridPosition);
    
    UFUNCTION()
    bool GetRandomFreeNode(FVector& OutLocation, FIntPoint& OutGrid);
    
    TSharedPtr<FMoveNode> FindClosestNodeToActor(AActor* TargetActor);
    TSharedPtr<FMoveNode> FindClosestNodeToPosition(FVector position);

    void SetNodeMap(TMap<FIntPoint, TSharedPtr<FMoveNode>> newNodeMap);

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
    TMap<FIntPoint, TSharedPtr<FMoveNode>> NodeMap;

    int32 NodeSeparation_;

};
