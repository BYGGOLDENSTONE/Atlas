#include "StationIntegrityDebugCommands.h"
#include "../Core/AtlasGameState.h"
#include "../Components/StationIntegrityComponent.h"
#include "Engine/Console.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

void FStationIntegrityDebugCommands::RegisterCommands()
{
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.DamageIntegrity"),
        TEXT("Damage station integrity by specified amount. Usage: Atlas.DamageIntegrity [amount]"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FStationIntegrityDebugCommands::DamageIntegrity),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.SetIntegrity"),
        TEXT("Set station integrity to specific value. Usage: Atlas.SetIntegrity [value]"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FStationIntegrityDebugCommands::SetIntegrity),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.SetIntegrityPercent"),
        TEXT("Set station integrity to specific percentage. Usage: Atlas.SetIntegrityPercent [percent]"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FStationIntegrityDebugCommands::SetIntegrityPercent),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ShowIntegrityStatus"),
        TEXT("Display current station integrity status"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FStationIntegrityDebugCommands::ShowIntegrityStatus),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.TestHighRiskAbility"),
        TEXT("Test high-risk ability integrity cost. Usage: Atlas.TestHighRiskAbility [cost]"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FStationIntegrityDebugCommands::TestHighRiskAbility),
        ECVF_Cheat
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.ResetIntegrity"),
        TEXT("Reset station integrity to maximum"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&FStationIntegrityDebugCommands::ResetIntegrity),
        ECVF_Cheat
    );
}

void FStationIntegrityDebugCommands::DamageIntegrity(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Usage: Atlas.DamageIntegrity [amount]"));
        return;
    }
    
    float DamageAmount = FCString::Atof(*Args[0]);
    
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return;
    }
    
    AAtlasGameState* GameState = AAtlasGameState::GetAtlasGameState(World);
    if (!GameState || !GameState->StationIntegrityComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find Station Integrity Component"));
        return;
    }
    
    GameState->StationIntegrityComponent->ApplyIntegrityDamage(DamageAmount);
    UE_LOG(LogTemp, Warning, TEXT("Applied %f damage to station integrity"), DamageAmount);
}

void FStationIntegrityDebugCommands::SetIntegrity(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Usage: Atlas.SetIntegrity [value]"));
        return;
    }
    
    float NewIntegrity = FCString::Atof(*Args[0]);
    
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return;
    }
    
    AAtlasGameState* GameState = AAtlasGameState::GetAtlasGameState(World);
    if (!GameState || !GameState->StationIntegrityComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find Station Integrity Component"));
        return;
    }
    
    GameState->StationIntegrityComponent->SetIntegrity(NewIntegrity);
    UE_LOG(LogTemp, Warning, TEXT("Set station integrity to %f"), NewIntegrity);
}

void FStationIntegrityDebugCommands::SetIntegrityPercent(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Usage: Atlas.SetIntegrityPercent [percent]"));
        return;
    }
    
    float Percent = FCString::Atof(*Args[0]);
    
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return;
    }
    
    AAtlasGameState* GameState = AAtlasGameState::GetAtlasGameState(World);
    if (!GameState || !GameState->StationIntegrityComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find Station Integrity Component"));
        return;
    }
    
    GameState->StationIntegrityComponent->SetIntegrityPercent(Percent);
    UE_LOG(LogTemp, Warning, TEXT("Set station integrity to %f%%"), Percent);
}

void FStationIntegrityDebugCommands::ShowIntegrityStatus(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return;
    }
    
    AAtlasGameState* GameState = AAtlasGameState::GetAtlasGameState(World);
    if (!GameState || !GameState->StationIntegrityComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find Station Integrity Component"));
        return;
    }
    
    UStationIntegrityComponent* IntegrityComp = GameState->StationIntegrityComponent;
    
    UE_LOG(LogTemp, Warning, TEXT("=== Station Integrity Status ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current: %f / %f"), 
        IntegrityComp->GetCurrentIntegrity(), 
        IntegrityComp->GetMaxIntegrity());
    UE_LOG(LogTemp, Warning, TEXT("Percentage: %f%%"), IntegrityComp->GetIntegrityPercent());
    UE_LOG(LogTemp, Warning, TEXT("Critical: %s"), IntegrityComp->IsIntegrityCritical() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Failed: %s"), IntegrityComp->IsIntegrityFailed() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("=============================="));
}

void FStationIntegrityDebugCommands::TestHighRiskAbility(const TArray<FString>& Args)
{
    float Cost = 20.0f;
    if (Args.Num() >= 1)
    {
        Cost = FCString::Atof(*Args[0]);
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return;
    }
    
    AAtlasGameState* GameState = AAtlasGameState::GetAtlasGameState(World);
    if (!GameState || !GameState->StationIntegrityComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find Station Integrity Component"));
        return;
    }
    
    UStationIntegrityComponent* IntegrityComp = GameState->StationIntegrityComponent;
    
    // Always apply the damage - player takes the risk!
    IntegrityComp->ApplyIntegrityDamage(Cost);
    UE_LOG(LogTemp, Warning, TEXT("Used high-risk ability with %f integrity cost"), Cost);
    UE_LOG(LogTemp, Warning, TEXT("Station integrity now at %f%%"), IntegrityComp->GetIntegrityPercent());
    
    if (IntegrityComp->IsIntegrityFailed())
    {
        UE_LOG(LogTemp, Error, TEXT("STATION DESTROYED! Game Over!"));
    }
    else if (IntegrityComp->IsIntegrityCritical())
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: Station integrity is critical!"));
    }
}

void FStationIntegrityDebugCommands::ResetIntegrity(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return;
    }
    
    AAtlasGameState* GameState = AAtlasGameState::GetAtlasGameState(World);
    if (!GameState || !GameState->StationIntegrityComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find Station Integrity Component"));
        return;
    }
    
    GameState->StationIntegrityComponent->SetIntegrityPercent(100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Reset station integrity to 100%%"));
}