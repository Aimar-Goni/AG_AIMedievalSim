
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "MS_MovementNode.generated.h"

USTRUCT()
struct FMoveNode
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Position;

	UPROPERTY()
	FIntPoint GridPosition;

	
	TMap<TSharedPtr<FMoveNode>, bool> Neighbors;
};
static bool bShowDebugLines = false;  // Set to false to disable debug lines

UCLASS()
class  AMS_MovementNode : public AActor
{
	GENERATED_BODY()
	
	FMoveNode node;

public:	
	// Sets default values for this actor's properties
	AMS_MovementNode();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionTrigger;

	UFUNCTION()
	void OnObstacleEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnObstacleExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
