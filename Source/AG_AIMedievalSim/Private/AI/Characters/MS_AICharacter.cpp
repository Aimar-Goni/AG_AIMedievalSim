// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Characters/MS_AICharacter.h"
#include "AI/Characters/MS_AICharacterController.h"
#include "Placeables/Interactables/MS_WorkpPlacePool.h"
#include "Placeables/Interactables/MS_BaseWorkPlace.h"
#include "Placeables/Buildings/MS_StorageBuildingPool.h"
#include "Placeables/Buildings/MS_StorageBuilding.h"
#include "Placeables/Buildings/MS_BulletingBoardPool.h"
#include "Placeables/Buildings/MS_BulletingBoard.h"
#include "Components/BoxComponent.h"
#include "Systems/MS_PawnStatComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMS_AICharacter::AMS_AICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	ShopCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ShopCollision"));
	ShopCollision->SetupAttachment(RootComponent);
	ShopCollision->SetCollisionProfileName(TEXT("Trigger"));

	// Bind the overlap event
	ShopCollision->OnComponentBeginOverlap.AddDynamic(this, &AMS_AICharacter::OnOverlapBegin);

	// Stat Component
	PawnStats_ = CreateDefaultSubobject<UMS_PawnStatComponent>(TEXT("StatsComponent"));

	// Inventory Component
	Inventory_ = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));


	// Widget Component
	WidgetComponent_ = CreateDefaultSubobject<UWidgetComponent>(TEXT("Stats"));
	WidgetComponent_->SetupAttachment(RootComponent);
	WidgetComponent_->SetWidgetSpace(EWidgetSpace::World);
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClass{ TEXT("/Game/Dynamic/UI/UI_PawnStats") };
	if (WidgetClass.Succeeded())
	{
		WidgetComponent_->SetWidgetClass((WidgetClass.Class));

	}

	// Add State Change Delegate
	PawnStats_->OnStateChanged.AddDynamic(this, &AMS_AICharacter::CheckIfHungry);
}

// Called when the game starts or when spawned
void AMS_AICharacter::BeginPlay()
{
	Super::BeginPlay();

	UWorld* world = GetWorld();
	if (world) {

		// Setup Storage Building Pool
		AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_StorageBuildingPool::StaticClass());
		if (FoundActor) {
			StorageBuldingsPool_ = FoundActor;
		}
		else {
			StorageBuldingsPool_ = world->SpawnActor<AMS_StorageBuildingPool>(AMS_StorageBuildingPool::StaticClass());
		}

		// Setup Workplace Pool
		FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_WorkpPlacePool::StaticClass());
		if (FoundActor) {
			WorkPlacesPool_ = FoundActor;
		}
		else {
			WorkPlacesPool_ = world->SpawnActor<AMS_WorkpPlacePool>(AMS_WorkpPlacePool::StaticClass());
		}

		// Setup Bulleting Board Pool
		FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMS_BulletingBoardPool::StaticClass());
		if (FoundActor) {
			BulletingBoardPool_ = FoundActor;
		}
		else {
			BulletingBoardPool_ = world->SpawnActor<AMS_BulletingBoardPool>(AMS_BulletingBoardPool::StaticClass());
		}
		AMS_BulletingBoardPool* Pool = Cast<AMS_BulletingBoardPool>(BulletingBoardPool_);

		// Link delegate to call when a new quest is added
		for (AMS_BulletingBoard* BulletinBoard : Pool->BulletingBoards_)
		{
			BulletinBoard->OnQuestAvaliable.AddDynamic(this, &AMS_AICharacter::NewQuestAdded);
		}
	}


}

// Called every frame
void AMS_AICharacter::Tick(float DeltaTime)
{
	

}

// Called to bind functionality to input
void AMS_AICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// Updates the BB states when called
void AMS_AICharacter::CheckIfHungry() {
	AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(this->GetController());
	if (PawnStats_->IsHungry()) {
		AIController->GetBlackboardComponent()->SetValueAsBool("Working", false);
		AIController->GetBlackboardComponent()->SetValueAsBool("GettingFood", true);
	
	}
	if (PawnStats_->IsThirsty()) {
		AIController->GetBlackboardComponent()->SetValueAsBool("Working", false);
		AIController->GetBlackboardComponent()->SetValueAsBool("GettingWater", true);

	}
	if (!PawnStats_->IsThirsty())
	{
		AIController->GetBlackboardComponent()->SetValueAsBool("Working", true);
		AIController->GetBlackboardComponent()->SetValueAsBool("GettingWater", false);
	}
	if (!PawnStats_->IsHungry())
	{
		AIController->GetBlackboardComponent()->SetValueAsBool("Working", true);
		AIController->GetBlackboardComponent()->SetValueAsBool("GettingFood", false);
	}
}

