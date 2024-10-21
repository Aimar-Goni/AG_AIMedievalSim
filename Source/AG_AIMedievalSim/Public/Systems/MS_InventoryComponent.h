// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MS_InventoryComponent.generated.h"

UENUM(BlueprintType)
enum class ResourceType : uint8 {

	BERRIES UMETA(DisplayName = "Berries"),
	WOOD UMETA(DisplayName = "Wood"),
	WHEAT UMETA(DisplayName = "Wheat"),

};

USTRUCT(BlueprintType)
struct FInventory

{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Food_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Wood_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Pelts_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Berries_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Resources")
	int Wheat_;
};

USTRUCT(BlueprintType)
struct FResource

{
	GENERATED_BODY()
	ResourceType Type;

	int32 Ammount;

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
