#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Movement/MS_MovementNode.h"
#include "MS_MovementNodeMeshStarter.generated.h"


static bool bShowDebugLinesStarter = false;  // Set to false to disable debug lines


FORCEINLINE uint32 GetTypeHash(const FIntPoint& Point)
{
	return HashCombine(GetTypeHash(Point.X), GetTypeHash(Point.Y));
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNodeMapReady);

UCLASS()
class CUSTOMMOVEMENTPLUGIN_API AMS_MovementNodeMeshStarter : public AActor
{
	GENERATED_BODY()


	TMap<FIntPoint, TSharedPtr<FMoveNode>> NodeMap;

	int32 NodeSeparationX_ = 250, NodeSeparationY_ = 250;
	FTimerHandle PathCheckTimer;
public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNodeMapReady OnNodeMapReady;
	
	UPROPERTY(BlueprintReadOnly)
	bool bNodeMapReady = false;
	
public:	
	// Sets default values for this actor's properties
	AMS_MovementNodeMeshStarter();
	virtual ~AMS_MovementNodeMeshStarter() override;
	
	void GenerateNodes(FVector FirstPos);

	bool PerformRaycastAtPosition(const FVector& Position);

	bool PerformRaycastToPosition(const FVector& Start, const FVector& End);

	void SpawnAgentAtPosition(const FVector& Position);

	void UpdateBlockedPaths();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
