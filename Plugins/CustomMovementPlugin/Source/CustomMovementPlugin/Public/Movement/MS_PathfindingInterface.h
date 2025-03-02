#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MS_PathfindingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UMS_PathfindingInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for pathfinding functionality that can be used in Blueprints.
 */
class CUSTOMMOVEMENTPLUGIN_API IMS_PathfindingInterface
{
    GENERATED_BODY()

public:
    // Find a path between two nodes
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pathfinding")
    TArray<FVector> FindPath(FVector StartPosition, FVector GoalPosition);

    // Add a node at a specific position
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pathfinding")
    void AddNode(FVector Position);
};