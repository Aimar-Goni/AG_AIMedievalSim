// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Characters/MS_AICharacter.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Placeables/Buildings/MS_BulletingBoardPool.h"
#include "Placeables/Buildings/MS_StorageBuilding.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "Placeables/Buildings/MS_BulletingBoard.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMS_AICharacter::AMS_AICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ShopCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ShopCollision"));
	ShopCollision->SetupAttachment(RootComponent);
	ShopCollision->SetCollisionProfileName(TEXT("Trigger"));

	// Bind the overlap event
	ShopCollision->OnComponentBeginOverlap.AddDynamic(this, &AMS_AICharacter::OnOverlapBegin);

	// Stat Component
	PawnStats_ = CreateDefaultSubobject<UMS_PawnStatComponent>(TEXT("StatsComponent"));

	// Widget Component
	WidgetComponent_ = CreateDefaultSubobject<UWidgetComponent>(TEXT("Stats"));
	WidgetComponent_->SetupAttachment(RootComponent);
	WidgetComponent_->SetWidgetSpace(EWidgetSpace::World);
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClass{ TEXT("/Game/Dynamic/UI/UI_PawnStats") };
	if (WidgetClass.Succeeded())
	{
		WidgetComponent_->SetWidgetClass((WidgetClass.Class));

	}
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

		FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_BulletingBoardPool::StaticClass());
		if (FoundActor) {
			BulletingBoardPool_ = FoundActor;
		}
		else {
			BulletingBoardPool_ = world->SpawnActor<AMS_BulletingBoardPool>(AMS_BulletingBoardPool::StaticClass());
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



void AMS_AICharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{

	// Check if the overlapping actor is your AI character
	if (OtherActor && OtherActor != this)
	{
		AMS_StorageBuilding* StorageBuilding = Cast<AMS_StorageBuilding>(OtherActor);
		if (StorageBuilding)
		{
			AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(this->GetController());
			if (AIController->GetBlackboardComponent()->GetValueAsObject("Target") == StorageBuilding)
			{
				StorageBuilding->ResourceSystem_->SetBerries(Inventory_.Berries_);
				StorageBuilding->ResourceSystem_->SetWood(Inventory_.Wood_);
				Inventory_.ResetInventory();
				UE_LOG(LogTemp, Warning, TEXT("AI Character has entered the storage!"));
			}
		}

		AMS_BulletingBoard* BulletingBoard = Cast<AMS_BulletingBoard>(OtherActor);
		if (BulletingBoard)
		{
			AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(this->GetController());
			if (AIController->GetBlackboardComponent()->GetValueAsObject("Target") == BulletingBoard)
			{
				UE_LOG(LogTemp, Warning, TEXT("AI Character has entered the billboard!"));
				Quest_.Type = static_cast<ResourceType>(FMath::RandRange(0, 1));
				//Quest_.Type = ResourceType::BERRIES;
				Quest_.Amount = FMath::RandRange(1, 15);
				//Quest_.Amount = 6;
			}
		}
		
		AMS_BaseWorkPlace* WorkPlace = Cast<AMS_BaseWorkPlace>(OtherActor);
		if (WorkPlace)
		{
			AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(this->GetController());
			if (AIController->GetBlackboardComponent()->GetValueAsObject("Target") == WorkPlace && WorkPlace->ResourceAvaliable_)
			{
				FResource recieved = WorkPlace->TakeResources();

				switch (recieved.Type)
				{
				case ResourceType::BERRIES:
					Inventory_.Berries_ += recieved.Amount;
					break;
				case ResourceType::WOOD:
					Inventory_.Wood_ += recieved.Amount;
					break;
					/*	case ResourceType::WHEAT:
							Inventory_.Wheat_ += recieved.Amount;
							break;*/
				default:
					break;
				}

				UE_LOG(LogTemp, Warning, TEXT("AI Character has entered the workplace!"));
			
			}
		}
	}
}