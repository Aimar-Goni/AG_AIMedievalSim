// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MS_SkyController.generated.h"

class ADirectionalLight;
class UMS_TimeSubsystem;
class UCurveFloat; 
class UCurveLinearColor;

UCLASS()
class ALESANDFABLES_API AMS_SkyController : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMS_SkyController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Finds the Time Subsystem */
	void InitializeTimeSubsystem();

	/** Updates the light's properties based on the current time */
	void UpdateSunlight(float CurrentHour);

	/** Calculates the sun's rotation based on the hour */
	FRotator CalculateSunRotation(float CurrentHour) const;

	/** Calculates the sun's intensity based on the hour */
	float CalculateSunIntensity(float CurrentHour) const;

	/** Calculates the sun's color based on the hour */
	FLinearColor CalculateSunColor(float CurrentHour) const;

	/** Reference to the Time Subsystem */
	UPROPERTY()
	TObjectPtr<UMS_TimeSubsystem> TimeSubsystem;

	/** Cached pointer to the directional light component for performance */
	UPROPERTY()
	TObjectPtr<class UDirectionalLightComponent> DirectionalLightComponent;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// --- Configuration ---

	/** The main directional light representing the sun in the level. Assign this in the Editor. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Sky Controller|Setup")
	TObjectPtr<ADirectionalLight> TargetDirectionalLight;

	/** Minimum intensity the light will have (at night). Should be > 0 to avoid pitch black. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Intensity", meta = (ClampMin = "0.0"))
	float MinNightIntensity = 12.0f;

	/** Maximum intensity the light will have (at midday). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Intensity", meta = (ClampMin = "0.0"))
	float MaxDayIntensity = 5.0f;

	/** Color of the light during the day. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Color")
	FLinearColor DayColor = FLinearColor::White;

	/** Color of the light during the night (slightly blueish is common). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Color")
	FLinearColor NightColor = FLinearColor(0.1f, 0.2f, 0.4f);

	/** Rotation of the sun at sunrise (Pitch). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Rotation")
	float SunrisePitch = -15.0f; // Sun slightly above horizon

	/** Rotation of the sun at midday (Pitch - Highest point). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Rotation")
	float MiddayPitch = -90.0f; // Directly overhead

	/** Rotation of the sun at sunset (Pitch). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Rotation")
	float SunsetPitch = -165.0f; // Sun slightly above horizon

	/** Fixed Yaw rotation for the sun's path. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Rotation")
	float FixedSunYaw = 0.0f;

	/** Fixed Roll rotation for the sun's path. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Rotation")
	float FixedSunRoll = 0.0f;

	/** Optional curve to control intensity over the 0-24 hour cycle. If set, overrides Min/Max Intensity interpolation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Advanced Curves")
	TObjectPtr<UCurveFloat> IntensityCurve;

	/** Optional curve to control color over the 0-24 hour cycle. If set, overrides Day/Night Color interpolation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Advanced Curves")
	TObjectPtr<UCurveLinearColor> ColorCurve;

	/** Optional curve to control Pitch rotation over the 0-24 hour cycle. If set, overrides Sunrise/Midday/Sunset interpolation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Controller|Advanced Curves")
	TObjectPtr<UCurveFloat> PitchCurve;


private:
    /** Used to trigger initial update */
    bool bInitialUpdateDone = false;
};