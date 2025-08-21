#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SlotManagerComponent.generated.h"

// Forward declarations
class URewardDataAsset;
class UActionManagerComponent;

/**
 * Represents a single equipped reward with its current stack level
 */
USTRUCT(BlueprintType)
struct FEquippedReward
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	URewardDataAsset* RewardData = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 StackLevel = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = -1;

	FEquippedReward() {}
	FEquippedReward(URewardDataAsset* InReward, int32 InSlot) 
		: RewardData(InReward), StackLevel(1), SlotIndex(InSlot) {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRewardEquipped, int32, SlotIndex, URewardDataAsset*, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRewardRemoved, int32, SlotIndex, URewardDataAsset*, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRewardEnhanced, int32, SlotIndex, URewardDataAsset*, Reward, int32, NewStackLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlotsChanged);

/**
 * Component that manages the player's equipped rewards and slot system.
 * Rewards persist between runs and provide various bonuses.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API USlotManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USlotManagerComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// SLOT MANAGEMENT
	// ========================================
	
	/**
	 * Equip a reward to a specific slot
	 * @param Reward The reward to equip
	 * @param SlotIndex The slot to equip to (0-based)
	 * @return True if successfully equipped
	 */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	bool EquipReward(URewardDataAsset* Reward, int32 SlotIndex);
	
	/**
	 * Remove a reward from a specific slot
	 * @param SlotIndex The slot to clear
	 * @return True if a reward was removed
	 */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	bool RemoveReward(int32 SlotIndex);
	
	/**
	 * Enhance an existing reward (increase stack level)
	 * @param RewardTag The tag of the reward to enhance
	 * @return True if successfully enhanced
	 */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	bool EnhanceReward(FGameplayTag RewardTag);
	
	/**
	 * Replace a reward with another one
	 * @param SlotIndex The slot containing the reward to replace
	 * @param NewReward The new reward to equip
	 * @return True if successfully replaced
	 */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	bool ReplaceReward(int32 SlotIndex, URewardDataAsset* NewReward);
	
	/**
	 * Swap two rewards between slots
	 * @param SlotA First slot index
	 * @param SlotB Second slot index
	 * @return True if successfully swapped
	 */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	bool SwapRewards(int32 SlotA, int32 SlotB);
	
	/**
	 * Clear all equipped rewards
	 */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void ClearAllRewards();
	
	// ========================================
	// QUERIES
	// ========================================
	
	/**
	 * Get the reward equipped in a specific slot
	 * @param SlotIndex The slot to check
	 * @return The equipped reward or nullptr if empty
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager")
	URewardDataAsset* GetRewardInSlot(int32 SlotIndex) const;
	
	/**
	 * Get the stack level of a reward in a slot
	 * @param SlotIndex The slot to check
	 * @return Stack level (0 if no reward)
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager")
	int32 GetRewardStackLevel(int32 SlotIndex) const;
	
	/**
	 * Find a reward by its tag
	 * @param RewardTag The tag to search for
	 * @return The equipped reward data or nullptr if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager")
	FEquippedReward FindRewardByTag(FGameplayTag RewardTag) const;
	
	/**
	 * Check if a reward is currently equipped
	 * @param RewardTag The reward tag to check
	 * @return True if equipped
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager")
	bool IsRewardEquipped(FGameplayTag RewardTag) const;
	
	/**
	 * Get the total number of slots being used
	 * @return Number of used slots
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager")
	int32 GetUsedSlotCount() const;
	
	/**
	 * Get the number of available slots
	 * @return Number of free slots
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager")
	int32 GetAvailableSlotCount() const;
	
	/**
	 * Check if we have enough slots for a reward
	 * @param Reward The reward to check
	 * @return True if enough slots available
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager")
	bool HasSlotsForReward(URewardDataAsset* Reward) const;
	
	/**
	 * Get all currently equipped rewards
	 * @return Array of equipped rewards
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager")
	TArray<FEquippedReward> GetAllEquippedRewards() const;
	
	/**
	 * Get rewards of a specific category
	 * @param Category The category to filter by
	 * @return Array of rewards in that category
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager")
	TArray<FEquippedReward> GetRewardsByCategory(ERewardCategory Category) const;
	
	// ========================================
	// STAT CALCULATIONS
	// ========================================
	
	/**
	 * Calculate total stat modifier from all equipped rewards
	 * @param StatName The stat to calculate
	 * @return Total modifier value
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager|Stats")
	float CalculateTotalStatModifier(FName StatName) const;
	
	/**
	 * Get all active stat modifiers
	 * @return Map of stat names to total values
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager|Stats")
	TMap<FName, float> GetAllStatModifiers() const;
	
	// ========================================
	// PERSISTENCE
	// ========================================
	
	/**
	 * Save current rewards for next run
	 */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager|Persistence")
	void SaveRewardsForNextRun();
	
	/**
	 * Load rewards from previous run
	 */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager|Persistence")
	void LoadRewardsFromSave();
	
	/**
	 * Reset to default loadout
	 */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager|Persistence")
	void ResetToDefaultLoadout();
	
	// ========================================
	// VALIDATION
	// ========================================
	
	/**
	 * Check if a reward can be equipped
	 * @param Reward The reward to check
	 * @param OutReason Reason why it can't be equipped
	 * @return True if can be equipped
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager|Validation")
	bool CanEquipReward(URewardDataAsset* Reward, FText& OutReason) const;
	
	/**
	 * Check for incompatible rewards
	 * @param Reward The reward to check
	 * @return Array of incompatible reward tags
	 */
	UFUNCTION(BlueprintPure, Category = "Slot Manager|Validation")
	TArray<FGameplayTag> GetIncompatibleRewards(URewardDataAsset* Reward) const;
	
	// ========================================
	// EVENTS
	// ========================================
	
	UPROPERTY(BlueprintAssignable, Category = "Slot Manager|Events")
	FOnRewardEquipped OnRewardEquipped;
	
	UPROPERTY(BlueprintAssignable, Category = "Slot Manager|Events")
	FOnRewardRemoved OnRewardRemoved;
	
	UPROPERTY(BlueprintAssignable, Category = "Slot Manager|Events")
	FOnRewardEnhanced OnRewardEnhanced;
	
	UPROPERTY(BlueprintAssignable, Category = "Slot Manager|Events")
	FOnSlotsChanged OnSlotsChanged;

