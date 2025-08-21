#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "GameplayTagContainer.h"
#include "SlotManagerWidget.generated.h"

// Forward declarations
class USlotManagerComponent;
class URewardDataAsset;
class UButton;
class UImage;
class UTextBlock;
class UBorder;
class UUniformGridPanel;
class UVerticalBox;
class UHorizontalBox;

/**
 * Drag and drop operation for reward slots
 */
UCLASS()
class ATLAS_API URewardDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** Source slot index */
	UPROPERTY()
	int32 SourceSlotIndex = -1;
	
	/** The reward being dragged */
	UPROPERTY()
	URewardDataAsset* DraggedReward = nullptr;
	
	/** Visual representation during drag */
	UPROPERTY()
	UUserWidget* DragVisual = nullptr;
};

/**
 * Individual slot widget for displaying and managing a single reward slot
 */
UCLASS()
class ATLAS_API USlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========================================
	// UI COMPONENTS
	// ========================================
	
	/** Border for the slot */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBorder* SlotBorder;
	
	/** Icon for the equipped reward */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* RewardIcon;
	
	/** Stack level indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* StackLevelText;
	
	/** Slot number label */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* SlotNumberText;
	
	/** Empty slot indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* EmptySlotImage;
	
	/** Multi-slot indicator (for rewards that take multiple slots) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* MultiSlotConnector;
	
	// ========================================
	// DATA
	// ========================================
	
	/** Index of this slot */
	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = -1;
	
	/** Currently equipped reward */
	UPROPERTY(BlueprintReadOnly)
	URewardDataAsset* EquippedReward = nullptr;
	
	/** Current stack level */
	UPROPERTY(BlueprintReadOnly)
	int32 StackLevel = 1;
	
	/** Whether this slot is occupied */
	UPROPERTY(BlueprintReadOnly)
	bool bIsOccupied = false;
	
	/** Whether this is a continuation of a multi-slot reward */
	UPROPERTY(BlueprintReadOnly)
	bool bIsMultiSlotContinuation = false;
	
	// ========================================
	// FUNCTIONS
	// ========================================
	
	/** Initialize this slot */
	UFUNCTION(BlueprintCallable, Category = "Slot")
	void InitializeSlot(int32 Index);
	
	/** Set the reward in this slot */
	UFUNCTION(BlueprintCallable, Category = "Slot")
	void SetReward(URewardDataAsset* Reward, int32 InStackLevel = 1);
	
	/** Clear this slot */
	UFUNCTION(BlueprintCallable, Category = "Slot")
	void ClearSlot();
	
	/** Update visual state */
	UFUNCTION(BlueprintCallable, Category = "Slot")
	void UpdateVisuals();
	
	/** Check if a reward can be placed in this slot */
	UFUNCTION(BlueprintPure, Category = "Slot")
	bool CanAcceptReward(URewardDataAsset* Reward) const;
	
	/** Play equip animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot")
	void PlayEquipAnimation();
	
	/** Play clear animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot")
	void PlayClearAnimation();
	
	/** Play highlight animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Slot")
	void PlayHighlightAnimation();

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
private:
	/** Create drag visual widget */
	UUserWidget* CreateDragVisual();
};

/**
 * Main slot manager widget for displaying and managing all reward slots
 */
UCLASS()
class ATLAS_API USlotManagerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========================================
	// UI COMPONENTS
	// ========================================
	
	/** Title text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TitleText;
	
	/** Grid panel containing all slots */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UUniformGridPanel* SlotGrid;
	
	/** Info panel for selected reward */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* InfoPanel;
	
	/** Selected reward name */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* SelectedRewardName;
	
	/** Selected reward description */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* SelectedRewardDescription;
	
	/** Stats preview panel */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* StatsPreview;
	
	/** Action buttons container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* ActionButtons;
	
	/** Equip button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* EquipButton;
	
	/** Replace button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ReplaceButton;
	
	/** Clear button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ClearButton;
	
	/** Cancel button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* CancelButton;
	
	// ========================================
	// CONFIGURATION
	// ========================================
	
	/** Number of slots to display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 TotalSlots = 6;
	
	/** Number of columns in the grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 GridColumns = 3;
	
	/** Widget class to use for slots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TSubclassOf<USlotWidget> SlotWidgetClass;
	
	// ========================================
	// RUNTIME DATA
	// ========================================
	
	/** Reference to the slot manager component */
	UPROPERTY(BlueprintReadOnly)
	USlotManagerComponent* SlotManagerComponent;
	
	/** All created slot widgets */
	UPROPERTY(BlueprintReadOnly)
	TArray<USlotWidget*> SlotWidgets;
	
	/** Currently selected slot */
	UPROPERTY(BlueprintReadOnly)
	int32 SelectedSlotIndex = -1;
	
	/** Reward pending placement */
	UPROPERTY(BlueprintReadOnly)
	URewardDataAsset* PendingReward = nullptr;
	
	// ========================================
	// DELEGATES
	// ========================================
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRewardEquipped, URewardDataAsset*, Reward, int32, SlotIndex);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRewardRemoved, URewardDataAsset*, Reward, int32, SlotIndex);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlotsSwapped, int32, Slot1, int32, Slot2, bool, Success);
	
	/** Called when a reward is equipped */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRewardEquipped OnRewardEquipped;
	
	/** Called when a reward is removed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRewardRemoved OnRewardRemoved;
	
	/** Called when slots are swapped */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSlotsSwapped OnSlotsSwapped;
	
	// ========================================
	// FUNCTIONS
	// ========================================
	
	/** Initialize with slot manager component */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void InitializeWithComponent(USlotManagerComponent* Component);
	
	/** Create all slot widgets */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void CreateSlots();
	
	/** Refresh all slots from component data */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void RefreshSlots();
	
	/** Select a specific slot */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void SelectSlot(int32 SlotIndex);
	
	/** Present a reward for placement */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void PresentRewardForPlacement(URewardDataAsset* Reward);
	
	/** Equip pending reward to selected slot */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void EquipToSelectedSlot();
	
	/** Replace reward in selected slot */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void ReplaceSelectedSlot();
	
	/** Clear selected slot */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void ClearSelectedSlot();
	
	/** Cancel current operation */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void CancelOperation();
	
	/** Swap two slots */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void SwapSlots(int32 Slot1, int32 Slot2);
	
	/** Update info panel for selected reward */
	UFUNCTION(BlueprintCallable, Category = "Slot Manager")
	void UpdateInfoPanel();
	
	/** Play slot swap animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animations")
	void PlaySwapAnimation(int32 Slot1, int32 Slot2);
	
	/** Play reward equipped animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Animations")
	void PlayEquippedAnimation(int32 SlotIndex);

protected:
	virtual void NativeConstruct() override;
	
private:
	/** Handle button clicks */
	UFUNCTION()
	void OnEquipButtonClicked();
	
	UFUNCTION()
	void OnReplaceButtonClicked();
	
	UFUNCTION()
	void OnClearButtonClicked();
	
	UFUNCTION()
	void OnCancelButtonClicked();
	
	/** Update button states */
	void UpdateButtonStates();
	
	/** Get slot widget at index */
	USlotWidget* GetSlotWidget(int32 Index) const;
};