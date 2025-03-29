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

    TArray<TSharedPtr<FNode>> FindPath(TSharedPtr<FNode> StartNode, TSharedPtr<FNode> GoalNode);

    TSharedPtr<FNode> FindClosestNodeToActor(AActor* TargetActor);
    TSharedPtr<FNode> FindClosestNodeToPosition(FVector position);

    void SetNodeMap(TMap<FIntPoint, TSharedPtr<FNode>> newNodeMap);

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
    TMap<FIntPoint, TSharedPtr<FNode>> NodeMap;

    int32 NodeSeparation_;

};