protected:
	// ========================================
	// INTERNAL FUNCTIONS
	// ========================================
	
	/** Apply effects when a reward is equipped */
	void ApplyRewardEffects(const FEquippedReward& Reward);
	
	/** Remove effects when a reward is unequipped */
	void RemoveRewardEffects(const FEquippedReward& Reward);
	
	/** Update stat modifiers on the owner */
	void UpdateOwnerStats();
	
	/** Register active abilities with ActionManager */
	void RegisterActiveAbilities();
	
	/** Validate all equipped rewards are still valid */
	void ValidateEquippedRewards();

protected:
	// ========================================
	// PROPERTIES
	// ========================================
	
	/** Maximum number of reward slots available */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	int32 MaxSlots = 6;
	
	/** Currently equipped rewards */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TArray<FEquippedReward> EquippedRewards;
	
	/** Map of reward tags to their stack levels */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TMap<FGameplayTag, int32> RewardStacks;
	
	/** Default rewards that are always equipped at start */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	TArray<URewardDataAsset*> DefaultRewards;
	
	/** Reference to the action manager component */
	UPROPERTY()
	UActionManagerComponent* ActionManagerComponent;
	
	/** Owner character reference */
	UPROPERTY()
	class AGameCharacterBase* OwnerCharacter;
	
	/** Save game slot name for persistence */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	FString SaveSlotName = TEXT("AtlasRewards");
};