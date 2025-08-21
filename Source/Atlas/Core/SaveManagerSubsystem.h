#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "SaveManagerSubsystem.generated.h"

// Forward declarations
class UAtlasSaveGame;
class USlotManagerComponent;
class URunManagerComponent;
struct FRunProgressData;
struct FEquippedReward;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveGameLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveGameSaved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveGameFailed, FText, ErrorMessage);

/**
 * Subsystem that manages save game functionality for Atlas.
 * Handles saving/loading rewards, run progress, and player statistics.
 */
UCLASS()
class ATLAS_API USaveManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========================================
	// SUBSYSTEM LIFECYCLE
	// ========================================
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// SAVE/LOAD OPERATIONS
	// ========================================
	
	/**
	 * Save the current game state
	 * @return True if save was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager")
	bool SaveGame();
	
	/**
	 * Load the saved game state
	 * @return True if load was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager")
	bool LoadGame();
	
	/**
	 * Auto-save the game (quick save without UI feedback)
	 * @return True if auto-save was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager")
	bool AutoSave();
	
	/**
	 * Delete the current save game
	 * @return True if deletion was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager")
	bool DeleteSaveGame();
	
	/**
	 * Check if a save game exists
	 * @return True if save file exists
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager")
	bool HasSaveGame() const;
	
	/**
	 * Check if there's a run in progress to resume
	 * @return True if can resume
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager")
	bool CanResumeRun() const;
	
	// ========================================
	// REWARD MANAGEMENT
	// ========================================
	
	/**
	 * Save equipped rewards from slot manager
	 * @param SlotManager The slot manager to save from
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager|Rewards")
	void SaveRewards(USlotManagerComponent* SlotManager);
	
	/**
	 * Load rewards into slot manager
	 * @param SlotManager The slot manager to load into
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager|Rewards")
	void LoadRewards(USlotManagerComponent* SlotManager);
	
	/**
	 * Get saved equipped rewards
	 * @return Array of equipped rewards
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager|Rewards")
	TArray<FEquippedReward> GetSavedRewards() const;
	
	/**
	 * Mark a reward as unlocked
	 * @param RewardTag The reward to unlock
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager|Rewards")
	void UnlockReward(FGameplayTag RewardTag);
	
	/**
	 * Check if a reward is unlocked
	 * @param RewardTag The reward to check
	 * @return True if unlocked
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager|Rewards")
	bool IsRewardUnlocked(FGameplayTag RewardTag) const;
	
	// ========================================
	// RUN PROGRESS
	// ========================================
	
	/**
	 * Save current run progress
	 * @param RunData The run progress to save
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager|Run")
	void SaveRunProgress(const FRunProgressData& RunData);
	
	/**
	 * Load saved run progress
	 * @param OutRunData The loaded run data
	 * @return True if run data was loaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager|Run")
	bool LoadRunProgress(FRunProgressData& OutRunData);
	
	/**
	 * Clear current run from save (after completion/failure)
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager|Run")
	void ClearCurrentRun();
	
	/**
	 * Update run statistics after completion
	 * @param RunData The completed run data
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager|Run")
	void UpdateRunStatistics(const FRunProgressData& RunData);
	
	// ========================================
	// STATISTICS
	// ========================================
	
	/**
	 * Get total runs completed
	 * @return Number of completed runs
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager|Stats")
	int32 GetTotalRunsCompleted() const;
	
	/**
	 * Get highest room reached
	 * @return Highest room level
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager|Stats")
	int32 GetHighestRoomReached() const;
	
	/**
	 * Get total enemies defeated
	 * @return Enemy defeat count
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager|Stats")
	int32 GetTotalEnemiesDefeated() const;
	
	/**
	 * Get best run record
	 * @return Most rooms completed in a run
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager|Stats")
	int32 GetBestRunRecord() const;
	
	/**
	 * Increment a statistic value
	 * @param StatName The stat to increment
	 * @param Amount Amount to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager|Stats")
	void IncrementStat(FName StatName, int32 Amount = 1);
	
	// ========================================
	// SETTINGS
	// ========================================
	
	/**
	 * Save game settings
	 * @param MasterVol Master volume
	 * @param MusicVol Music volume
	 * @param SFXVol SFX volume
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager|Settings")
	void SaveSettings(float MasterVol, float MusicVol, float SFXVol);
	
	/**
	 * Load game settings
	 * @param OutMasterVol Master volume
	 * @param OutMusicVol Music volume
	 * @param OutSFXVol SFX volume
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager|Settings")
	void LoadSettings(float& OutMasterVol, float& OutMusicVol, float& OutSFXVol);
	
	// ========================================
	// ACHIEVEMENTS
	// ========================================
	
	/**
	 * Mark an achievement as completed
	 * @param AchievementName The achievement ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager|Achievements")
	void UnlockAchievement(FName AchievementName);
	
	/**
	 * Check if an achievement is unlocked
	 * @param AchievementName The achievement ID
	 * @return True if unlocked
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager|Achievements")
	bool IsAchievementUnlocked(FName AchievementName) const;
	
	// ========================================
	// UTILITIES
	// ========================================
	
	/**
	 * Get the current save game object
	 * @return Active save game or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager")
	UAtlasSaveGame* GetCurrentSaveGame() const { return CurrentSaveGame; }
	
	/**
	 * Create a new save game object
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager")
	void CreateNewSaveGame();
	
	/**
	 * Validate the current save game
	 * @return True if save is valid
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager")
	bool ValidateSaveGame() const;
	
	/**
	 * Get save game slot name
	 * @return The slot name used for saving
	 */
	UFUNCTION(BlueprintPure, Category = "Save Manager")
	FString GetSaveSlotName() const { return SaveSlotName; }
	
	/**
	 * Export save data to string (for cloud saves, etc.)
	 * @return Serialized save data
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager")
	FString ExportSaveData();
	
	/**
	 * Import save data from string
	 * @param SaveDataString The serialized save data
	 * @return True if import was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Manager")
	bool ImportSaveData(const FString& SaveDataString);
	
	// ========================================
	// EVENTS
	// ========================================
	
	UPROPERTY(BlueprintAssignable, Category = "Save Manager|Events")
	FOnSaveGameLoaded OnSaveGameLoaded;
	
	UPROPERTY(BlueprintAssignable, Category = "Save Manager|Events")
	FOnSaveGameSaved OnSaveGameSaved;
	
	UPROPERTY(BlueprintAssignable, Category = "Save Manager|Events")
	FOnSaveGameFailed OnSaveGameFailed;

protected:
	// ========================================
	// INTERNAL FUNCTIONS
	// ========================================
	
	/** Perform the actual save operation */
	bool PerformSave();
	
	/** Perform the actual load operation */
	bool PerformLoad();
	
	/** Update save game timestamp and checksum */
	void UpdateSaveMetadata();
	
	/** Handle save errors */
	void HandleSaveError(const FText& ErrorMessage);
	
	/** Migrate save data from old versions */
	void MigrateSaveData(int32 OldVersion);

protected:
	// ========================================
	// PROPERTIES
	// ========================================
	
	/** Current save game object */
	UPROPERTY()
	UAtlasSaveGame* CurrentSaveGame;
	
	/** Name of the save slot */
	UPROPERTY()
	FString SaveSlotName = TEXT("AtlasSaveSlot");
	
	/** User index for saves */
	UPROPERTY()
	int32 UserIndex = 0;
	
	/** Whether auto-save is enabled */
	UPROPERTY()
	bool bAutoSaveEnabled = true;
	
	/** Time between auto-saves */
	UPROPERTY()
	float AutoSaveInterval = 60.0f;
	
	/** Timer handle for auto-save */
	FTimerHandle AutoSaveTimerHandle;
	
	/** Whether we're currently saving (prevents concurrent saves) */
	bool bIsSaving = false;
	
	/** Whether we're currently loading */
	bool bIsLoading = false;
};