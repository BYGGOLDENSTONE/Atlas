#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class USlotManagerComponent;
class URewardDataAsset;

/**
 * Slate widget for displaying and managing equipped rewards in slots
 */
class ATLAS_API SSlotManagerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSlotManagerWidget)
	{}
		SLATE_ARGUMENT(USlotManagerComponent*, SlotManager)
		SLATE_EVENT(FSimpleDelegate, OnSlotChanged)
		SLATE_EVENT(FSimpleDelegate, OnRewardEquipped)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	void RefreshSlots();
	void UpdateSlot(int32 SlotIndex);
	void SelectSlot(int32 SlotIndex);
	void ClearSlot(int32 SlotIndex);
	void EquipReward(URewardDataAsset* Reward, int32 SlotIndex);
	
	int32 GetSelectedSlotIndex() const { return SelectedSlotIndex; }
	URewardDataAsset* GetRewardInSlot(int32 SlotIndex) const;

private:
	TSharedRef<SWidget> CreateSlotGrid();
	TSharedRef<SWidget> CreateSlotWidget(int32 SlotIndex);
	TSharedRef<SWidget> CreateInfoPanel();
	
	FReply OnSlotClicked(int32 SlotIndex);
	FReply OnClearSlotClicked();
	FReply OnEquipRewardClicked();
	
	FText GetSlotText(int32 SlotIndex) const;
	FText GetSelectedRewardName() const;
	FText GetSelectedRewardDescription() const;
	FText GetSelectedRewardStats() const;
	
	FSlateColor GetSlotBorderColor(int32 SlotIndex) const;
	FSlateColor GetSlotTextColor(int32 SlotIndex) const;
	
	EVisibility GetInfoPanelVisibility() const;
	EVisibility GetSlotContentVisibility(int32 SlotIndex) const;
	EVisibility GetEmptySlotVisibility(int32 SlotIndex) const;

private:
	USlotManagerComponent* SlotManagerRef;
	
	FSimpleDelegate OnSlotChangedDelegate;
	FSimpleDelegate OnRewardEquippedDelegate;
	
	int32 SelectedSlotIndex;
	URewardDataAsset* PendingReward;
	
	TArray<TSharedPtr<SWidget>> SlotWidgets;
	
	static constexpr int32 MaxSlots = 6;
	static constexpr int32 GridColumns = 3;
};