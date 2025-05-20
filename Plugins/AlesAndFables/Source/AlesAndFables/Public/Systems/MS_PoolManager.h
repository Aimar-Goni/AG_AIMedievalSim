// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MS_PoolManager.generated.h"

/**
 * 
 */
UCLASS()
class ALESANDFABLES_API UMS_PoolManager : public UObject
{
	GENERATED_BODY()

public:
    void Initialize();
    class AMS_BulletingBoardPool* GetBulletingBoardPool() const;
    class AMS_StorageBuildingPool* GetStorageBuildingPool() const;
    class AMS_WorkpPlacePool* GetWorkPlacePool() const;
private:

    UPROPERTY()
    class AMS_StorageBuildingPool* StorageBuildingPool;

    UPROPERTY()
    class AMS_WorkpPlacePool* WorkPlacePool;
};