// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Characters/MS_AICharacter.h"
#include "Components/BoxComponent.h"
#include "Placeables/Buildings/MS_BulletingBoard.h"

// Sets default values
AMS_BulletingBoard::AMS_BulletingBoard()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    ShopCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ShopCollision"));
    ShopCollision->SetupAttachment(RootComponent);
    ShopCollision->SetCollisionProfileName(TEXT("Trigger"));

    // Bind the overlap event
    ShopCollision->OnComponentBeginOverlap.AddDynamic(this, &AMS_BulletingBoard::OnOverlapBegin);

}

// Called when the game starts or when spawned
void AMS_BulletingBoard::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMS_BulletingBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMS_BulletingBoard::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("AI Character has entered the shop!"));
    // Check if the overlapping actor is your AI character
    if (OtherActor && OtherActor != this)
    {
        AMS_AICharacter* AICharacter = Cast<AMS_AICharacter>(OtherActor);
        if (AICharacter)
        {
            AICharacter->OnEnterShop();
        }
    }
}