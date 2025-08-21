#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AIDifficultyComponent.generated.h"

/**
 * AI behavior presets based on difficulty
 */
UENUM(BlueprintType)
enum class EAIBehaviorPreset : uint8
{
	Defensive     UMETA(DisplayName = "Defensive (Blocks/Parries more)"),
	Balanced      UMETA(DisplayName = "Balanced (Mixed approach)"),
	Aggressive    UMETA(DisplayName = "Aggressive (Attacks more)"),
	Berserker     UMETA(DisplayName = "Berserker (All offense)"),
	Tactical      UMETA(DisplayName = "Tactical (Uses environment)")
};

/**
 * Player pattern analysis data
 */
USTRUCT(BlueprintType)
struct FPlayerPatternData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 BlockCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ParryCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 DashCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 AttackCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 HeavyAttackCount = 0;

	UPROPERTY(BlueprintReadOnly)
	float AverageReactionTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag MostUsedAction;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag LastPlayerAction;

	FPlayerPatternData() {}
};

/**
 * Component that manages AI difficulty scaling based on player power.
 * Implements the GDD rule: Enemy Power = Player Equipped Slots + 1
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UAIDifficultyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIDifficultyComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// DIFFICULTY CALCULATION
	// ========================================
	
	/**
	 * Calculate difficulty based on player equipped slots
	 * @param PlayerEquippedSlots Number of slots the player has equipped
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Difficulty")
	void CalculateDifficulty(int32 PlayerEquippedSlots);
	
	/**
	 * Apply difficulty scaling to the AI
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Difficulty")
	void ApplyDifficultyScaling();
	
	/**
	 * Recalculate difficulty (checks player slots automatically)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Difficulty")
	void RecalculateDifficulty();
	
	// ========================================
	// STAT MODIFIERS
	// ========================================
	
	/**
	 * Get damage multiplier based on difficulty
	 * @return Damage multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Stats")
	float GetDamageMultiplier() const { return DamageMultiplier; }
	
	/**
	 * Get health multiplier based on difficulty
	 * @return Health multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Stats")
	float GetHealthMultiplier() const { return HealthMultiplier; }
	
	/**
	 * Get attack speed modifier
	 * @return Attack speed multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Stats")
	float GetAttackSpeedModifier() const { return AttackSpeedModifier; }
	
	/**
	 * Get reaction time modifier (lower = faster reactions)
	 * @return Reaction time multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Stats")
	float GetReactionTimeModifier() const { return ReactionTimeModifier; }
	
	/**
	 * Get movement speed modifier
	 * @return Movement speed multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Stats")
	float GetMovementSpeedModifier() const { return MovementSpeedModifier; }
	
	/**
	 * Get poise damage multiplier
	 * @return Poise damage multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Stats")
	float GetPoiseDamageMultiplier() const { return PoiseDamageMultiplier; }
	
	// ========================================
	// BEHAVIOR MODIFIERS
	// ========================================
	
	/**
	 * Get aggression level (0-1, affects decision making)
	 * @return Aggression level
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Behavior")
	float GetAggressionLevel() const { return AggressionLevel; }
	
	/**
	 * Get defense priority (0-1, affects blocking/parrying frequency)
	 * @return Defense priority
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Behavior")
	float GetDefensePriority() const { return DefensePriority; }
	
	/**
	 * Get combo likelihood (0-1, chance to chain attacks)
	 * @return Combo likelihood
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Behavior")
	float GetComboLikelihood() const { return ComboLikelihood; }
	
	/**
	 * Get ability usage frequency (0-1, how often to use special abilities)
	 * @return Ability frequency
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Behavior")
	float GetAbilityUsageFrequency() const { return AbilityUsageFrequency; }
	
	/**
	 * Get current behavior preset
	 * @return AI behavior preset
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Behavior")
	EAIBehaviorPreset GetBehaviorPreset() const { return CurrentBehaviorPreset; }
	
	/**
	 * Set behavior preset
	 * @param NewPreset The preset to use
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Difficulty|Behavior")
	void SetBehaviorPreset(EAIBehaviorPreset NewPreset);
	
	// ========================================
	// ADAPTIVE AI
	// ========================================
	
	/**
	 * Record a player action for pattern analysis
	 * @param ActionTag The action the player performed
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Difficulty|Adaptive")
	void RecordPlayerAction(FGameplayTag ActionTag);
	
	/**
	 * Analyze player patterns and adapt strategy
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Difficulty|Adaptive")
	void AnalyzePlayerPattern();
	
	/**
	 * Get the analyzed player pattern data
	 * @return Player pattern data
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Adaptive")
	FPlayerPatternData GetPlayerPatternData() const { return PlayerPattern; }
	
	/**
	 * Adapt AI strategy based on player patterns
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Difficulty|Adaptive")
	void AdaptToPlayerStrategy();
	
	/**
	 * Reset pattern analysis
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Difficulty|Adaptive")
	void ResetPatternAnalysis();
	
	// ========================================
	// DECISION MAKING
	// ========================================
	
	/**
	 * Should the AI attack now?
	 * @return True if AI should attack
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Decisions")
	bool ShouldAttack() const;
	
	/**
	 * Should the AI defend now?
	 * @return True if AI should defend
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Decisions")
	bool ShouldDefend() const;
	
	/**
	 * Should the AI use an ability?
	 * @return True if AI should use ability
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty|Decisions")
	bool ShouldUseAbility() const;
	
	/**
	 * Get the recommended action based on current state
	 * @return Recommended action tag
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Difficulty|Decisions")
	FGameplayTag GetRecommendedAction();
	
	// ========================================
	// QUERIES
	// ========================================
	
	/**
	 * Get current power level
	 * @return AI power level
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty")
	int32 GetPowerLevel() const { return PowerLevel; }
	
	/**
	 * Get difficulty rating (0-10)
	 * @return Difficulty rating
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty")
	float GetDifficultyRating() const;
	
	/**
	 * Is the AI in berserker mode (low health, high aggression)?
	 * @return True if berserking
	 */
	UFUNCTION(BlueprintPure, Category = "AI Difficulty")
	bool IsBerserking() const;

