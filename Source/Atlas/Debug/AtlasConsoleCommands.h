#pragma once

#include "CoreMinimal.h"

/**
 * Essential console commands for Atlas gameplay testing
 */
class ATLAS_API FAtlasConsoleCommands
{
public:
    static void RegisterCommands();
    
private:
    // Core Run Commands
    static void StartRun(const TArray<FString>& Args);
    static void CompleteRoom(const TArray<FString>& Args);
    
    // Player State Commands
    static void SetHealth(const TArray<FString>& Args);
    static void SetIntegrity(const TArray<FString>& Args);
    static void GodMode(const TArray<FString>& Args);
    
    // Action System Commands
    static void AssignAction(const TArray<FString>& Args);
    static void ShowSlots(const TArray<FString>& Args);
    
    // Debug Commands
    static void SpawnEnemy(const TArray<FString>& Args);
    static void KillEnemy(const TArray<FString>& Args);
    static void ShowDebugInfo(const TArray<FString>& Args);
    static void ShowRooms(const TArray<FString>& Args);
    
    // Helper functions
    static class AGameCharacterBase* GetPlayerCharacter();
    
    static bool bGodModeEnabled;
};