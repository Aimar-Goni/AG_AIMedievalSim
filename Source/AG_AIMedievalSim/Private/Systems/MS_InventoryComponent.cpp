// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/MS_InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
    Resources_.Add(ResourceType::BERRIES, 0);
    Resources_.Add(ResourceType::WOOD, 0);
    Resources_.Add(ResourceType::WATER, 0);
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UInventoryComponent::AddToResources(ResourceType Type, int32 NewAmount)
{
    if (0 != NewAmount)
    {
        int32& ResourceValue = Resources_.FindOrAdd(Type);
        ResourceValue += NewAmount;

        // Broadcast changes based on the resource type
        OnResourceChanged.Broadcast(Type, ResourceValue);
    }
}

int32 UInventoryComponent::ExtractFromResources(ResourceType Type, int32 ExtactAmount)
{
    if (ExtactAmount < GetResourceAmount(Type))
    {
        int32& ResourceValue = Resources_.FindOrAdd(Type);
        ResourceValue -= ExtactAmount;

        OnResourceChanged.Broadcast(Type, ResourceValue);
        return ExtactAmount;
    }
    return -1;
}


void UInventoryComponent::SetResource(ResourceType Type, int32 NewAmount)
{

    int32& ResourceValue = Resources_.FindOrAdd(Type);
    ResourceValue = NewAmount;

    // Broadcast changes based on the resource type
    OnResourceChanged.Broadcast(Type, ResourceValue);

}

int32 UInventoryComponent::GetResource(ResourceType Type)
{
    return GetResourceAmount(Type);
}

