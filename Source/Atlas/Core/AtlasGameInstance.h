#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AtlasGameInstance.generated.h"

UENUM(BlueprintType)
enum class EGameOverReason : uint8
{
    None,
    PlayerDeath,
    StationIntegrityFailure,
    MissionComplete
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOver, EGameOverReason, Reason);

UCLASS()
class ATLAS_API UAtlasGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void TriggerGameOver(EGameOverReason Reason);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    void RestartGame();

    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnGameOver OnGameOver;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Game State")
    EGameOverReason LastGameOverReason = EGameOverReason::None;

private:
    void HandleGameOver(EGameOverReason Reason);
};