#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "RewardSelectionWidget.generated.h"

// Forward declarations
class URewardDataAsset;
class URewardSelectionComponent;
class UButton;
class UImage;
class UTextBlock;
class UProgressBar;
class UHorizontalBox;
class UBorder;

/**
 * Individual reward card widget that displays a single reward option
 */
UCLASS()
class ATLAS_API URewardCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========================================
	// UI COMPONENTS
	// ========================================
	
	/** Border that contains the entire card */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBorder* CardBorder;
	
	/** Button for selecting this reward */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* SelectButton;
	
	/** Icon image for the reward */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* RewardIcon;
	
	/** Name of the reward */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* RewardName;
	
	/** Description of the reward */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* RewardDescription;
	
	/** Slot cost display */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* SlotCostText;
	
	/** Stack level indicator (if enhancing) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* StackLevelText;
	
	/** Category color indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* CategoryColorBar;
	
	// ========================================
	// DATA
	// ========================================
	
	/** The reward this card represents */
	UPROPERTY(BlueprintReadOnly)
	URewardDataAsset* RewardData;
	
	/** Whether this is an enhancement of an existing reward */
	UPROPERTY(BlueprintReadOnly)
	bool bIsEnhancement;
	
	/** Current stack level if enhancing */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentStackLevel;
	
	// ========================================
	// FUNCTIONS
	// ========================================
	
	/** Initialize this card with reward data */
	UFUNCTION(BlueprintCallable, Category = "Reward Card")
	void SetupCard(URewardDataAsset* InReward, bool bEnhancing = false, int32 StackLevel = 1);
	
	/** Play selection animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Reward Card")
	void PlaySelectionAnimation();
	
	/** Play hover animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Reward Card")
	void PlayHoverAnimation();
	
	/** Get category color for visual theming */
	UFUNCTION(BlueprintPure, Category = "Reward Card")
	FLinearColor GetCategoryColor() const;

protected:
	virtual void NativeConstruct() override;
	
private:
	UFUNCTION()
	void OnSelectButtonClicked();
	
	UFUNCTION()
	void OnSelectButtonHovered();
};

/**
 * Main reward selection widget that presents reward choices to the player
 */
UCLASS()
class ATLAS_API URewardSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========================================
	// UI COMPONENTS
	// ========================================
	
	/** Background blur/overlay */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* BackgroundOverlay;
	
	/** Title text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TitleText;
	
	/** Container for reward cards */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* CardsContainer;
	
	/** Timer progress bar */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* TimerBar;
	
	/** Time remaining text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TimeRemainingText;
	
	/** Skip button (if allowed) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* SkipButton;
	
	/** Skip button text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* SkipButtonText;
	
	// ========================================
	// CONFIGURATION
	// ========================================
	
	/** Time allowed for selection (0 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float SelectionTimeout = 15.0f;
	
	/** Whether player can skip reward selection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bAllowSkip = false;
	
	/** Widget class to use for reward cards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TSubclassOf<URewardCardWidget> RewardCardClass;
	
	// ========================================
	// RUNTIME DATA
	// ========================================
	
	/** Reference to the reward selection component */
	UPROPERTY(BlueprintReadOnly)
	URewardSelectionComponent* SelectionComponent;
	
	/** Current reward choices */
	UPROPERTY(BlueprintReadOnly)
	TArray<URewardDataAsset*> CurrentChoices;
	
	/** Created card widgets */
	UPROPERTY(BlueprintReadOnly)
	TArray<URewardCardWidget*> RewardCards;
	
	/** Time remaining for selection */
	UPROPERTY(BlueprintReadOnly)
	float TimeRemaining;
	
	// ========================================
	// DELEGATES
	// ========================================
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardSelected, URewardDataAsset*, SelectedReward);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectionSkipped);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectionTimeout);
	
	/** Called when a reward is selected */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRewardSelected OnRewardSelected;
	
	/** Called when selection is skipped */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSelectionSkipped OnSelectionSkipped;
	
	/** Called when selection times out */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSelectionTimeout OnSelectionTimeout;
	
	// ========================================
	// FUNCTIONS
	// ========================================
	
	/** Initialize with reward selection component */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	void InitializeWithComponent(URewardSelectionComponent* Component);
	
	/** Present reward choices to player */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	void PresentRewardChoices(const TArray<URewardDataAsset*>& Rewards);
	
	/** Select a specific reward */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	void SelectReward(URewardDataAsset* Reward);
	
	/** Skip reward selection */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	void SkipSelection();
	
	/** Clear current selection */
	UFUNCTION(BlueprintCallable, Category = "Reward Selection")
	void ClearSelection();
	
	/** Play intro animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animations")
	void PlayIntroAnimation();
	
	/** Play outro animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animations")
	void PlayOutroAnimation();
	
	/** Play selection made animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animations")
	void PlaySelectionMadeAnimation();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	/** Handle skip button click */
	UFUNCTION()
	void OnSkipButtonClicked();
	
	/** Create reward card widgets */
	void CreateRewardCards();
	
	/** Handle timeout */
	void HandleTimeout();
	
	/** Update timer display */
	void UpdateTimerDisplay();
	
	/** Timer handle for auto-selection */
	FTimerHandle TimeoutTimerHandle;
};