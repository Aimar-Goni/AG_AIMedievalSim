#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MS_PathfindingSubsystemLib.generated.h"

UCLASS()
class CUSTOMMOVEMENTPLUGIN_API UMS_PathfindingSubsystemLib : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Blueprint-accessible function to add a node at a position */
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    static void AddNodeVector(FVector Position);

    /** Blueprint-accessible function to find a path */
    UFUNCTION(BlueprintCallable, Category = "Pathfinding", meta = (WorldContext = "WorldContextObject"))
    static TArray<FVector> FindPathVector(const UObject* WorldContextObject, FVector Start, FVector Goal);
};
