
#include "Systems/MS_TimeSubsystem.h"
#include "Engine/Engine.h"         
#include "EngineUtils.h"
#include "Logging/LogMacros.h"      

UMS_TimeSubsystem::UMS_TimeSubsystem()
{
	TimeScale =  600.0f; 
	CurrentHour = 8.0f;
	CurrentDay = 1;
	DayStartHour = 5.0f;
	NightStartHour = 22.0f;
	bIsCurrentlyNight = !IsDayTime(); // Initialize based on start time
	PreviousHourInteger = FMath::FloorToInt(CurrentHour);
	bEnableOnScreenDebug = true; // Enable debug display by default
}

void UMS_TimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("UMS_TimeSubsystem Initialized. Current Time: Day %d, Hour %.2f"), CurrentDay, CurrentHour);
	
	bIsCurrentlyNight = !IsDayTime();
	PreviousHourInteger = FMath::FloorToInt(CurrentHour);
	UpdateCycleState(true); // Force initial broadcast
}

void UMS_TimeSubsystem::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("UMS_TimeSubsystem Deinitialized."));
}

bool UMS_TimeSubsystem::IsTickable() const
{
	return !IsTemplate() && GetWorld() != nullptr && GEngine != nullptr;
}

TStatId UMS_TimeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMS_TimeSubsystem, STATGROUP_Tickables);
}


void UMS_TimeSubsystem::Tick(float DeltaTime)
{
	if (TimeScale <= 0.0f)
	{
        // Optionally display paused message if debugging is enabled
        if (bEnableOnScreenDebug && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow, TEXT("Time Paused"));
        }
		return; // Time is paused or invalid
	}

	// Calculate elapsed game hours
	const float ElapsedGameHours = (DeltaTime * TimeScale) / 3600.0f;

	// Store previous hour before update for comparison
	const float PreviousHourPrecise = CurrentHour;

	// Update current hour
	CurrentHour += ElapsedGameHours;

	// Check for hour change
	const int32 CurrentHourInteger = FMath::FloorToInt(CurrentHour);
	if (CurrentHourInteger != PreviousHourInteger)
	{
		int32 NumHoursPassed = FMath::FloorToInt(CurrentHour) - FMath::FloorToInt(PreviousHourPrecise);

		if (NumHoursPassed > 0) // Only broadcast if hour actually increased
		{
			for(int i = 0; i < NumHoursPassed; ++i)
			{
				int32 BroadcastHour = (PreviousHourInteger + 1 + i) % 24; // Calculate the hour that was passed
				OnHourChanged.Broadcast(BroadcastHour);
				//UE_LOG(LogTemp, Log, TEXT("Hour Changed: %d"), BroadcastHour);

				// Day change broadcast will happen below if CurrentHour >= 24
			}
		}
		PreviousHourInteger = CurrentHourInteger; // Update the tracked integer hour
	}


	// Check for day rollover
	if (CurrentHour >= 24.0f)
	{
		// Calculate how many full days passed (handles very large DeltaTime or TimeScale)
        int32 DaysPassed = FMath::FloorToInt(CurrentHour / 24.0f);
		CurrentDay += DaysPassed;
		CurrentHour = FMath::Fmod(CurrentHour, 24.0f); // Wrap hour back into 0-23.99 range

		// Update integer hour again after wrapping
		PreviousHourInteger = FMath::FloorToInt(CurrentHour);

		// Broadcast day change
		OnDayChanged.Broadcast(CurrentDay);
		UE_LOG(LogTemp, Log, TEXT("New Day Started: Day %d"), CurrentDay);
	}

	// Check and update day/night state
	UpdateCycleState();

	// --- On-Screen Debug Display ---
	if (bEnableOnScreenDebug && GEngine)
	{
		// Display Day and Hour
		GEngine->AddOnScreenDebugMessage(1, // Key: Overwrite message 1
			0.0f,                             // TimeToDisplay (0.0f = 1 frame)
			FColor::Cyan,                     // Color
			FString::Printf(TEXT("Day: %d | Hour: %.0f"), CurrentDay, CurrentHour) // Text
		);

		// Display Time Scale
		GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::Green, FString::Printf(TEXT("Time Scale: %.1fx"), TimeScale));

		// Display Day/Night State
		GEngine->AddOnScreenDebugMessage(3, 0.0f, FColor::Yellow, FString::Printf(TEXT("State: %s"), IsDayTime() ? TEXT("Day") : TEXT("Night")));
	}
}

bool UMS_TimeSubsystem::IsDayTime() const
{
	// Handles the case where DayStartHour might be later than NightStartHour (e.g., polar night/day, though unlikely here)
	if (DayStartHour < NightStartHour)
	{
		// Normal case: Day is between DayStart and NightStart
		return CurrentHour >= DayStartHour && CurrentHour < NightStartHour;
	}
	else // Day wraps around midnight
	{
		return CurrentHour >= DayStartHour || CurrentHour < NightStartHour;
	}
}

bool UMS_TimeSubsystem::IsNightTime() const
{
	return !IsDayTime();
}

