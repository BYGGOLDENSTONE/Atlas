#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Atlas/Data/RewardDataAsset.h"

class USlotManagerComponent;
class URewardDataAsset;

/**
 * Compact slot manager widget - Always visible at bottom-right
 * Shows currently equipped rewards in a simple read-only display
 */
class ATLAS_API SSimpleSlotManagerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSimpleSlotManagerWidget) {}
		SLATE_ARGUMENT(USlotManagerComponent*, SlotManager)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	// Refresh the display when slots change
	void RefreshSlots();
	
	// Get the reward in a specific slot
	URewardDataAsset* GetRewardInSlot(int32 SlotIndex) const;

private:
	// Widget creation
	TSharedRef<SWidget> CreateSlotWidget(int32 SlotIndex);
	
	// Display helpers
	FText GetSlotText(int32 SlotIndex) const;
	FText GetSlotKeyBindText(int32 SlotIndex) const;
	FSlateColor GetSlotTextColor(int32 SlotIndex) const;
	FLinearColor GetCategoryColor(ERewardCategory Category) const;
	
private:
	// Core reference
	USlotManagerComponent* SlotManagerRef;
	
	// Slot configuration
	static constexpr int32 MaxSlots = 5; // Head, Body, Arms, Legs, Accessory
	static const TArray<FString> SlotNames;
};