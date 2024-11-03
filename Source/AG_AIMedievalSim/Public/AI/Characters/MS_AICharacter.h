// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Systems/MS_InventoryComponent.h"
#include "Systems/MS_PawnStatComponent.h"
#include "Systems/MS_ResourceSystem.h"
#include "MS_AICharacter.generated.h"






UCLASS()
class AG_AIMEDIEVALSIM_API AMS_AICharacter : public ACharacter
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal|AI")
	class UBehaviorTree* behaviorTree_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Behavior")
	AActor* target_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Workplaces")
	AActor* WorkPlacesPool_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	AActor* StorageBuldingsPool_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	AActor* BulletingBoardPool_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Inventory")
	UInventoryComponent* Inventory_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Quests")
	FQuest Quest_;

	UPROPERTY(EditAnywhere, Category = "Collision")
	UBoxComponent* ShopCollision;

	UPROPERTY(EditAnywhere, Category = "Design|Stats")
	UMS_PawnStatComponent* PawnStats_;

	UPROPERTY(EditAnywhere, Category = "Internal|Stats")
	UWidgetComponent* WidgetComponent_;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// Sets default values for this character's properties
	AMS_AICharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void CheckIfHungry();

};
