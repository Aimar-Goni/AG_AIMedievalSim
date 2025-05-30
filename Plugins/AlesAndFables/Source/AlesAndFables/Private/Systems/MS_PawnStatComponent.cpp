
#include "Systems/MS_PawnStatComponent.h"
#include "TimerManager.h"

UMS_PawnStatComponent::UMS_PawnStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UMS_PawnStatComponent::BeginPlay()
{
	Super::BeginPlay();

	// Set timer to call DecreaseStats function every second
	GetWorld()->GetTimerManager().SetTimer(StatTimerHandle, this, &UMS_PawnStatComponent::DecreaseStats, 1.0f, true);
}

// Decreases all the stats by the decrease rate each second 
void UMS_PawnStatComponent::DecreaseStats()
{
	ModifyHunger(-HungerDecreaseRate);
	ModifyThirst(-ThirstDecreaseRate);
	ModifyEnergy(-EnergyDecreaseRate);
	ModifyHappiness(-HappinessDecreaseRate);


	Hungry = Hunger < HungryThreshold;
	Thirsty = Thirst < ThirstThreshold;
	Tired = Energy < EnergyThreshold;
	Sad = Happiness < HappinessThreshold;

	OnStateChanged.Broadcast();
}

// Called every frame
void UMS_PawnStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

//General broadcast to notify stat chnages
void UMS_PawnStatComponent::BroadcastStatChange(float& Stat, float Amount, FOnStatChanged& Delegate)
{
	float OldValue = Stat;
	Stat = FMath::Clamp(Stat + Amount, 0.0f, 100.0f);
	if (Stat != OldValue)
	{
		Delegate.Broadcast(Stat);
	}
}

void UMS_PawnStatComponent::ModifyHunger(float Amount)
{
	BroadcastStatChange(Hunger, Amount, OnHungerChanged);
}

void UMS_PawnStatComponent::ModifyThirst(float Amount)
{
	BroadcastStatChange(Thirst, Amount, OnThirstChanged);
}

void UMS_PawnStatComponent::ModifyEnergy(float Amount)
{
	BroadcastStatChange(Energy, Amount, OnEnergyChanged);
}

void UMS_PawnStatComponent::ModifyHappiness(float Amount)
{
	BroadcastStatChange(Happiness, Amount, OnHappinessChanged);
}


float UMS_PawnStatComponent::GetHunger()
{
	return Hunger;
}
float UMS_PawnStatComponent::GetThirst()
{
	return Thirst;
}
float UMS_PawnStatComponent::GetEnergy()
{
	return Energy;
}
float UMS_PawnStatComponent::GetHappiness()
{
	return Happiness;
}



bool UMS_PawnStatComponent::IsHungry() { return Hungry; }

bool UMS_PawnStatComponent::IsThirsty() { return Thirsty; }
				
bool UMS_PawnStatComponent::IsTired() { return Tired; }
	
bool UMS_PawnStatComponent::IsSad() { return Sad; }

