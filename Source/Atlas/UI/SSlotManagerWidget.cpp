#include "SSlotManagerWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Styling/SlateColor.h"

void SSlotManagerWidget::Construct(const FArguments& InArgs)
{
	SlotManagerRef = InArgs._SlotManager;
	OnSlotChangedDelegate = InArgs._OnSlotChanged;
	OnRewardEquippedDelegate = InArgs._OnRewardEquipped;
	
	SelectedSlotIndex = -1;
	PendingReward = nullptr;
	
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(10.0f))
		[
			SNew(SVerticalBox)
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("EQUIPMENT SLOTS")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
			
			+ SVerticalBox::Slot()
			.FillHeight(0.6f)
			.Padding(0, 0, 0, 10)
			[
				CreateSlotGrid()
			]
			
			+ SVerticalBox::Slot()
			.FillHeight(0.4f)
			[
				CreateInfoPanel()
			]
		]
	];
}

TSharedRef<SWidget> SSlotManagerWidget::CreateSlotGrid()
{
	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);
	
	SlotWidgets.Empty();
	
	for (int32 i = 0; i < MaxSlots; i++)
	{
		int32 Row = i / GridColumns;
		int32 Col = i % GridColumns;
		
		TSharedPtr<SWidget> SlotWidget = CreateSlotWidget(i);
		SlotWidgets.Add(SlotWidget);
		
		Grid->AddSlot(Col, Row)
		.Padding(2.0f)
		[
			SlotWidget.ToSharedRef()
		];
	}
	
	return Grid;
}

TSharedRef<SWidget> SSlotManagerWidget::CreateSlotWidget(int32 SlotIndex)
{
	return SNew(SButton)
		.OnClicked(this, &SSlotManagerWidget::OnSlotClicked, SlotIndex)
		.ButtonColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f))
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(this, &SSlotManagerWidget::GetSlotBorderColor, SlotIndex)
			.Padding(5.0f)
			[
				SNew(SBox)
				.WidthOverride(80)
				.HeightOverride(80)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SOverlay)
					
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SSlotManagerWidget::GetSlotText, SlotIndex)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						.ColorAndOpacity(this, &SSlotManagerWidget::GetSlotTextColor, SlotIndex)
						.Visibility(this, &SSlotManagerWidget::GetSlotContentVisibility, SlotIndex)
					]
					
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::Format(NSLOCTEXT("SlotManager", "SlotNumber", "Slot {0}"), FText::AsNumber(SlotIndex + 1)))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
						.Visibility(this, &SSlotManagerWidget::GetEmptySlotVisibility, SlotIndex)
					]
				]
			]
		];
}

TSharedRef<SWidget> SSlotManagerWidget::CreateInfoPanel()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f))
		.Padding(10.0f)
		.Visibility(this, &SSlotManagerWidget::GetInfoPanelVisibility)
		[
			SNew(SVerticalBox)
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SNew(STextBlock)
				.Text(this, &SSlotManagerWidget::GetSelectedRewardName)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
			
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(0, 0, 0, 10)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(STextBlock)
					.Text(this, &SSlotManagerWidget::GetSelectedRewardDescription)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
					.AutoWrapText(true)
				]
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 5, 0, 5)
			[
				SNew(STextBlock)
				.Text(this, &SSlotManagerWidget::GetSelectedRewardStats)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.8f, 1.0f)))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0, 0, 5, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Clear Slot")))
					.OnClicked(this, &SSlotManagerWidget::OnClearSlotClicked)
					.IsEnabled_Lambda([this]() { 
						return SelectedSlotIndex >= 0 && GetRewardInSlot(SelectedSlotIndex) != nullptr; 
					})
				]
				
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(5, 0, 0, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Equip Reward")))
					.OnClicked(this, &SSlotManagerWidget::OnEquipRewardClicked)
					.IsEnabled_Lambda([this]() { 
						return SelectedSlotIndex >= 0 && PendingReward != nullptr; 
					})
				]
			]
		];
}

void SSlotManagerWidget::RefreshSlots()
{
	if (!SlotManagerRef)
		return;
}

void SSlotManagerWidget::UpdateSlot(int32 SlotIndex)
{
	if (!SlotManagerRef || SlotIndex < 0 || SlotIndex >= MaxSlots)
		return;
}

void SSlotManagerWidget::SelectSlot(int32 SlotIndex)
{
	if (SlotIndex >= 0 && SlotIndex < MaxSlots)
	{
		SelectedSlotIndex = SlotIndex;
		OnSlotChangedDelegate.ExecuteIfBound();
	}
}

void SSlotManagerWidget::ClearSlot(int32 SlotIndex)
{
	if (!SlotManagerRef || SlotIndex < 0 || SlotIndex >= MaxSlots)
		return;
	
	SlotManagerRef->RemoveReward(SlotIndex);
	RefreshSlots();
}

