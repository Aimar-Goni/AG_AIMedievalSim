
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MS_MovementNode.generated.h"

struct FNode
{
	FVector Position;
	FIntPoint GridPosition;
	TArray<FNode*> Neighbors;
	float GCost = FLT_MAX;
	float HCost = 0.0f;  
	float FCost() const { return GCost + HCost; }
	FNode* Parent = nullptr;
};
UCLASS()
class AG_AIMEDIEVALSIM_API AMS_MovementNode : public AActor
{
	GENERATED_BODY()
	
	FNode node;

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
};
