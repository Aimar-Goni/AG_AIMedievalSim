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
#include "Movement/MS_PathfindingSubsystem.h"
#include "MS_AICharacter.generated.h"

class AMS_AIManager; 
class UMS_PathfindingSubsystem;
class AMS_House;

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_AICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMS_AICharacter(); 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal|AI")
	TObjectPtr<UBehaviorTree> behaviorTree_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Inventory")
	TObjectPtr<UInventoryComponent> Inventory_;

	UPROPERTY(EditAnywhere, Category = "Design|Stats")
	TObjectPtr<UMS_PawnStatComponent> PawnStats_;

	UPROPERTY(EditAnywhere, Category = "Internal|Stats")
	TObjectPtr<UWidgetComponent> WidgetComponent_;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Dependencies") 
	TWeakObjectPtr<AActor> WorkPlacesPool_; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Dependencies")
	TWeakObjectPtr<AActor> StorageBuldingsPool_; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Storages")
	TWeakObjectPtr<AActor> BulletingBoardPool_;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Design|Dependencies")
    TWeakObjectPtr<AMS_AIManager> AIManager;
	
    UPROPERTY()
    TObjectPtr<UMS_PathfindingSubsystem> PathfindingSubsystem;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Design|Quests")
	FQuest AssignedQuest;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design|Economy")
    int32 Money = 20; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal|Movement")
	int32 CurrentNodeIndex = -1;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal|Movement")
	FVector CurrentTargetLocation;
	TArray<FIntPoint> Path_;
	
	UPROPERTY(EditAnywhere, Category = "Collision")
	TObjectPtr<UBoxComponent> ShopCollision;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI|Housing") // VisibleInstanceOnly: Set by AIManager, not directly in editor per AI
	TWeakObjectPtr<AMS_House> MyHouse;

protected:
	
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category="AI|Housing")
	void SetAssignedHouse(AMS_House* NewHouse);

	UFUNCTION(BlueprintPure, Category="AI|Housing")
	AMS_House* GetAssignedHouse() const;


	UFUNCTION()
	void OnNewQuestReceived(const FQuest& NewQuest);


	UFUNCTION()
	void EvaluateQuestAndBid(const FQuest& Quest);


    UFUNCTION()
    float CalculateBidValue(const FQuest& Quest);


    UFUNCTION()
    void AssignQuest(const FQuest& Quest);
	
    UFUNCTION(BlueprintCallable, Category="AI|Quests")
    void CompleteCurrentQuest();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void CheckIfHungry(); 

	UFUNCTION()
	void ConsumeResourceDirectly(ResourceType type, int32 ammount);

	TArray<FIntPoint> CreateMovementPath(AActor* TargetActor);

	UFUNCTION()
	void OnPathUpdated(FIntPoint ChangedNodePos);

    UFUNCTION(BlueprintPure, Category="AI|State")
    bool IsIdle() const;

	//UFUNCTION()
	//void NewQuestAdded();
};