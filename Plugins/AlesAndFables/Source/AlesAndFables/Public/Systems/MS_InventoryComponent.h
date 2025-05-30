// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MS_InventoryComponent.generated.h"

UENUM(BlueprintType)
enum class ResourceType : uint8
{
	ERROR       UMETA(DisplayName = "Error"),
	BERRIES     UMETA(DisplayName = "Berries"),
	WOOD        UMETA(DisplayName = "Wood"),
	WATER       UMETA(DisplayName = "Water"),
	WHEAT       UMETA(DisplayName = "Wheat")
};

struct ALESANDFABLES_API FResourceTypeExport
{
	static constexpr auto Type = ResourceType{};
};

USTRUCT(BlueprintType)
struct ALESANDFABLES_API FInventory

{
	GENERATED_BODY()

	
};

USTRUCT(BlueprintType)
struct ALESANDFABLES_API FResource

{
	GENERATED_BODY()
	ResourceType Type;

	int32 Amount;

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, ResourceType, Resource, int32, NewAmount);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ALESANDFABLES_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()



public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	TMap<ResourceType, int32> Resources_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int32 Money;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable)
	FOnResourceChanged OnResourceChanged;

	void SetResource(ResourceType Type, int32 NewAmount);
	int32 GetResource(ResourceType Type);

	void AddToResources(ResourceType Type, int32 NewAmount);
	int32 ExtractFromResources(ResourceType Type, int32 ExtactAmount);

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

