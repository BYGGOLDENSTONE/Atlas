#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Atlas/Data/RewardDataAsset.h"

class USlotManagerComponent;
class URewardDataAsset;

/**
 * Inventory widget for reward slot selection
 * Opens as a modal after reward selection to allow the player to choose which slot to equip the reward
 */
class ATLAS_API SInventoryWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInventoryWidget) {}
		SLATE_ARGUMENT(USlotManagerComponent*, SlotManager)
		SLATE_ARGUMENT(URewardDataAsset*, SelectedReward)
		SLATE_EVENT(FSimpleDelegate, OnRewardEquipped)
		SLATE_EVENT(FSimpleDelegate, OnBackToRewardSelection)
		SLATE_EVENT(FSimpleDelegate, OnCancelEquip)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	// Set the reward to be equipped
	void SetSelectedReward(URewardDataAsset* Reward);
	
	// Handle keyboard input for slot selection
	bool HandleKeyPress(const FKey& Key);

private:
	// Widget creation
	TSharedRef<SWidget> CreateSelectedRewardPanel();
	TSharedRef<SWidget> CreateSlotSelectionPanel();
	TSharedRef<SWidget> CreateSlotButton(int32 SlotIndex);
	
	// Slot interaction
	FReply OnSlotClicked(int32 SlotIndex);
	FReply OnBackButtonClicked();
	FReply OnCancelButtonClicked();
	
	// Helper functions
	URewardDataAsset* GetRewardInSlot(int32 SlotIndex) const;
	FText GetSlotText(int32 SlotIndex) const;
	FText GetRewardStats(URewardDataAsset* Reward) const;
	FSlateColor GetSlotBorderColor(int32 SlotIndex) const;
	FSlateColor GetSlotTextColor(int32 SlotIndex) const;
	FLinearColor GetCategoryColor(ERewardCategory Category) const;
	FString GetCategoryName(ERewardCategory Category) const;
	
	// Equip the selected reward to a slot
	void EquipRewardToSlot(int32 SlotIndex);

private:
	// Core references
	USlotManagerComponent* SlotManagerRef;
	URewardDataAsset* SelectedReward;
	
	// Delegates
	FSimpleDelegate OnRewardEquippedDelegate;
	FSimpleDelegate OnBackToRewardSelectionDelegate;
	FSimpleDelegate OnCancelEquipDelegate;
	
	// Selection state
	int32 HoveredSlotIndex;
	
	// Slot configuration
	static constexpr int32 MaxSlots = 5;
	static const TArray<FString> SlotNames;
	static const TArray<FKey> SlotKeyBinds;
};