protected:
	// ========================================
	// INTERNAL FUNCTIONS
	// ========================================
	
	/** Apply scaling based on power level */
	void ApplyPowerLevelScaling(int32 InPowerLevel);
	
	/** Update behavior based on preset */
	void UpdateBehaviorFromPreset();
	
	/** Counter strategies for different player types */
	void CounterDefensivePlayer();
	void CounterAggressivePlayer();
	void CounterEvasivePlayer();
	
	/** Calculate decision weights */
	float CalculateAttackWeight() const;
	float CalculateDefenseWeight() const;
	float CalculateAbilityWeight() const;

protected:
	// ========================================
	// STAT MODIFIERS
	// ========================================
	
	/** Current power level (Player Slots + 1) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 PowerLevel = 1;
	
	/** Damage output multiplier */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float DamageMultiplier = 1.0f;
	
	/** Health pool multiplier */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float HealthMultiplier = 1.0f;
	
	/** Attack animation speed multiplier */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float AttackSpeedModifier = 1.0f;
	
	/** Reaction time multiplier (lower = faster) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float ReactionTimeModifier = 1.0f;
	
	/** Movement speed multiplier */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float MovementSpeedModifier = 1.0f;
	
	/** Poise damage multiplier */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float PoiseDamageMultiplier = 1.0f;
	
	// ========================================
	// BEHAVIOR MODIFIERS
	// ========================================
	
	/** Aggression level (0-1) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	float AggressionLevel = 0.5f;
	
	/** Defense priority (0-1) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	float DefensePriority = 0.5f;
	
	/** Combo likelihood (0-1) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	float ComboLikelihood = 0.3f;
	
	/** Ability usage frequency (0-1) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	float AbilityUsageFrequency = 0.2f;
	
	/** Current behavior preset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	EAIBehaviorPreset CurrentBehaviorPreset = EAIBehaviorPreset::Balanced;
	
	// ========================================
	// ADAPTIVE AI
	// ========================================
	
	/** Analyzed player pattern data */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Adaptive")
	FPlayerPatternData PlayerPattern;
	
	/** Whether to enable adaptive AI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive")
	bool bEnableAdaptiveAI = true;
	
	/** Time window for pattern analysis */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive")
	float PatternAnalysisWindow = 30.0f;
	
	/** Minimum actions before adapting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive")
	int32 MinActionsForAdaptation = 10;
	
	// ========================================
	// CONFIGURATION
	// ========================================
	
	/** Base scaling per power level */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	float DamageScalingPerLevel = 0.15f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	float HealthScalingPerLevel = 0.2f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	float SpeedScalingPerLevel = 0.05f;
	
	/** Berserker mode threshold (health percentage) */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	float BerserkerHealthThreshold = 0.25f;
	
	/** Reference to owner character */
	UPROPERTY()
	class AGameCharacterBase* OwnerCharacter;
	
	/** Time tracking for pattern analysis */
	float LastActionTime = 0.0f;
	TArray<float> ReactionTimes;
};