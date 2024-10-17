// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MS_BulletingBoardPool.generated.h"

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_BulletingBoardPool : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_BulletingBoardPool();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	int n_BulletingBoards_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	TArray<AActor*> BulletingBoards_;

	void FindBulletingBoardsOnScene();
};
