#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "MS_WorkpPlacePool.generated.h"

UCLASS()
class ALESANDFABLES_API AMS_WorkpPlacePool : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_WorkpPlacePool();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float WorkplaceSpawnInterval = 0.5f;
	
	FTimerHandle SpawnTimerHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Workplaces")
	int n_workplaces_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Workplaces")
	bool bSpawning = false;
	
	// Active and Inactive pools

	TArray<TWeakObjectPtr<AMS_BaseWorkPlace>> ActiveWorkplaces_;


	TArray<TWeakObjectPtr<AMS_BaseWorkPlace>> InactiveWorkplaces_;

	void FindWorkplacesOnScene();

	UFUNCTION(BlueprintCallable)
	void DeactivateWorkplace(AMS_BaseWorkPlace* Workplace);

	UFUNCTION(BlueprintCallable)
	void ReactivateWorkplace(AMS_BaseWorkPlace* Workplace, const FVector& NewLocation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Spawn")
	TArray<TSubclassOf<AMS_BaseWorkPlace>> WorkplaceClasses;
	
	UFUNCTION(BlueprintCallable)
	void SpawnWorkplaceAtRandomNode();

	UFUNCTION(BlueprintCallable)
	void RemoveWorkplaceAndFreeNode(AMS_BaseWorkPlace* TargetWorkplace);

};
