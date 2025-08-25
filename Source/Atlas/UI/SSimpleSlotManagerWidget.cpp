#include "SSimpleSlotManagerWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Styling/SlateColor.h"

// Static definitions
const TArray<FString> SSimpleSlotManagerWidget::SlotNames = {
	TEXT("Head"),
	TEXT("Body"),
	TEXT("Arms"),
	TEXT("Legs"),
	TEXT("Accessory")
};

void SSimpleSlotManagerWidget::Construct(const FArguments& InArgs)
{
	SlotManagerRef = InArgs._SlotManager;
	
	// Create compact display widget positioned at bottom-right
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0, 0, 20, 20)) // 20 pixels from right and bottom edges
		[
			SNew(SBox)
			.WidthOverride(280.0f)  // Similar width to run progress widget
			.HeightOverride(160.0f) // Compact height
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.85f))
				.Padding(FMargin(12.0f, 8.0f))
				[
					SNew(SVerticalBox)
					
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 5)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("EQUIPPED REWARDS")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
					]
					
					// Slot list
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 2)
						[
							CreateSlotWidget(0)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 2)
						[
							CreateSlotWidget(1)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 2)
						[
							CreateSlotWidget(2)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 2)
						[
							CreateSlotWidget(3)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 2)
						[
							CreateSlotWidget(4)
						]
					]
				]
			]
		]
	];
}

TSharedRef<SWidget> SSimpleSlotManagerWidget::CreateSlotWidget(int32 SlotIndex)
{
	return SNew(SHorizontalBox)
		
		// Key bind
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 0, 5, 0)
		[
			SNew(STextBlock)
			.Text(GetSlotKeyBindText(SlotIndex))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
		]
		
		// Slot name
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 0, 5, 0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(SlotNames[SlotIndex] + TEXT(":")))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		]
		
		// Reward name or <Empty>
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(this, &SSimpleSlotManagerWidget::GetSlotText, SlotIndex)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			.ColorAndOpacity(this, &SSimpleSlotManagerWidget::GetSlotTextColor, SlotIndex)
		];
}

void SSimpleSlotManagerWidget::RefreshSlots()
{
	// Force a visual refresh of all slots
	Invalidate(EInvalidateWidgetReason::Paint);
}

URewardDataAsset* SSimpleSlotManagerWidget::GetRewardInSlot(int32 SlotIndex) const
{
	if (!SlotManagerRef || SlotIndex < 0 || SlotIndex >= MaxSlots)
		return nullptr;
	
	return SlotManagerRef->GetRewardInSlot(SlotIndex);
}

FText SSimpleSlotManagerWidget::GetSlotText(int32 SlotIndex) const
{
	URewardDataAsset* Reward = GetRewardInSlot(SlotIndex);
	if (Reward)
	{
		return Reward->RewardName;
	}
	return FText::FromString(TEXT("<Empty>"));
}

FText SSimpleSlotManagerWidget::GetSlotKeyBindText(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < MaxSlots)
	{
		return FText::Format(FText::FromString(TEXT("[{0}]")), FText::AsNumber(SlotIndex + 1));
	}
	return FText::GetEmpty();
}

FSlateColor SSimpleSlotManagerWidget::GetSlotTextColor(int32 SlotIndex) const
{
	URewardDataAsset* Reward = GetRewardInSlot(SlotIndex);
	if (Reward)
	{
		return FSlateColor(GetCategoryColor(Reward->Category));
	}
	return FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f));
}

FLinearColor SSimpleSlotManagerWidget::GetCategoryColor(ERewardCategory Category) const
{
	switch (Category)
	{
		case ERewardCategory::Offense:
			return FLinearColor(1.0f, 0.3f, 0.3f);
		case ERewardCategory::Defense:
			return FLinearColor(0.3f, 0.5f, 1.0f);
		case ERewardCategory::PassiveAbility:
			return FLinearColor(0.8f, 0.3f, 0.8f);
		case ERewardCategory::PassiveStats:
			return FLinearColor(0.3f, 0.8f, 0.3f);
		case ERewardCategory::Interactable:
			return FLinearColor(0.8f, 0.6f, 0.3f);
		default:
			return FLinearColor(0.5f, 0.5f, 0.5f);
	}
}