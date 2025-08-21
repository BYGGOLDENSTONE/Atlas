#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "RoomDataAsset.generated.h"

// Forward declarations
class AGameCharacterBase;
class URewardDataAsset;

/**
 * Difficulty scaling for room encounters
 */
UENUM(BlueprintType)
enum class ERoomDifficulty : uint8
{
	Easy         UMETA(DisplayName = "Easy"),
	Medium       UMETA(DisplayName = "Medium"),
	Hard         UMETA(DisplayName = "Hard"),
	Boss         UMETA(DisplayName = "Boss")
};

/**
 * Environmental hazards that can appear in rooms
 */
UENUM(BlueprintType)
enum class ERoomHazard : uint8
{
	None              UMETA(DisplayName = "None"),
	LowGravity        UMETA(DisplayName = "Low Gravity"),
	ElectricalSurges  UMETA(DisplayName = "Electrical Surges"),
	HullBreach        UMETA(DisplayName = "Hull Breach"),
	ToxicLeak         UMETA(DisplayName = "Toxic Leak"),
	SystemMalfunction UMETA(DisplayName = "System Malfunction")
};

/**
 * Data structure for reward choices presented to player
 */
USTRUCT(BlueprintType)
struct FRewardChoice
{
	GENERATED_BODY()

	/** The reward option */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	URewardDataAsset* Reward = nullptr;

	/** Weight for random selection (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.1f, ClampMax = 10.0f))
	float SelectionWeight = 1.0f;

	/** Minimum player level required for this reward to appear */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1))
	int32 MinimumLevel = 1;

	/** Maximum times this can appear per run */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 1))
	int32 MaxAppearances = 999;

	FRewardChoice() {}
};

/**
 * Data asset defining a room's configuration, enemy, and reward pool
 */
UCLASS(BlueprintType)
class ATLAS_API URoomDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ========================================
	// ROOM IDENTITY
	// ========================================
	
	/** Unique identifier for this room */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName RoomID;
	
	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText RoomName;
	
	/** Room description/lore */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;
	
	/** Room icon for map display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	class UTexture2D* RoomIcon;
	
	// ========================================
	// ROOM CONFIGURATION
	// ========================================
	
	/** The reward category theme for this room */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	ERewardCategory RoomTheme;
	
	/** Base difficulty of this room */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	ERoomDifficulty Difficulty = ERoomDifficulty::Medium;
	
	/** Environmental hazard in this room */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	ERoomHazard EnvironmentalHazard = ERoomHazard::None;
	
	/** Level(s) this room can appear on (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (ClampMin = 1, ClampMax = 5))
	TArray<int32> AppearOnLevels;
	
	/** Can this room appear multiple times in a single run? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	bool bCanRepeat = false;
	
	/** Weight for random room selection */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (ClampMin = 0.1f, ClampMax = 10.0f))
	float RoomSelectionWeight = 1.0f;
	
	// ========================================
	// ENEMY CONFIGURATION
	// ========================================
	
	/** The unique enemy that spawns in this room */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TSubclassOf<AGameCharacterBase> UniqueEnemy;
	
	/** Enemy display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FText EnemyName;
	
	/** Enemy description/lore */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (MultiLine = true))
	FText EnemyDescription;
	
	/** Base enemy power level (scales with player) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = 1, ClampMax = 10))
	int32 EnemyBasePower = 3;
	
	/** Enemy AI behavior preset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FGameplayTag EnemyAIPreset;
	
	/** Special abilities this enemy has */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TArray<FGameplayTag> EnemyAbilities;
	
	// ========================================
	// REWARD POOL
	// ========================================
	
	/** Pool of rewards that can appear in this room */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	TArray<FRewardChoice> RewardPool;
	
	/** Number of reward choices to present to player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards", meta = (ClampMin = 1, ClampMax = 4))
	int32 RewardChoiceCount = 2;
	
	/** Guaranteed reward (always appears as one of the choices) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	URewardDataAsset* GuaranteedReward;
	
	/** Chance for a bonus reward (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float BonusRewardChance = 0.1f;
	
	/** Bonus reward if chance succeeds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	URewardDataAsset* BonusReward;
	
	// ========================================
	// ENVIRONMENTAL SETUP
	// ========================================
	
	/** Level blueprint to load for this room */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
	TSoftObjectPtr<class UWorld> RoomLevel;
	
	/** Spawn points for enemies */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
	TArray<FTransform> EnemySpawnPoints;
	
	/** Spawn points for interactables */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
	TArray<FTransform> InteractableSpawnPoints;
	
	/** Ambient sound for this room */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
	class USoundBase* AmbientSound;
	
	/** Music track for combat */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
	class USoundBase* CombatMusic;
	
	// ========================================
	// SPECIAL CONDITIONS
	// ========================================
	
	/** Special win conditions beyond defeating enemy */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conditions")
	FGameplayTagContainer WinConditions;
	
	/** Special lose conditions beyond death/integrity loss */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conditions")
	FGameplayTagContainer LoseConditions;
	
	/** Time limit for this room (0 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conditions", meta = (ClampMin = 0.0f))
	float TimeLimit = 0.0f;
	
	/** Station integrity damage per second in this room */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conditions", meta = (ClampMin = 0.0f))
	float IntegrityDrainRate = 0.0f;
	
	// ========================================
	// DIALOGUE & NARRATIVE
	// ========================================
	
	/** Pre-combat dialogue/monologue */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
	TArray<FText> IntroDialogue;
	
	/** Victory dialogue */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
	TArray<FText> VictoryDialogue;
	
	/** Defeat dialogue */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
	TArray<FText> DefeatDialogue;
	
	// ========================================
	// HELPER FUNCTIONS
	// ========================================
	
	/**
	 * Get scaled enemy power based on player level
	 * @param PlayerLevel Current player level
	 * @param EquippedSlots Number of reward slots player has equipped
	 * @return Scaled enemy power level
	 */
	UFUNCTION(BlueprintPure, Category = "Room")
	int32 GetScaledEnemyPower(int32 PlayerLevel, int32 EquippedSlots) const
	{
		// As per GDD: Enemy power = Player equipped slots + 1
		return EquippedSlots + 1;
	}
	
	/**
	 * Select random rewards from the pool
	 * @param Count Number of rewards to select
	 * @param PlayerLevel Current player level for filtering
	 * @return Array of selected rewards
	 */
	UFUNCTION(BlueprintCallable, Category = "Room")
	TArray<URewardDataAsset*> SelectRandomRewards(int32 Count, int32 PlayerLevel) const;
	
	/**
	 * Check if this room is valid for a given level
	 * @param Level The level to check (1-5)
	 * @return True if room can appear on this level
	 */
	UFUNCTION(BlueprintPure, Category = "Room")
	bool IsValidForLevel(int32 Level) const
	{
		return AppearOnLevels.Contains(Level);
	}
	
	/**
	 * Get the display name for the environmental hazard
	 * @return Localized hazard name
	 */
	UFUNCTION(BlueprintPure, Category = "Room")
	FText GetHazardDisplayName() const;
};