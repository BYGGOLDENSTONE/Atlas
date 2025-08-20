#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

/**
 * Unified debug command system for Atlas
 * Consolidates all debug commands into a single organized location
 */
class ATLAS_API FAtlasDebugCommands
{
public:
    static void RegisterAllCommands();
    
    // ===== FOCUS MODE COMMANDS =====
    static void ToggleFocusMode(const TArray<FString>& Args);
    static void ShowFocusDebug(const TArray<FString>& Args);
    static void SpawnTestInteractable(const TArray<FString>& Args);
    static void TestInteraction(const TArray<FString>& Args);
    static void SetFocusRange(const TArray<FString>& Args);
    static void ForceUnlockFocus(const TArray<FString>& Args);
    static void ShowFocusInfo(const TArray<FString>& Args);
    static void TestSoftLock(const TArray<FString>& Args);
    static void SimulateVentInteraction(const TArray<FString>& Args);
    static void SimulateValveInteraction(const TArray<FString>& Args);
    
    // ===== VULNERABILITY COMMANDS =====
    static void ApplyStunnedTier();
    static void ApplyCrippledTier();
    static void ApplyExposedTier();
    static void ClearVulnerability();
    static void SimulateCriticalHit();
    static void ShowVulnerabilityInfo();
    static void ToggleDebugIndicators();
    static void SetStunnedMultiplier(const TArray<FString>& Args);
    static void SetCrippledMultiplier(const TArray<FString>& Args);
    static void SetExposedMultiplier(const TArray<FString>& Args);
    
    // ===== STATION INTEGRITY COMMANDS =====
    static void DamageIntegrity(const TArray<FString>& Args);
    static void SetIntegrity(const TArray<FString>& Args);
    static void SetIntegrityPercent(const TArray<FString>& Args);
    static void ShowIntegrityStatus(const TArray<FString>& Args);
    static void TestHighRiskAbility(const TArray<FString>& Args);
    static void ResetIntegrity(const TArray<FString>& Args);
    
    // ===== COMBAT COMMANDS =====
    static void GodMode(const TArray<FString>& Args);
    static void OneHitKill(const TArray<FString>& Args);
    static void InfinitePoise(const TArray<FString>& Args);
    static void ForceStagger(const TArray<FString>& Args);
    static void ResetCombat(const TArray<FString>& Args);
    
    // ===== ABILITY COMMANDS =====
    static void NoCooldowns(const TArray<FString>& Args);
    static void NoIntegrityCost(const TArray<FString>& Args);
    static void UnlockAllAbilities(const TArray<FString>& Args);
    static void TestAbility(const TArray<FString>& Args);
    
    // ===== ACTION MANAGER COMMANDS =====
    static void AssignAction(const TArray<FString>& Args);
    static void ClearSlot(const TArray<FString>& Args);
    static void SwapSlots(const TArray<FString>& Args);
    static void ListActions(const TArray<FString>& Args);
    static void ShowSlots(const TArray<FString>& Args);
    static void ResetSlots(const TArray<FString>& Args);
    
private:
    // Helper functions
    static class AGameCharacterBase* GetTargetCharacter();
    static class UVulnerabilityComponent* GetVulnerabilityComponent();
    static class UStationIntegrityComponent* GetStationIntegrity();
    static class UCombatComponent* GetCombatComponent();
    static class UActionManagerComponent* GetActionManager();
    
    // Debug flags
    static bool bGodModeEnabled;
    static bool bOneHitKillEnabled;
    static bool bInfinitePoiseEnabled;
    static bool bNoCooldownsEnabled;
    static bool bNoIntegrityCostEnabled;
};