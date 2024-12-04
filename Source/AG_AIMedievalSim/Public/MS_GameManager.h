// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MS_GameManager.generated.h"

UCLASS()
class AG_AIMEDIEVALSIM_API AMS_GameManager : public AActor
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    class UMS_PoolManager* PoolManager;

    //UPROPERTY()
    //class UTaskManager* TaskManager;


    void Initialize();
};