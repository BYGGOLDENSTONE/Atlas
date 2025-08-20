#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AtlasGameState.generated.h"

class UStationIntegrityComponent;

UCLASS()
class ATLAS_API AAtlasGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AAtlasGameState();

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnIntegrityFailedHandler();
    
    UFUNCTION()
    void OnIntegrityCriticalHandler();

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Station")
    UStationIntegrityComponent* StationIntegrityComponent;

    UFUNCTION(BlueprintCallable, Category = "Station")
    UStationIntegrityComponent* GetStationIntegrityComponent() const { return StationIntegrityComponent; }

    UFUNCTION(BlueprintCallable, Category = "Station", meta = (CallInEditor = "true"))
    static AAtlasGameState* GetAtlasGameState(const UObject* WorldContextObject);
};