void UMS_TimeSubsystem::UpdateCycleState(bool bForceImmediateBroadcast)
{
	bool bShouldBeNight = IsNightTime();

	if (bShouldBeNight != bIsCurrentlyNight || bForceImmediateBroadcast)
	{
		bIsCurrentlyNight = bShouldBeNight;

		if (bIsCurrentlyNight)
		{
			UE_LOG(LogTemp, Log, TEXT("Night Started. Day: %d, Hour: %.2f"), CurrentDay, CurrentHour);
			OnNightStart.Broadcast();
			// for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
			// {
			// 	SkyLightActor = *It;
			// 	break; // Only get the first one
			// }
			// if (SkyLightActor)
			// {
			// 	SkyLightActor->SetActorHiddenInGame(true);
			// 	//SkyLightActor->GetLightComponent()->SetVisibility(false);
			// 	UE_LOG(LogTemp, Log, TEXT("Night started: Skylight disabled"));
			// }
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Day Started. Day: %d, Hour: %.2f"), CurrentDay, CurrentHour);
			OnDayStart.Broadcast();
			// for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
			// {
			// 	SkyLightActor = *It;
			// 	break; // Only get the first one
			// }
			// if (SkyLightActor)
			// {
			// 	SkyLightActor->SetActorHiddenInGame(false);
			// 	//SkyLightActor->GetLightComponent()->SetVisibility(true);
			// 	UE_LOG(LogTemp, Log, TEXT("Day started: Skylight enabled"));
			// }
		}
	}
}


void UMS_TimeSubsystem::TimeSystem_ToggleDebug()
{
	bEnableOnScreenDebug = !bEnableOnScreenDebug;
	UE_LOG(LogTemp, Log, TEXT("Time Subsystem On-Screen Debug: %s"), bEnableOnScreenDebug ? TEXT("Enabled") : TEXT("Disabled"));
	if (!bEnableOnScreenDebug && GEngine)
	{
		// Clear existing messages when disabling
		GEngine->ClearOnScreenDebugMessages();
	}
}

void UMS_TimeSubsystem::TimeSystem_SetScale(float NewScale)
{
	if (NewScale < 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeSystem_SetScale: Scale cannot be negative. Setting to 0."));
		TimeScale = 0.0f;
	}
	else
	{
		TimeScale = NewScale;
	}
	UE_LOG(LogTemp, Log, TEXT("Time Subsystem Scale set to: %.2f"), TimeScale);
}

void UMS_TimeSubsystem::TimeSystem_SetHour(float NewHour)
{
	CurrentHour = FMath::Clamp(NewHour, 0.0f, 23.999f); // Clamp to valid range
	PreviousHourInteger = FMath::FloorToInt(CurrentHour); // Update integer tracking
	UpdateCycleState(true); // Force check for immediate Day/Night state change
	UE_LOG(LogTemp, Log, TEXT("Time Subsystem Hour set to: %.2f"), CurrentHour);
}

void UMS_TimeSubsystem::TimeSystem_AddHours(float HoursToAdd)
{
	// Use Tick logic essentially, but manually applied
	const float PreviousHourForCalc = CurrentHour;
	CurrentHour += HoursToAdd;

	// Re-evaluate hour integer change
	const int32 NewHourInteger = FMath::FloorToInt(CurrentHour);
	if (NewHourInteger != PreviousHourInteger)
	{
		int32 NumHoursPassed = FMath::FloorToInt(CurrentHour) - FMath::FloorToInt(PreviousHourForCalc);
		if (NumHoursPassed > 0) // Only broadcast if hour actually increased
		{
			for(int i = 0; i < NumHoursPassed; ++i)
			{
				int32 BroadcastHour = (PreviousHourInteger + 1 + i) % 24;
				OnHourChanged.Broadcast(BroadcastHour);
				UE_LOG(LogTemp, Log, TEXT("Hour Changed (via AddHours): %d"), BroadcastHour);
			}
		}
	}


	// Handle potential day rollover
	if (CurrentHour >= 24.0f)
	{
        int32 DaysPassed = FMath::FloorToInt(CurrentHour / 24.0f);
		CurrentDay += DaysPassed;
		CurrentHour = FMath::Fmod(CurrentHour, 24.0f);
		OnDayChanged.Broadcast(CurrentDay);
		UE_LOG(LogTemp, Log, TEXT("New Day Started (via AddHours): Day %d"), CurrentDay);
	}
	// Handle negative time wrap-around if needed (more complex, omitted for simplicity)
	else if (CurrentHour < 0.0f)
	{
		int32 DaysToGoBack = FMath::CeilToInt(FMath::Abs(CurrentHour) / 24.0f);
		CurrentDay -= DaysToGoBack;
		CurrentHour = 24.0f - FMath::Fmod(FMath::Abs(CurrentHour), 24.0f);
        if(FMath::IsNearlyZero(FMath::Fmod(FMath::Abs(CurrentHour), 24.0f))) CurrentHour = 0.0f; // Handle exact multiple of 24

		OnDayChanged.Broadcast(CurrentDay); // Broadcast potentially negative day change? Or just clamp day at 1?
		UE_LOG(LogTemp, Log, TEXT("Day Decreased (via AddHours): Day %d"), CurrentDay);
	}

	PreviousHourInteger = FMath::FloorToInt(CurrentHour); // Update integer tracking after wrap/clamp
	UpdateCycleState(true); // Force check for immediate Day/Night state change
	UE_LOG(LogTemp, Log, TEXT("Time Subsystem Time adjusted via AddHours. Current: Day %d, Hour %.2f"), CurrentDay, CurrentHour);
}

void UMS_TimeSubsystem::TimeSystem_SetDay(int32 NewDay)
{
	if (NewDay <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeSystem_SetDay: Day must be positive. Setting to 1."));
		CurrentDay = 1;
	}
	else
	{
		CurrentDay = NewDay;
	}
	OnDayChanged.Broadcast(CurrentDay); // Broadcast the change
	UpdateCycleState(true); // State might not change, but good practice if logic depended on day
	UE_LOG(LogTemp, Log, TEXT("Time Subsystem Day set to: %d"), CurrentDay);
}