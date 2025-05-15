// Fill out your copyright notice in the Description page of Project Settings.



#include "Systems/MS_SkyController.h" 
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Systems/MS_TimeSubsystem.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "Logging/LogMacros.h"
#include "Engine/Engine.h"         
#include "EngineUtils.h"
#include "Logging/LogMacros.h"      


AMS_SkyController::AMS_SkyController()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;


}

void AMS_SkyController::BeginPlay()
{
	Super::BeginPlay();

	InitializeTimeSubsystem();
	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
	{
		TargetDirectionalLight = *It;
		break; // Only get the first one
	}
	if (!TargetDirectionalLight)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMS_SkyController: TargetDirectionalLight is not assigned in the editor!"));
	}

	if (TargetDirectionalLight)
	{
		DirectionalLightComponent = Cast<UDirectionalLightComponent>(TargetDirectionalLight->GetLightComponent());
		if(!DirectionalLightComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("AMS_SkyController: Assigned TargetDirectionalLight has no Light Component!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AMS_SkyController: No valid TargetDirectionalLight found or assigned. Sky Controller will not function."));
		SetActorTickEnabled(false); // Disable ticking if no light
		return;
	}

    // Initial update might need to wait a frame for subsystem to be fully ready? Often safer.
    // Or rely on bInitialUpdateDone flag in Tick. Let's use the flag.
}

void AMS_SkyController::InitializeTimeSubsystem()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		TimeSubsystem = GameInstance->GetSubsystem<UMS_TimeSubsystem>();
	}

	if (!TimeSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("AMS_SkyController: Failed to find UMS_TimeSubsystem! Sky Controller will not function."));
		SetActorTickEnabled(false); // Disable ticking if no time system
	}
}


void AMS_SkyController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TimeSubsystem && DirectionalLightComponent)
	{
        // Ensure an update happens on the first valid tick
        if (!bInitialUpdateDone)
        {
             UpdateSunlight(TimeSubsystem->GetCurrentHour());
             bInitialUpdateDone = true;
        }
        else
        {
            // Subsequent ticks only update based on actual time change
            UpdateSunlight(TimeSubsystem->GetCurrentHour());
        }
	}
	else
	{
		// Attempt to re-initialize if something failed in BeginPlay (optional)
		if (!TimeSubsystem) InitializeTimeSubsystem();
		if (!DirectionalLightComponent && TargetDirectionalLight) DirectionalLightComponent = Cast<UDirectionalLightComponent>(TargetDirectionalLight->GetLightComponent());

		// Disable tick if still invalid
		if (!TimeSubsystem || !DirectionalLightComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("AMS_SkyController: Dependencies not met in Tick. Disabling Tick."));
			SetActorTickEnabled(false);
		}
	}
}

void AMS_SkyController::UpdateSunlight(float CurrentHour)
{
	if (!DirectionalLightComponent) return;

	// --- Rotation ---
	FRotator NewRotation = CalculateSunRotation(CurrentHour);
	TargetDirectionalLight->SetActorRotation(NewRotation); // Use the Actor's rotation

	// --- Intensity ---
	float NewIntensity = CalculateSunIntensity(CurrentHour);
	DirectionalLightComponent->SetIntensity(NewIntensity);

	// --- Color ---
	FLinearColor NewColor = CalculateSunColor(CurrentHour);
	DirectionalLightComponent->SetLightColor(NewColor);
}

FRotator AMS_SkyController::CalculateSunRotation(float CurrentHour) const
{
    float CalculatedPitch = 0.0f;

	if (PitchCurve)
	{
		CalculatedPitch = PitchCurve->GetFloatValue(CurrentHour);
	}
    else if(TimeSubsystem) // Fallback to interpolation if no curve
    {
		const float DayStart = TimeSubsystem->DayStartHour;
		const float NightStart = TimeSubsystem->NightStartHour;
        float DayDuration = 0.f;
        bool bDayWraps = (NightStart <= DayStart);

        if(bDayWraps)
        {
            DayDuration = (24.0f - DayStart) + NightStart;
        }
        else
        {
             DayDuration = NightStart - DayStart;
        }

        if (DayDuration <= 0.0f) return FRotator(SunrisePitch, FixedSunYaw, FixedSunRoll); // Avoid division by zero

        float MiddayHour = 0;
        if(bDayWraps)
        {
            MiddayHour = FMath::Fmod(DayStart + DayDuration / 2.0f, 24.0f);
        }
        else
        {
            MiddayHour = DayStart + DayDuration / 2.0f;
        }


		if (TimeSubsystem->IsDayTime())
		{
            float TimeSinceDayStart = 0.f;
            if(bDayWraps && CurrentHour < DayStart) // Handle wrap around case for current time
            {
                 TimeSinceDayStart = (24.0f - DayStart) + CurrentHour;
            }
            else
            {
                 TimeSinceDayStart = CurrentHour - DayStart;
            }

			// Interpolate from Sunrise to Midday, then Midday to Sunset
			if (CurrentHour < MiddayHour || (bDayWraps && CurrentHour >= DayStart && MiddayHour < DayStart)) // Before Midday (Handle wrap around for midday comparison)
			{
				float LerpAlpha = TimeSinceDayStart / (DayDuration / 2.0f);
                CalculatedPitch = FMath::Lerp(SunrisePitch, MiddayPitch, LerpAlpha);
			}
			else // After Midday
			{
                float TimeUntilNightStart = 0.f;
                 if(bDayWraps && CurrentHour >= DayStart) // Wrapped day, after start hour
                 {
                     TimeUntilNightStart = (DayDuration / 2.0f) - ((24.0f - CurrentHour) + NightStart);
                 }
                 else if(bDayWraps && CurrentHour < NightStart) // Wrapped day, early morning
                 {
                     TimeUntilNightStart = (DayDuration / 2.0f) - (NightStart - CurrentHour);
                 }
                 else // Normal day
                 {
                     TimeUntilNightStart = NightStart - CurrentHour;
                 }

                 float LerpAlpha = 1.0f - (TimeUntilNightStart / (DayDuration / 2.0f)); // Reversed Lerp from Midday
				 CalculatedPitch = FMath::Lerp(MiddayPitch, SunsetPitch, LerpAlpha);
			}
            CalculatedPitch = FMath::Clamp(CalculatedPitch, FMath::Min(SunrisePitch, SunsetPitch), MiddayPitch);
		}
		else // Night Time
		{
			// Could interpolate pitch below horizon, or just set to a fixed low value
            // Let's just keep it at the sunrise/sunset pitch during night
            // Check which boundary is closer
            float HoursToDayStart = FMath::Fmod(DayStart - CurrentHour + 24.0f, 24.0f);
            float HoursToNightEnd = FMath::Fmod(CurrentHour - NightStart + 24.0f, 24.0f);
             CalculatedPitch = (HoursToDayStart < HoursToNightEnd) ? SunrisePitch : SunsetPitch;

             // A more complex approach could calculate the "midnight dip" pitch
		}
	}

	return FRotator(CalculatedPitch, FixedSunYaw, FixedSunRoll);
}

