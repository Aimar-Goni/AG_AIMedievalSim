// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Characters/MS_AICharacter.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMS_AICharacter::AMS_AICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMS_AICharacter::BeginPlay()
{
	Super::BeginPlay();

	UWorld* world = GetWorld();
	if (world) {

		AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_StorageBuildingPool::StaticClass());
		if (FoundActor) {
			StorageBuldingsPool_ = FoundActor;
		}
		else {
			StorageBuldingsPool_ = world->SpawnActor<AMS_StorageBuildingPool>(AMS_StorageBuildingPool::StaticClass());
		}

		FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_WorkpPlacePool::StaticClass());
		if (FoundActor) {
			WorkPlacesPool_ = FoundActor;
		}
		else {
			WorkPlacesPool_ = world->SpawnActor<AMS_WorkpPlacePool>(AMS_WorkpPlacePool::StaticClass());
		}

		FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_BulletingBoard::StaticClass());
		if (FoundActor) {
			BulletingBoardPool_ = FoundActor;
		}
		else {
			BulletingBoardPool_ = world->SpawnActor<AMS_BulletingBoard>(AMS_BulletingBoard::StaticClass());
		}
	}

}

// Called every frame
void AMS_AICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMS_AICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