void SSlotManagerWidget::EquipReward(URewardDataAsset* Reward, int32 SlotIndex)
{
	if (!SlotManagerRef || !Reward || SlotIndex < 0 || SlotIndex >= MaxSlots)
		return;
	
	SlotManagerRef->EquipReward(Reward, SlotIndex);
	RefreshSlots();
	OnRewardEquippedDelegate.ExecuteIfBound();
}

URewardDataAsset* SSlotManagerWidget::GetRewardInSlot(int32 SlotIndex) const
{
	if (!SlotManagerRef || SlotIndex < 0 || SlotIndex >= MaxSlots)
		return nullptr;
	
	return SlotManagerRef->GetRewardInSlot(SlotIndex);
}

FReply SSlotManagerWidget::OnSlotClicked(int32 SlotIndex)
{
	SelectSlot(SlotIndex);
	return FReply::Handled();
}

FReply SSlotManagerWidget::OnClearSlotClicked()
{
	if (SelectedSlotIndex >= 0)
	{
		ClearSlot(SelectedSlotIndex);
	}
	return FReply::Handled();
}

FReply SSlotManagerWidget::OnEquipRewardClicked()
{
	if (SelectedSlotIndex >= 0 && PendingReward)
	{
		EquipReward(PendingReward, SelectedSlotIndex);
		PendingReward = nullptr;
	}
	return FReply::Handled();
}

FText SSlotManagerWidget::GetSlotText(int32 SlotIndex) const
{
	URewardDataAsset* Reward = GetRewardInSlot(SlotIndex);
	if (Reward)
	{
		FString RewardNameStr = Reward->RewardName.ToString();
		FString ShortName = RewardNameStr.Len() > 8 ? 
			RewardNameStr.Left(6) + TEXT("...") : RewardNameStr;
		return FText::FromString(ShortName);
	}
	return FText::GetEmpty();
}

FText SSlotManagerWidget::GetSelectedRewardName() const
{
	if (SelectedSlotIndex >= 0)
	{
		URewardDataAsset* Reward = GetRewardInSlot(SelectedSlotIndex);
		if (Reward)
		{
			return Reward->RewardName;
		}
	}
	return FText::FromString(TEXT("No Reward Selected"));
}

FText SSlotManagerWidget::GetSelectedRewardDescription() const
{
	if (SelectedSlotIndex >= 0)
	{
		URewardDataAsset* Reward = GetRewardInSlot(SelectedSlotIndex);
		if (Reward)
		{
			return Reward->Description;
		}
	}
	return FText::FromString(TEXT("Select a slot to view reward details."));
}

FText SSlotManagerWidget::GetSelectedRewardStats() const
{
	if (SelectedSlotIndex >= 0)
	{
		URewardDataAsset* Reward = GetRewardInSlot(SelectedSlotIndex);
		if (Reward)
		{
			FString StatsString = FString::Printf(TEXT("Slot Cost: %d"), Reward->SlotCost);
			if (Reward->MaxStackLevel > 1)
			{
				StatsString += FString::Printf(TEXT(" | Max Stack Level: %d"), Reward->MaxStackLevel);
			}
			return FText::FromString(StatsString);
		}
	}
	return FText::GetEmpty();
}

FSlateColor SSlotManagerWidget::GetSlotBorderColor(int32 SlotIndex) const
{
	if (SlotIndex == SelectedSlotIndex)
	{
		return FSlateColor(FLinearColor(0.2f, 0.6f, 1.0f));
	}
	
	URewardDataAsset* Reward = GetRewardInSlot(SlotIndex);
	if (Reward)
	{
		switch (Reward->Category)
		{
			case ERewardCategory::Offense:
				return FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f, 0.5f));
			case ERewardCategory::Defense:
				return FSlateColor(FLinearColor(0.2f, 0.4f, 1.0f, 0.5f));
			case ERewardCategory::PassiveAbility:
				return FSlateColor(FLinearColor(0.8f, 0.2f, 0.8f, 0.5f));
			case ERewardCategory::PassiveStats:
				return FSlateColor(FLinearColor(0.2f, 0.8f, 0.2f, 0.5f));
			case ERewardCategory::Interactable:
				return FSlateColor(FLinearColor(0.8f, 0.6f, 0.2f, 0.5f));
			default:
				return FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f, 0.5f));
		}
	}
	
	return FSlateColor(FLinearColor(0.1f, 0.1f, 0.1f));
}

FSlateColor SSlotManagerWidget::GetSlotTextColor(int32 SlotIndex) const
{
	URewardDataAsset* Reward = GetRewardInSlot(SlotIndex);
	if (Reward)
	{
		return FSlateColor(FLinearColor::White);
	}
	return FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f));
}

EVisibility SSlotManagerWidget::GetInfoPanelVisibility() const
{
	return SelectedSlotIndex >= 0 ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SSlotManagerWidget::GetSlotContentVisibility(int32 SlotIndex) const
{
	return GetRewardInSlot(SlotIndex) != nullptr ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SSlotManagerWidget::GetEmptySlotVisibility(int32 SlotIndex) const
{
	return GetRewardInSlot(SlotIndex) == nullptr ? EVisibility::Visible : EVisibility::Collapsed;
}