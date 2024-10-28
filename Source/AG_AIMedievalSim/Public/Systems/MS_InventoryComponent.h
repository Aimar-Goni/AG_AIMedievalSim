// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MS_InventoryComponent.generated.h"

UENUM(BlueprintType)
enum class ResourceType : uint8 {

	BERRIES UMETA(DisplayName = "Berries"),
	WOOD UMETA(DisplayName = "Wood"),
	WATER UMETA(DisplayName = "Water"),
	//WHEAT UMETA(DisplayName = "Wheat"),

};

USTRUCT(BlueprintType)
struct FInventory

{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	TMap<ResourceType, int32> Resources_;

	FInventory()
	{
		Resources_.Add(ResourceType::BERRIES, 0);
		Resources_.Add(ResourceType::WOOD, 0);
		Resources_.Add(ResourceType::WATER, 0);
	}


	void ResetInventory()
	{

		for (auto& Resource : Resources_)
		{
			Resource.Value = 0;
		}
	}

	int32 GetResourceAmount(ResourceType Type) const
	{
		const int32* Amount = Resources_.Find(Type);
		return Amount ? *Amount : 0;
	}
};

USTRUCT(BlueprintType)
struct FResource

{
	GENERATED_BODY()
	ResourceType Type;

	int32 Amount;

};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AG_AIMEDIEVALSIM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