// Check collision wirh interactable buildings
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

			//Collision with Storage
			AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(this->GetController());
			if (AIController->GetBlackboardComponent()->GetValueAsObject("Target") == StorageBuilding)
			{

				// If it is working, that means he needs to leave resources
				if (AIController->GetBlackboardComponent()->GetValueAsBool("Working")) {
					for (const auto& Resource : Inventory_->Resources_)
					{
						StorageBuilding->Inventory_->AddToResources(Resource.Key, Resource.Value);
					}
					Inventory_->ResetInventory();
					UE_LOG(LogTemp, Warning, TEXT("AI Character has entered the storage!"));
				}
				// If it is Getting food it means it needs to take food
				if (AIController->GetBlackboardComponent()->GetValueAsBool("GettingFood")) {
					// Check if its enough
					if (StorageBuilding->Inventory_->GetResourceAmount(ResourceType::BERRIES) < 20) {
						//Empty inventory
						for (const auto& Resource : Inventory_->Resources_)
						{
							StorageBuilding->Inventory_->AddToResources(Resource.Key, Resource.Value);
						}
						Inventory_->ResetInventory();

						// Change state to ignore everyting and look for food
						AIController->GetBlackboardComponent()->SetValueAsBool("Ignoring", true);
						
						// Force the quest to food
						Quest_ = FQuest(ResourceType::BERRIES, 10);
					}
					else {
						//Take the resources if there is enough
						StorageBuilding->Inventory_->ExtractFromResources(ResourceType::BERRIES, 20);
						this->PawnStats_->ModifyHunger(100);
					}
				}
				// If it is Getting water it means it needs to take water
				if (AIController->GetBlackboardComponent()->GetValueAsBool("GettingWater")) {
					// Check if its enough
					if (StorageBuilding->Inventory_->GetResourceAmount(ResourceType::WATER) < 20) {
						//Empty inventory
						for (const auto& Resource : Inventory_->Resources_)
						{
							StorageBuilding->Inventory_->AddToResources(Resource.Key, Resource.Value);
						}
						Inventory_->ResetInventory();

						// Change state to ignore everyting and look for water
						AIController->GetBlackboardComponent()->SetValueAsBool("Ignoring", true);

						// Force the quest to water
						Quest_ = FQuest(ResourceType::WATER, 10);

					}
					else {
						StorageBuilding->Inventory_->ExtractFromResources(ResourceType::WATER, 20);
						this->PawnStats_->ModifyThirst(100);

					}
				}
	
			}
		}

		// Collision with bulleting board
		AMS_BulletingBoard* BulletingBoard = Cast<AMS_BulletingBoard>(OtherActor);
		if (BulletingBoard)
		{
			AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(this->GetController());
			//Check if its th one it needs to interact with
			if (AIController->GetBlackboardComponent()->GetValueAsObject("Target") == BulletingBoard)
			{
				UE_LOG(LogTemp, Warning, TEXT("AI Character has entered the billboard!"));

				//Get a new quest
				FQuest newQuest = BulletingBoard->GetQuest();
				if (newQuest.Type == ResourceType::ERROR)
				{
					AIController->GetBlackboardComponent()->SetValueAsBool("Working", false);
				}
				else Quest_ = newQuest;
			}
		}
		
		// Collision with workplace
		AMS_BaseWorkPlace* WorkPlace = Cast<AMS_BaseWorkPlace>(OtherActor);
		if (WorkPlace)
		{
			AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(this->GetController());
			if (AIController->GetBlackboardComponent()->GetValueAsObject("Target") == WorkPlace && WorkPlace->ResourceAvaliable_)
			{

				if (!AIController->GetBlackboardComponent()->GetValueAsBool("Ignoring"))
				{
					//Extect resources
					FResource recieved = WorkPlace->TakeResources();
					//Add them to inventory
					Inventory_->Resources_.FindOrAdd(recieved.Type) += recieved.Amount;
				}
				UE_LOG(LogTemp, Warning, TEXT("AI Character has entered the workplace!"));
			
			}
		}
	}
}

// This is for when it gets to the working place but is Ignoring and consumes te resources
void AMS_AICharacter::ConsumeResourceDirectly(ResourceType type, int32 ammount) {
	switch (type)
	{
	case ResourceType::ERROR:
		break;
	case ResourceType::BERRIES:
		this->PawnStats_->ModifyHunger(100);

		break;
	case ResourceType::WOOD:

		break;
	case ResourceType::WATER:
		this->PawnStats_->ModifyThirst(100);

		break;
	default:
		break;
	}

}

//Change state when getting a quest
void AMS_AICharacter:: NewQuestAdded() {
	AMS_AICharacterController* AIController = Cast<AMS_AICharacterController>(this->GetController());
	AIController->GetBlackboardComponent()->SetValueAsBool("Working", true);
}

// Use the pathfinding subsistem to get a path to the objective
TArray<FNode*> AMS_AICharacter::CreateMovementPath(AActor* ClosestWorkplace) {
	UMS_PathfindingSubsystem* PathfindingSubsystem = GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
	if (PathfindingSubsystem)
	{
		FNode* Begin = PathfindingSubsystem->FindClosestNodeToActor(this);
		FNode* End = PathfindingSubsystem->FindClosestNodeToActor(ClosestWorkplace);
		//DrawDebugSphere(GetWorld(), Begin->Position, 100.0f, 16, FColor::Red, false, 5.0f);
		//if(End)
		//DrawDebugSphere(GetWorld(), End->Position, 100.0f, 16, FColor::Green, false, 5.0f);


		Path_ = PathfindingSubsystem->FindPath(Begin, End);
	/*	for (int32 i = 0; i < Path_.Num() - 1; ++i)
		{
			DrawDebugLine(
				GetWorld(),
				Path_[i]->Position,
				Path_[i + 1]->Position,
				FColor::Blue,
				false,
				10.0f,
				0,
				5.0f
			);
		}*/
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PathfindingSubsystem not found."));
	}
	return TArray<FNode*>();
}