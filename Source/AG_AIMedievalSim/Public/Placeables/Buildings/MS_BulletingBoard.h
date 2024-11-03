// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Systems/MS_ResourceSystem.h"
#include "MS_BulletingBoard.generated.h"



UCLASS()
class AG_AIMEDIEVALSIM_API AMS_BulletingBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_BulletingBoard();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Quests")
	TArray<FQuest> Quests_;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
