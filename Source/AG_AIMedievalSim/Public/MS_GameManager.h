#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MS_GameManager.generated.h"

UCLASS()
class AG_AIMEDIEVALSIM_API UMS_GameManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    class AMS_WorkpPlacePool* GetWorkPlacePool() const;
private:
    UPROPERTY()
    class UMS_PoolManager* PoolManager;

    // UPROPERTY()
    // class UTaskManager* TaskManager;

    void InitGameSystems();
};
