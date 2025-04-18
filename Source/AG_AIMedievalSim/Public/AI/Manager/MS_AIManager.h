// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/MS_ResourceSystem.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Placeables/Buildings/MS_StorageBuilding.h"
#include "Placeables/Buildings/MS_BulletingBoardPool.h"
#include "Placeables/Buildings/MS_BulletingBoard.h"
#include "Systems/MS_InventoryComponent.h"
#include "Systems/MS_TimeSubsystem.h"
#include "MS_AIManager.generated.h"

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_AIManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_AIManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	TWeakObjectPtr<AMS_StorageBuildingPool> StorageBuldingsPool_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	TWeakObjectPtr<AMS_BulletingBoardPool> BulletingBoardPool_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Inventory")
	UInventoryComponent* Inventory_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Quests")
	TArray<FQuest> ActiveQuests_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Quests")
	bool SendingQuests = false;

	UFUNCTION()
	void UpdateResources(ResourceType type, int32 amount);

	UFUNCTION()
	void RemoveQuest(FQuest Quest);

	UFUNCTION()
	void OnBulletingBoardPoolReady();


};
