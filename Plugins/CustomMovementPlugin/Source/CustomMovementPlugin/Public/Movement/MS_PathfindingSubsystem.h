// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MS_MovementNodeMeshStarter.h"
#include "MS_PathfindingSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMMOVEMENTPLUGIN_API UMS_PathfindingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected: 

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
public:

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<TSharedPtr<FNode>> FindPath(TSharedPtr<FNode> StartNode, TSharedPtr<FNode> GoalNode);

    TSharedPtr<FNode> FindClosestNodeToActor(AActor* TargetActor);

    TSharedPtr<FNode> FindClosestNodeToPositon(FVector position);

    void SetNodeMap(TMap<FIntPoint, TSharedPtr<FNode>> newNodeMap);

    FIntPoint AddNodeAtPosition(const FVector& Position);

    void SetNodeSeparation(int32 newSeparation);
private:
    TMap<FIntPoint, TSharedPtr<FNode>> NodeMap;

    int32 NodeSeparation_;

};
