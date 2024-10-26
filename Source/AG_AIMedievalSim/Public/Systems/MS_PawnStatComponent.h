// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MS_PawnStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatChanged, float, NewValue);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AG_AIMEDIEVALSIM_API UMS_PawnStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMS_PawnStatComponent();

protected:
	virtual void BeginPlay() override;
	FTimerHandle StatTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float HungerDecreaseRate = 0.1f;
	UPROPERTY(EditAnywhere, Category = "Stats")
	float ThirstDecreaseRate = 0.2f;
	UPROPERTY(EditAnywhere, Category = "Stats")
	float EnergyDecreaseRate = 0.05f;
	UPROPERTY(EditAnywhere, Category = "Stats")
	float HappinessDecreaseRate = 0.03f;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float Hunger = 100.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float Thirst = 100.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float Energy = 100.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float Happiness = 100.0f;

	void DecreaseStats();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Delegates to broadcast stat changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStatChanged OnHungerChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStatChanged OnThirstChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStatChanged OnEnergyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStatChanged OnHappinessChanged;

	// Modify stat methods
	void ModifyHunger(float Amount);
	void ModifyThirst(float Amount);
	void ModifyEnergy(float Amount);
	void ModifyHappiness(float Amount);

private:
	void BroadcastStatChange(float& Stat, float Amount, FOnStatChanged& Delegate);
};