#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Atlas/Data/RewardDataAsset.h"

// Forward declarations
class URewardDataAsset;
class URunManagerComponent;

/**
 * Slate widget for reward selection after defeating enemies
 */
class ATLAS_API SRewardSelectionWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRewardSelectionWidget)
	{}
		/** The rewards to choose from */
		SLATE_ARGUMENT(TArray<URewardDataAsset*>, RewardChoices)
		/** Callback when a reward is selected */
		SLATE_EVENT(FSimpleDelegate, OnRewardSelected)
		/** Callback when selection is cancelled */
		SLATE_EVENT(FSimpleDelegate, OnSelectionCancelled)
		/** Reference to the run manager */
		SLATE_ARGUMENT(URunManagerComponent*, RunManager)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	/** Handle reward selection */
	FReply OnRewardButtonClicked(int32 RewardIndex);
	
	/** Handle cancel button */
	FReply OnCancelButtonClicked();
	
	/** Create reward button widget */
	TSharedRef<SWidget> CreateRewardButton(URewardDataAsset* Reward, int32 Index);
	
	/** Get reward button text */
	FText GetRewardButtonText(URewardDataAsset* Reward, int32 Index) const;
	
	/** Get reward description text */
	FText GetRewardDescriptionText(URewardDataAsset* Reward) const;
	
	/** Create the reward selection area */
	TSharedRef<SWidget> CreateRewardSelectionArea();
	
	/** Get category display text */
	FText GetCategoryDisplayText(ERewardCategory Category) const;
	
	/** Get category color */
	FLinearColor GetCategoryColor(ERewardCategory Category) const;

private:
	/** The reward choices */
	TArray<URewardDataAsset*> RewardChoices;
	
	/** Callback delegates */
	FSimpleDelegate OnRewardSelectedDelegate;
	FSimpleDelegate OnSelectionCancelledDelegate;
	
	/** Reference to run manager */
	URunManagerComponent* RunManagerRef;
	
	/** Selected reward index (-1 if none selected) */
	int32 SelectedRewardIndex = -1;
};