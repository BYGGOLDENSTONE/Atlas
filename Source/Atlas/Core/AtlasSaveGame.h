#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Components/RunManagerComponent.h"
#include "AtlasSaveGame.generated.h"

/**
 * Save game object for Atlas that stores persistent progression and current run state
 */
UCLASS()
class ATLAS_API UAtlasSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UAtlasSaveGame();

	// ========================================
	// PERSISTENT PROGRESSION
	// ========================================
	
	/** Rewards equipped at the end of last run (persist between runs) */
	UPROPERTY(VisibleAnywhere, Category = "Progression")
	TArray<FEquippedReward> EquippedRewards;
	
	/** Total number of runs completed */
	UPROPERTY(VisibleAnywhere, Category = "Progression")
	int32 TotalRunsCompleted = 0;
	
	/** Highest room level reached in any run */
	UPROPERTY(VisibleAnywhere, Category = "Progression")
	int32 HighestRoomReached = 0;
	
	/** All rewards that have been unlocked */
	UPROPERTY(VisibleAnywhere, Category = "Progression")
	TMap<FGameplayTag, int32> UnlockedRewards;
	
	/** Currency earned across all runs */
	UPROPERTY(VisibleAnywhere, Category = "Progression")
	int32 TotalCurrency = 0;
	
	// ========================================
	// CURRENT RUN STATE (For Resume)
	// ========================================
	
	/** Current run level (0 if no run in progress) */
	UPROPERTY(VisibleAnywhere, Category = "Current Run")
	int32 CurrentRunLevel = 0;
	
	/** Player's current health */
	UPROPERTY(VisibleAnywhere, Category = "Current Run")
	float CurrentHealth = 100.0f;
	
	/** Station's current integrity */
	UPROPERTY(VisibleAnywhere, Category = "Current Run")
	float StationIntegrity = 100.0f;
	
	/** IDs of rooms completed in current run */
	UPROPERTY(VisibleAnywhere, Category = "Current Run")
	TArray<FName> CompletedRoomIDs;
	
	/** Rewards collected during current run */
	UPROPERTY(VisibleAnywhere, Category = "Current Run")
	TArray<FGameplayTag> CurrentRunRewards;
	
	/** Time elapsed in current run */
	UPROPERTY(VisibleAnywhere, Category = "Current Run")
	float CurrentRunTime = 0.0f;
	
	/** Whether there's a run to resume */
	UPROPERTY(VisibleAnywhere, Category = "Current Run")
	bool bHasRunInProgress = false;
	
	// ========================================
	// STATISTICS
	// ========================================
	
	/** Total enemies defeated across all runs */
	UPROPERTY(VisibleAnywhere, Category = "Statistics")
	int32 TotalEnemiesDefeated = 0;
	
	/** Total damage dealt across all runs */
	UPROPERTY(VisibleAnywhere, Category = "Statistics")
	float TotalDamageDealt = 0.0f;
	
	/** Total damage taken across all runs */
	UPROPERTY(VisibleAnywhere, Category = "Statistics")
	float TotalDamageTaken = 0.0f;
	
	/** Number of perfect parries performed */
	UPROPERTY(VisibleAnywhere, Category = "Statistics")
	int32 PerfectParries = 0;
	
	/** Number of successful blocks */
	UPROPERTY(VisibleAnywhere, Category = "Statistics")
	int32 SuccessfulBlocks = 0;
	
	/** Number of times died */
	UPROPERTY(VisibleAnywhere, Category = "Statistics")
	int32 TotalDeaths = 0;
	
	/** Number of times station was destroyed */
	UPROPERTY(VisibleAnywhere, Category = "Statistics")
	int32 StationDestructions = 0;
	
	/** Favorite reward (most equipped) */
	UPROPERTY(VisibleAnywhere, Category = "Statistics")
	FGameplayTag FavoriteReward;
	
	/** Best run record (rooms completed) */
	UPROPERTY(VisibleAnywhere, Category = "Statistics")
	int32 BestRunRooms = 0;
	
	/** Fastest run completion time */
	UPROPERTY(VisibleAnywhere, Category = "Statistics")
	float FastestRunTime = 999999.0f;
	
	// ========================================
	// SETTINGS & PREFERENCES
	// ========================================
	
	/** Player's preferred difficulty */
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	int32 PreferredDifficulty = 1;
	
	/** Audio settings */
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	float MasterVolume = 1.0f;
	
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	float MusicVolume = 0.8f;
	
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	float SFXVolume = 1.0f;
	
	/** Control preferences */
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	bool bInvertYAxis = false;
	
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	float MouseSensitivity = 1.0f;
	
	// ========================================
	// ACHIEVEMENTS & UNLOCKS
	// ========================================
	
	/** Achievement flags */
	UPROPERTY(VisibleAnywhere, Category = "Achievements")
	TMap<FName, bool> AchievementFlags;
	
	/** Unlocked room themes */
	UPROPERTY(VisibleAnywhere, Category = "Unlocks")
	TArray<FName> UnlockedRooms;
	
	/** Unlocked enemy types */
	UPROPERTY(VisibleAnywhere, Category = "Unlocks")
	TArray<FName> UnlockedEnemies;
	
	// ========================================
	// VERSION & VALIDATION
	// ========================================
	
	/** Save game version for compatibility */
	UPROPERTY(VisibleAnywhere, Category = "Version")
	int32 SaveGameVersion = 1;
	
	/** Timestamp of last save */
	UPROPERTY(VisibleAnywhere, Category = "Version")
	FDateTime LastSaveTime;
	
	/** Checksum for save validation */
	UPROPERTY(VisibleAnywhere, Category = "Version")
	uint32 SaveChecksum = 0;
	
	// ========================================
	// HELPER FUNCTIONS
	// ========================================
	
	/**
	 * Update statistics with run data
	 * @param RunData The run progress to add to statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	void UpdateStatistics(const FRunProgressData& RunData);
	
	/**
	 * Clear current run data (keep persistent progression)
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	void ClearCurrentRun();
	
	/**
	 * Validate save data integrity
	 * @return True if save data is valid
	 */
	UFUNCTION(BlueprintPure, Category = "Save Game")
	bool ValidateSaveData() const;
	
	/**
	 * Calculate checksum for validation
	 * @return Calculated checksum
	 */
	uint32 CalculateChecksum() const;
	
	/**
	 * Get formatted play time
	 * @return Play time as formatted string
	 */
	UFUNCTION(BlueprintPure, Category = "Save Game")
	FString GetFormattedPlayTime() const;
};