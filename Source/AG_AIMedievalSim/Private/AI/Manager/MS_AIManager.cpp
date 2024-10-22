// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Manager/MS_AIManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMS_AIManager::AMS_AIManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMS_AIManager::BeginPlay()
{
	Super::BeginPlay();
	UWorld* world = GetWorld();
	if (world) {
		AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_ResourceSystem::StaticClass());
		if (FoundActor) {
			ResourceSystem_ = FoundActor;
		}
		else {
			ResourceSystem_ = world->SpawnActor<AMS_ResourceSystem>(AMS_ResourceSystem::StaticClass());
		}
	}
}

// Called every frame
void AMS_AIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

