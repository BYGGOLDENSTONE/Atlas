#include "AtlasGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UAtlasGameInstance::TriggerGameOver(EGameOverReason Reason)
{
    if (Reason == EGameOverReason::None)
    {
        return;
    }

    LastGameOverReason = Reason;
    HandleGameOver(Reason);
}

void UAtlasGameInstance::RestartGame()
{
    UWorld* World = GetWorld();
    if (World)
    {
        FString CurrentLevelName = World->GetMapName();
        UGameplayStatics::OpenLevel(World, FName(*CurrentLevelName));
    }
}

void UAtlasGameInstance::HandleGameOver(EGameOverReason Reason)
{
    OnGameOver.Broadcast(Reason);

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC)
    {
        PC->SetPause(true);
        PC->bShowMouseCursor = true;
        PC->SetInputMode(FInputModeUIOnly());
    }

    FString ReasonText;
    switch (Reason)
    {
    case EGameOverReason::PlayerDeath:
        ReasonText = TEXT("You Died");
        break;
    case EGameOverReason::StationIntegrityFailure:
        ReasonText = TEXT("Station Destroyed - Structural Integrity Failed");
        break;
    case EGameOverReason::MissionComplete:
        ReasonText = TEXT("Mission Complete");
        break;
    default:
        ReasonText = TEXT("Game Over");
        break;
    }

    UE_LOG(LogTemp, Warning, TEXT("GAME OVER: %s"), *ReasonText);
}