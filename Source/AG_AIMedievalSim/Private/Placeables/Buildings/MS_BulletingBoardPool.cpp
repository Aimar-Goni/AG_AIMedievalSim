// Fill out your copyright notice in the Description page of Project Settings.


#include "Placeables/Buildings/MS_BulletingBoardPool.h"
#include "Placeables/Buildings/MS_BulletingBoard.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMS_BulletingBoardPool::AMS_BulletingBoardPool()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMS_BulletingBoardPool::BeginPlay()
{
	Super::BeginPlay();
	FindBulletingBoardsOnScene();
}

// Called every frame
void AMS_BulletingBoardPool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMS_BulletingBoardPool::FindBulletingBoardsOnScene() {
	UWorld* world = GetWorld();
	if (world) {

		TSubclassOf<AMS_BulletingBoard> WorkPlaceClass = AMS_BulletingBoard::StaticClass();
		TArray<AActor*> Workplaces;
		UGameplayStatics::GetAllActorsOfClass(world, WorkPlaceClass, Workplaces);
		for (AActor* actor : Workplaces)
		{
			if (actor)
			{
				BulletingBoards_.Add(actor);
				n_BulletingBoards_++;
			}
		}
	}
}