// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MS_ResourceSystem.generated.h"

USTRUCT(BlueprintType)
struct FQuest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	ResourceType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int32 Amount;

	bool operator<(const FQuest& Other) const
	{
		return Amount > Other.Amount;
	}

	FQuest(){
		Type = ResourceType::ERROR;
		Amount = 0;
	}
};


UCLASS()
class AG_AIMEDIEVALSIM_API AMS_ResourceSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMS_ResourceSystem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


};
