#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "RewardSelectionComponent.generated.h"

// Forward declarations
class URewardDataAsset;
class USlotManagerComponent;
class UUserWidget;

/**
 * Result of a reward selection
 */
UENUM(BlueprintType)
enum class ERewardSelectionResult : uint8
{
	Selected       UMETA(DisplayName = "Reward Selected"),
	Skipped        UMETA(DisplayName = "Selection Skipped"),
	Cancelled      UMETA(DisplayName = "Selection Cancelled"),
	TimedOut       UMETA(DisplayName = "Selection Timed Out")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRewardSelected, URewardDataAsset*, SelectedReward, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionComplete, ERewardSelectionResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectionPresented);

/**
 * Component that manages the reward selection UI and flow.
 * Presents reward choices to the player after defeating enemies.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API URewardSelectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URewardSelectionComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// REWARD PRESENTATION
	// ========================================
	
	/**
	 * Present reward choices to the player
	 * @param Rewards Array of rewards to choose from
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	void PresentRewardChoice(const TArray<URewardDataAsset*>& Rewards);
	
	/**
	 * Present a single reward (no choice)
	 * @param Reward The reward to present
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	void PresentSingleReward(URewardDataAsset* Reward);
	
	/**
	 * Close the reward selection UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	void CloseSelection();
	
	/**
	 * Force selection timeout
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	void ForceTimeout();
	
	// ========================================
	// SELECTION HANDLING
	// ========================================
	
	/**
	 * Called when player selects a reward
	 * @param SelectedReward The chosen reward
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	void OnRewardChosen(URewardDataAsset* SelectedReward);
	
	/**
	 * Called when player skips selection
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	void OnSelectionSkipped();
	
	/**
	 * Handle reward equipping or enhancement
	 * @param Reward The reward to equip
	 * @return True if successfully equipped/enhanced
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	bool ProcessRewardSelection(URewardDataAsset* Reward);
	
	// ========================================
	// SLOT MANAGEMENT
	// ========================================
	
	/**
	 * Show slot management UI for reward placement
	 * @param Reward The reward to place
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection|Slots")
	void ShowSlotManagement(URewardDataAsset* Reward);
	
	/**
	 * Try to auto-equip a reward to the best slot
	 * @param Reward The reward to equip
	 * @return Slot index if equipped, -1 if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection|Slots")
	int32 AutoEquipReward(URewardDataAsset* Reward);
	
	/**
	 * Called when player selects a slot for the reward
	 * @param SlotIndex The chosen slot
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection|Slots")
	void OnSlotSelected(int32 SlotIndex);
	
	/**
	 * Check if player needs to choose a slot
	 * @param Reward The reward to check
	 * @return True if slot selection is needed
	 */
	UFUNCTION(BlueprintPure, Category = "Reward Selection|Slots")
	bool NeedsSlotSelection(URewardDataAsset* Reward) const;
	
	// ========================================
	// QUERIES
	// ========================================
	
	/**
	 * Check if selection UI is currently active
	 * @return True if UI is showing
	 */
	UFUNCTION(BlueprintPure, Category = "Reward Selection")
	bool IsSelectionActive() const { return bSelectionActive; }
	
	/**
	 * Get currently presented rewards
	 * @return Array of current reward choices
	 */
	UFUNCTION(BlueprintPure, Category = "Reward Selection")
	TArray<URewardDataAsset*> GetCurrentChoices() const { return CurrentRewardChoices; }
	
	/**
	 * Get the pending reward waiting for slot selection
	 * @return Pending reward or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Reward Selection")
	URewardDataAsset* GetPendingReward() const { return PendingReward; }
	
	/**
	 * Get time remaining for selection
	 * @return Seconds remaining or -1 if no timeout
	 */
	UFUNCTION(BlueprintPure, Category = "Reward Selection")
	float GetTimeRemaining() const;
	
	// ========================================
	// UI CREATION
	// ========================================
	
	/**
	 * Create and show the reward selection widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection|UI")
	void CreateSelectionWidget();
	
	/**
	 * Destroy the active selection widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection|UI")
	void DestroySelectionWidget();
	
	/**
	 * Update the widget with new reward data
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection|UI")
	void UpdateSelectionWidget();
	
	// ========================================
	// COMPARISON & PREVIEW
	// ========================================
	
	/**
	 * Compare a reward with currently equipped items
	 * @param Reward The reward to compare
	 * @return Comparison data for UI display
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection|Preview")
	FText GetRewardComparison(URewardDataAsset* Reward) const;
	
	/**
	 * Preview what would happen if reward is selected
	 * @param Reward The reward to preview
	 * @return Preview text
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection|Preview")
	FText GetSelectionPreview(URewardDataAsset* Reward) const;
	
	/**
	 * Check if a reward would enhance an existing one
	 * @param Reward The reward to check
	 * @return True if it would stack/enhance
	 */
	UFUNCTION(BlueprintPure, Category = "Reward Selection|Preview")
	bool WouldEnhanceExisting(URewardDataAsset* Reward) const;
	
	// ========================================
	// EVENTS
	// ========================================
	
	UPROPERTY(BlueprintAssignable, Category = "Reward Selection|Events")
	FOnRewardSelected OnRewardSelected;
	
	UPROPERTY(BlueprintAssignable, Category = "Reward Selection|Events")
	FOnSelectionComplete OnSelectionComplete;
	
	UPROPERTY(BlueprintAssignable, Category = "Reward Selection|Events")
	FOnSelectionPresented OnSelectionPresented;

protected:
	// ========================================
	// INTERNAL FUNCTIONS
	// ========================================
	
	/** Handle selection timeout */
	void OnSelectionTimeout();
	
	/** Finalize the selection process */
	void FinalizeSelection(ERewardSelectionResult Result);
	
	/** Get the best slot for auto-equipping */
	int32 FindBestSlotForReward(URewardDataAsset* Reward) const;
	
	/** Check if reward can replace an existing one */
	bool CanReplaceReward(URewardDataAsset* NewReward, URewardDataAsset* ExistingReward) const;

protected:
	// ========================================
	// PROPERTIES
	// ========================================
	
	/** Widget class for reward selection UI */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> RewardSelectionWidgetClass;
	
	/** Widget class for slot management UI */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> SlotManagementWidgetClass;
	
	/** Active selection widget instance */
	UPROPERTY()
	UUserWidget* ActiveSelectionWidget;
	
	/** Active slot management widget */
	UPROPERTY()
	UUserWidget* ActiveSlotWidget;
	
	/** Currently presented reward choices */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TArray<URewardDataAsset*> CurrentRewardChoices;
	
	/** Reward waiting for slot selection */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	URewardDataAsset* PendingReward;
	
	/** Whether selection is currently active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bSelectionActive = false;
	
	/** Reference to player's slot manager */
	UPROPERTY()
	USlotManagerComponent* PlayerSlotManager;
	
	/** Time allowed for selection (0 = no limit) */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	float SelectionTimeout = 30.0f;
	
	/** Allow skipping reward selection */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	bool bAllowSkip = true;
	
	/** Auto-select random reward on timeout */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	bool bAutoSelectOnTimeout = true;
	
	/** Pause game during selection */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	bool bPauseDuringSelection = true;
	
	/** Timer handle for selection timeout */
	FTimerHandle TimeoutTimerHandle;
	
	/** Time when selection started */
	float SelectionStartTime;
};