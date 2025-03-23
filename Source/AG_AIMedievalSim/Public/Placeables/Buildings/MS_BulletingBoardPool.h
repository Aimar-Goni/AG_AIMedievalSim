// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Placeables/Buildings/MS_BulletingBoard.h"
#include "MS_BulletingBoardPool.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBulletingBoardPoolInitialized);

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_BulletingBoardPool : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_BulletingBoardPool();

	UFUNCTION()
	void OnNodeMapInitialized();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	int n_BulletingBoards_;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	TArray< TWeakObjectPtr<AMS_BulletingBoard>> BulletingBoards_;

	void FindBulletingBoardsOnScene();

	FOnBulletingBoardPoolInitialized OnBulletingBoardPoolInitialized;

};
