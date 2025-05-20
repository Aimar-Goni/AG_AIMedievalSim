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

    UFUNCTION(BlueprintPure, Category = "Pathfinding|Nodes")
    bool IsNodeBlocked(const FIntPoint& GridPosition) const;
    
    UFUNCTION(BlueprintCallable, Category = "Pathfinding|Nodes")
    void BlockNode(FVector Position);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding|Nodes")
    void BlockNodeGrid(FIntPoint GridPosition);


    UFUNCTION(BlueprintCallable, Category = "Pathfinding|Nodes")
    void UnblockNode(FVector Position);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding|Nodes")
    void UnblockNodeGrid(FIntPoint GridPosition);
    
    UFUNCTION(BlueprintCallable, Category = "Pathfinding|Nodes")
    bool DeactivateClosestNodes(const FVector& CenterPoint, UPARAM(DisplayName="Deactivated Positions") TArray<FIntPoint>& OutDeactivatedNodePositions, int32 NumNodesToDeactivate = 4);

    
    
    FOnPathUpdated OnPathUpdated; 

private:
    TMap<FIntPoint, TSharedPtr<FMoveNode>> NodeMap;
    
    TSet<FIntPoint> BlockedNodes;

    int32 NodeSeparation_;

    void SetNodeBlockedStatus(const FIntPoint& GridPosition, bool bBlocked);
};
