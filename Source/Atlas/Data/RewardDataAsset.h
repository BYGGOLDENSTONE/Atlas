#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RewardDataAsset.generated.h"

// Forward declarations
class UActionDataAsset;

/**
 * Reward categories as defined in the GDD
 */
UENUM(BlueprintType)
enum class ERewardCategory : uint8
{
	Defense         UMETA(DisplayName = "Defense (Block, Parry, Dodge)"),
	Offense         UMETA(DisplayName = "Offense (Attacks)"),
	PassiveStats    UMETA(DisplayName = "Passive Stats (Speed, Health, Knockback)"),
	PassiveAbility  UMETA(DisplayName = "Passive Ability (Second Life, etc)"),
	Interactable    UMETA(DisplayName = "Interactable (Hacking abilities)")
};

/**
 * Data asset defining a reward that can be equipped by the player.
 * Rewards persist between runs and can be enhanced through stacking.
 */
UCLASS(BlueprintType)
class ATLAS_API URewardDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ========================================
	// IDENTITY
	// ========================================
	
	/** Unique tag identifying this reward */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FGameplayTag RewardTag;
	
	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText RewardName;
	
	/** Description of what this reward does */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;
	
	/** Icon for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	class UTexture2D* Icon;
	
	// ========================================
	// CONFIGURATION
	// ========================================
	
	/** Category this reward belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	ERewardCategory Category;
	
	/** Number of slots this reward requires when equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (ClampMin = 1, ClampMax = 3))
	int32 SlotCost = 1;
	
	/** Maximum times this reward can be stacked/enhanced */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (ClampMin = 1, ClampMax = 5))
	int32 MaxStackLevel = 3;
	
	/** Multipliers applied at each stack level (index 0 = level 1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	TArray<float> StackMultipliers = {1.0f, 1.5f, 2.0f};
	
	// ========================================
	// ACTIVE ABILITY REWARDS
	// ========================================
	
	/** For active ability rewards - the action this reward grants */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Active Ability", 
		meta = (EditCondition = "Category == ERewardCategory::Defense || Category == ERewardCategory::Offense"))
	UActionDataAsset* LinkedAction;
	
	/** Whether this replaces an existing ability or adds a new one */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Active Ability",
		meta = (EditCondition = "LinkedAction != nullptr"))
	bool bReplacesExistingAbility = false;
	
	/** If replacing, which action tag to replace */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Active Ability",
		meta = (EditCondition = "bReplacesExistingAbility && LinkedAction != nullptr"))
	FGameplayTag ReplacesActionTag;
	
	// ========================================
	// PASSIVE STAT MODIFIERS
	// ========================================
	
	/** Stat modifiers applied when this reward is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Passive Stats",
		meta = (EditCondition = "Category == ERewardCategory::PassiveStats"))
	TMap<FName, float> StatModifiers;
	
	/** Common stat names for reference:
	 * MaxHealth - Increases maximum health
	 * MoveSpeed - Movement speed multiplier
	 * AttackSpeed - Attack animation speed multiplier
	 * KnockbackForce - Knockback force multiplier
	 * DamageReduction - Incoming damage reduction
	 * DamageMultiplier - Outgoing damage multiplier
	 * PoiseMax - Maximum poise value
	 * PoiseRegen - Poise regeneration rate
	 */
	
	// ========================================
	// PASSIVE ABILITIES
	// ========================================
	
	/** Special passive ability type (for non-stat passives) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Passive Ability",
		meta = (EditCondition = "Category == ERewardCategory::PassiveAbility"))
	FGameplayTag PassiveAbilityTag;
	
	/** Custom parameters for passive abilities */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Passive Ability",
		meta = (EditCondition = "Category == ERewardCategory::PassiveAbility"))
	TMap<FName, float> PassiveParameters;
	
	// ========================================
	// INTERACTABLES
	// ========================================
	
	/** Type of environmental interaction this enables */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable",
		meta = (EditCondition = "Category == ERewardCategory::Interactable"))
	FGameplayTag InteractableType;
	
	/** Range at which this interactable can be triggered */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable",
		meta = (EditCondition = "Category == ERewardCategory::Interactable", ClampMin = 100.0f, ClampMax = 2000.0f))
	float InteractionRange = 500.0f;
	
	/** Cooldown between uses of this interactable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable",
		meta = (EditCondition = "Category == ERewardCategory::Interactable", ClampMin = 0.0f))
	float InteractableCooldown = 5.0f;
	
	// ========================================
	// REQUIREMENTS & RESTRICTIONS
	// ========================================
	
	/** Minimum player level required to equip this reward */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	int32 MinimumLevel = 1;
	
	/** Tags that must be present for this reward to function */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	FGameplayTagContainer RequiredTags;
	
	/** Tags that prevent this reward from functioning */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	FGameplayTagContainer BlockedByTags;
	
	/** Other rewards that cannot be equipped alongside this one */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	TArray<FGameplayTag> IncompatibleRewards;
	
	// ========================================
	// PROGRESSION
	// ========================================
	
	/** Whether this reward is unlocked by default */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	bool bUnlockedByDefault = false;
	
	/** Conditions required to unlock this reward */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	FGameplayTagContainer UnlockRequirements;
	
	/** Currency cost to purchase/unlock this reward */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 UnlockCost = 100;
	
	// ========================================
	// VISUAL & AUDIO
	// ========================================
	
	/** Visual effect spawned when this reward activates */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* ActivationVFX;
	
	/** Sound played when this reward activates */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	class USoundBase* ActivationSound;
	
	/** Persistent visual effect while equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	class UParticleSystem* PersistentVFX;
	
public:
	// ========================================
	// HELPER FUNCTIONS
	// ========================================
	
	/** Get the effectiveness multiplier for a given stack level */
	UFUNCTION(BlueprintPure, Category = "Reward")
	float GetStackMultiplier(int32 StackLevel) const
	{
		if (StackMultipliers.Num() == 0) return 1.0f;
		int32 Index = FMath::Clamp(StackLevel - 1, 0, StackMultipliers.Num() - 1);
		return StackMultipliers[Index];
	}
	
	/** Check if this reward can stack with another instance */
	UFUNCTION(BlueprintPure, Category = "Reward")
	bool CanStackWith(const URewardDataAsset* Other) const
	{
		return Other && Other->RewardTag == RewardTag && MaxStackLevel > 1;
	}
	
	/** Get display text for a specific stack level */
	UFUNCTION(BlueprintPure, Category = "Reward")
	FText GetStackedDescription(int32 StackLevel) const
	{
		float Multiplier = GetStackMultiplier(StackLevel);
		return FText::Format(NSLOCTEXT("Reward", "StackedDesc", "{0} (x{1})"), 
			Description, FText::AsNumber(Multiplier));
	}
};