float AMS_SkyController::CalculateSunIntensity(float CurrentHour) const
{
	if (IntensityCurve)
	{
		return FMath::Max(0.0f, IntensityCurve->GetFloatValue(CurrentHour)); // Ensure intensity isn't negative
	}
    else if(TimeSubsystem) // Fallback to interpolation
    {
		// Simple blend during transitions (e.g., 1 hour transition)
		const float TransitionDuration = 1.0f;
		const float DayStart = TimeSubsystem->DayStartHour;
		const float NightStart = TimeSubsystem->NightStartHour;

		// Sunrise transition
		float SunriseAlpha = FMath::Clamp((CurrentHour - (DayStart - TransitionDuration / 2.0f)) / TransitionDuration, 0.0f, 1.0f);
		// Sunset transition
		float SunsetAlpha = FMath::Clamp((CurrentHour - (NightStart - TransitionDuration / 2.0f)) / TransitionDuration, 0.0f, 1.0f);

        // Lerp based on transitions
        if (CurrentHour > DayStart && CurrentHour < NightStart) // Most of the day
        {
             if(CurrentHour < DayStart + TransitionDuration / 2.0f) // Sunrise
                 return FMath::Lerp(MinNightIntensity, MaxDayIntensity, SunriseAlpha);
             else if (CurrentHour > NightStart - TransitionDuration / 2.0f) // Sunset
                 return FMath::Lerp(MaxDayIntensity, MinNightIntensity, SunsetAlpha);
             else // Full Day
                 return MaxDayIntensity;
        }
        else // Night
        {
             // Handle transitions crossing midnight if DayStart > NightStart (omitted for simplicity here)
             if(CurrentHour < DayStart + TransitionDuration / 2.0f && CurrentHour > DayStart - TransitionDuration / 2.0f) // Sunrise transition during night->day
                 return FMath::Lerp(MinNightIntensity, MaxDayIntensity, SunriseAlpha);
             else if(CurrentHour > NightStart - TransitionDuration / 2.0f && CurrentHour < NightStart + TransitionDuration / 2.0f) // Sunset transition during day->night
                 return FMath::Lerp(MaxDayIntensity, MinNightIntensity, SunsetAlpha);
             else // Full Night
                return MinNightIntensity;
        }
	}
    return MinNightIntensity; // Default fallback
}

FLinearColor AMS_SkyController::CalculateSunColor(float CurrentHour) const
{
	if (ColorCurve)
	{
		return ColorCurve->GetLinearColorValue(CurrentHour);
	}
    else if (TimeSubsystem)// Fallback to interpolation
    {
       // Similar transition logic as intensity
		const float TransitionDuration = 1.5f; // Color transition can be longer
		const float DayStart = TimeSubsystem->DayStartHour;
		const float NightStart = TimeSubsystem->NightStartHour;

		float SunriseAlpha = FMath::Clamp((CurrentHour - (DayStart - TransitionDuration / 2.0f)) / TransitionDuration, 0.0f, 1.0f);
		float SunsetAlpha = FMath::Clamp((CurrentHour - (NightStart - TransitionDuration / 2.0f)) / TransitionDuration, 0.0f, 1.0f);

        if (CurrentHour > DayStart && CurrentHour < NightStart)
        {
             if(CurrentHour < DayStart + TransitionDuration / 2.0f)
                 return FLinearColor::LerpUsingHSV(NightColor, DayColor, SunriseAlpha);
             else if (CurrentHour > NightStart - TransitionDuration / 2.0f)
                 return FLinearColor::LerpUsingHSV(DayColor, NightColor, SunsetAlpha);
             else
                 return DayColor;
        }
        else
        {
            if(CurrentHour < DayStart + TransitionDuration / 2.0f && CurrentHour > DayStart - TransitionDuration / 2.0f)
                 return FLinearColor::LerpUsingHSV(NightColor, DayColor, SunriseAlpha);
             else if(CurrentHour > NightStart - TransitionDuration / 2.0f && CurrentHour < NightStart + TransitionDuration / 2.0f)
                 return FLinearColor::LerpUsingHSV(DayColor, NightColor, SunsetAlpha);
             else
                return NightColor;
        }
	}
     return NightColor; // Default fallback
}