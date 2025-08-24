#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Data/RoomDataAsset.h"  // For ERoomType
#include "Phase3ConsoleCommands.generated.h"

/**
 * Console commands for testing Phase 3 features
 * All commands are prefixed with "Atlas.Phase3."
 */
UCLASS()
class ATLAS_API UPhase3ConsoleCommands : public UObject
{
	GENERATED_BODY()

public:
	/** Initialize all Phase 3 console commands */
	static void RegisterCommands();
	
	/** Unregister commands on shutdown */
	static void UnregisterCommands();
	
	// ========================================
	// REWARD SYSTEM COMMANDS
	// ========================================
	
	/** List all available rewards */
	static void ListAllRewards(const TArray<FString>& Args);
	
	/** Give player a specific reward */
	static void GiveReward(const TArray<FString>& Args);
	
	/** Present reward selection UI with specific rewards */
	static void PresentRewardChoice(const TArray<FString>& Args);
	
	/** Equip reward to specific slot */
	static void EquipRewardToSlot(const TArray<FString>& Args);
	
	/** Enhance existing reward */
	static void EnhanceReward(const TArray<FString>& Args);
	
	/** Clear specific slot */
	static void ClearSlot(const TArray<FString>& Args);
	
	/** Clear all slots */
	static void ClearAllSlots(const TArray<FString>& Args);
	
	/** Show current slot status */
	static void ShowSlots(const TArray<FString>& Args);
	
	/** Test reward persistence save */
	static void SaveRewards(const TArray<FString>& Args);
	
	/** Test reward persistence load */
	static void LoadRewards(const TArray<FString>& Args);
	
	// ========================================
	// ROOM SYSTEM COMMANDS
	// ========================================
	
	/** List all rooms */
	static void ListRooms(const TArray<FString>& Args);
	
	/** Start a new run */
	static void StartRun(const TArray<FString>& Args);
	
	/** Complete current room */
	static void CompleteRoom(const TArray<FString>& Args);
	
	/** Fail current room */
	static void FailRoom(const TArray<FString>& Args);
	
	/** Skip to specific room */
	static void SkipToRoom(const TArray<FString>& Args);
	
	/** Show current run progress */
	static void ShowRunProgress(const TArray<FString>& Args);
	
	/** Force specific room sequence */
	static void SetRoomSequence(const TArray<FString>& Args);
	
	// ========================================
	// ROOM TELEPORT COMMANDS
	// ========================================
	
	/** Teleport to specific room */
	static void GoToRoom(const TArray<FString>& Args);
	
	/** Reset all rooms */
	static void ResetRooms(const TArray<FString>& Args);
	
	/** Debug room info */
	static void DebugRooms(const TArray<FString>& Args);
	
	/** Start a complete 5-room run */
	static void StartTestRun(const TArray<FString>& Args);
	
	/** Progress to next room in run */
	static void NextRoom(const TArray<FString>& Args);
	
	/** Complete current room and progress */
	static void CompleteAndProgress(const TArray<FString>& Args);
	
	/** Check if game setup is correct */
	static void CheckSetup(const TArray<FString>& Args);
	
	/** Check enemy status for auto-progression */
	static void CheckEnemyStatus(class UWorld* World);
	
private:
	// Run tracking
	static TArray<ERoomType> CurrentRunOrder;
	static int32 CurrentRunIndex;
	static AActor* CurrentSpawnedEnemy;
	static FTimerHandle EnemyCheckTimer;
	
	// ========================================
	// ENEMY AI COMMANDS
	// ========================================
	
	/** Spawn specific enemy type */
	static void SpawnEnemy(const TArray<FString>& Args);
	
	/** Set enemy difficulty */
	static void SetEnemyDifficulty(const TArray<FString>& Args);
	
	/** Give enemy specific abilities */
	static void GiveEnemyAbility(const TArray<FString>& Args);
	
	/** Show enemy AI state */
	static void ShowEnemyState(const TArray<FString>& Args);
	
	/** Force enemy behavior */
	static void ForceEnemyBehavior(const TArray<FString>& Args);
	
	// ========================================
	// UI TESTING COMMANDS
	// ========================================
	
	/** Show reward selection UI */
	static void ShowRewardSelectionUI(const TArray<FString>& Args);
	
	/** Show slot manager UI */
	static void ShowSlotManagerUI(const TArray<FString>& Args);
	
	/** Show run progress UI */
	static void ShowRunProgressUI(const TArray<FString>& Args);
	
	/** Hide all UI */
	static void HideAllUI(const TArray<FString>& Args);
	
	/** Test UI animations */
	static void TestUIAnimation(const TArray<FString>& Args);
	
	// ========================================
	// STAT TESTING COMMANDS
	// ========================================
	
	/** Modify player stat */
	static void ModifyPlayerStat(const TArray<FString>& Args);
	
	/** Show all player stats */
	static void ShowPlayerStats(const TArray<FString>& Args);
	
	/** Reset all stats to default */
	static void ResetStats(const TArray<FString>& Args);
	
	/** Apply stat modifier temporarily */
	static void ApplyStatModifier(const TArray<FString>& Args);
	
	// ========================================
	// INTERACTABLE COMMANDS
	// ========================================
	
	/** Spawn interactable object */
	static void SpawnInteractable(const TArray<FString>& Args);
	
	/** Trigger interactable */
	static void TriggerInteractable(const TArray<FString>& Args);
	
	/** List nearby interactables */
	static void ListInteractables(const TArray<FString>& Args);
	
	/** Set interactable cooldown */
	static void SetInteractableCooldown(const TArray<FString>& Args);
	
	// ========================================
	// DEBUG & TESTING COMMANDS
	// ========================================
	
	/** Enable Phase 3 debug mode */
	static void EnablePhase3Debug(const TArray<FString>& Args);
	
	/** Run Phase 3 validation tests */
	static void RunPhase3Tests(const TArray<FString>& Args);
	
	/** Simulate full run */
	static void SimulateRun(const TArray<FString>& Args);
	
	/** Test reward balance */
	static void TestRewardBalance(const TArray<FString>& Args);
	
	/** Test enemy scaling */
	static void TestEnemyScaling(const TArray<FString>& Args);
	
	/** Generate Phase 3 report */
	static void GeneratePhase3Report(const TArray<FString>& Args);
	
	/** Quick test specific Phase 3 feature */
	static void QuickTest(const TArray<FString>& Args);
	
private:
	/** Helper to get player character */
	static class AGameCharacterBase* GetPlayerCharacter();
	
	/** Helper to get slot manager component */
	static class USlotManagerComponent* GetSlotManager();
	
	/** Helper to get run manager component */
	static class URunManagerComponent* GetRunManager();
	
	/** Helper to get reward selection component */
	static class URewardSelectionComponent* GetRewardSelection();
	
	/** Helper to find reward by name or tag */
	static class URewardDataAsset* FindReward(const FString& Identifier);
	
	/** Helper to find room by ID */
	static class URoomDataAsset* FindRoom(const FString& RoomID);
	
	/** Print help text for commands */
	static void PrintHelp(const FString& CommandName);
	
	/** Log Phase 3 message */
	static void LogPhase3(const FString& Message, bool bError = false);
};