// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Systems/MS_ResourceSystem.h"
#include "MS_BulletingBoard.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestObtained, FQuest, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestAvaliable);


UCLASS()
class AG_AIMEDIEVALSIM_API AMS_BulletingBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_BulletingBoard();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Quests")
	TArray<FQuest> Quests_;

	UPROPERTY(BlueprintAssignable)
	FOnQuestObtained OnQuestObtained;

	UPROPERTY(BlueprintAssignable)
	FOnQuestAvaliable OnQuestAvaliable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Movement")
	FIntPoint GridPosition_;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool placeActive_ = false;

	FQuest GetQuest();
	void AddQuest(FQuest NewQuest);
};
