// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h" 
#include "Engine/SkyLight.h"
#include "MS_TimeSubsystem.generated.h"

// Delegate signatures
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDayStartDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNightStartDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHourChangedDelegate, int32, NewHour);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayChangedDelegate, int32, NewDay);


/**
 * Manages the in-game time, day/night cycle, and related events.
 */
UCLASS()
class AG_AIMEDIEVALSIM_API UMS_TimeSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UMS_TimeSubsystem();


	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;


	
	// --- Debugging ---

	/** If true, displays current time information on the screen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time|Debug")
	bool bEnableOnScreenDebug = true;

	// --- Time Properties ---
	/** How fast time progresses. 1.0 = real time, 60.0 = 1 real second is 1 game minute, 3600.0 = 1 real second is 1 game hour. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0"))
	float TimeScale = 60.0f; // Default: 1 real minute = 1 game hour

	/** The current hour (0.0 to 23.59...). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	float CurrentHour = 18.0f; // Start at 8 AM

	/** The current day number. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	int32 CurrentDay = 1;

	// --- Day/Night Cycle ---

	/** The hour when day begins (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time|Cycle", meta = (ClampMin = "0.0", ClampMax = "23.99"))
	float DayStartHour = 6.0f;

	/** The hour when night begins (inclusive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time|Cycle", meta = (ClampMin = "0.0", ClampMax = "23.99"))
	float NightStartHour = 20.0f;

	/** Checks if it's currently considered day time. */
	UFUNCTION(BlueprintPure, Category = "Time|Cycle")
	bool IsDayTime() const;

	/** Checks if it's currently considered night time. */
	UFUNCTION(BlueprintPure, Category = "Time|Cycle")
	bool IsNightTime() const;

	// --- Accessors ---
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetCurrentHour() const { return CurrentHour; }

	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetCurrentDay() const { return CurrentDay; }

	// --- Delegates ---
	/** Broadcasts when the time transitions from night to day. */
	UPROPERTY(BlueprintAssignable, Category = "Time|Events")
	FOnDayStartDelegate OnDayStart;

	/** Broadcasts when the time transitions from day to night. */
	UPROPERTY(BlueprintAssignable, Category = "Time|Events")
	FOnNightStartDelegate OnNightStart;

	/** Broadcasts when the integer part of the hour changes. */
	UPROPERTY(BlueprintAssignable, Category = "Time|Events")
	FOnHourChangedDelegate OnHourChanged;

	/** Broadcasts when the day changes. */
	UPROPERTY(BlueprintAssignable, Category = "Time|Events")
	FOnDayChangedDelegate OnDayChanged;

	// --- Console Commands ---

	/** Toggles the on-screen time debug display. Usage: TimeSystem_ToggleDebug */
	UFUNCTION(Exec)
	void TimeSystem_ToggleDebug();

	/** Sets the time scale. Usage: TimeSystem_SetScale [NewScale] */
	UFUNCTION(Exec)
	void TimeSystem_SetScale(float NewScale);

	/** Sets the current hour (0-23.99...). Usage: TimeSystem_SetHour [NewHour] */
	UFUNCTION(Exec)
	void TimeSystem_SetHour(float NewHour);

	/** Adds hours to the current time. Usage: TimeSystem_AddHours [HoursToAdd] */
	UFUNCTION(Exec)
	void TimeSystem_AddHours(float HoursToAdd);

	/** Sets the current day. Usage: TimeSystem_SetDay [NewDay] */
	UFUNCTION(Exec)
	void TimeSystem_SetDay(int32 NewDay);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lighting")
	ASkyLight* SkyLightActor;
	
private:
	// ... (private members remain the same)
	/** Internal state to track if it was night in the previous tick, used for delegate broadcasting. */
	bool bIsCurrentlyNight = false;

	/** Internal state to track the previous hour integer value for the OnHourChanged delegate */
	int32 PreviousHourInteger = -1;

	/** Checks the current time and broadcasts DayStart/NightStart delegates if the state changed. */
	void UpdateCycleState(bool bForceImmediateBroadcast = false);
};