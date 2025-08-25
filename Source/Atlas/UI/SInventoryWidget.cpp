#include "SInventoryWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SButton.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Styling/SlateColor.h"
#include "Engine/Engine.h"

// Static definitions
const TArray<FString> SInventoryWidget::SlotNames = {
	TEXT("Head"),
	TEXT("Body"),
	TEXT("Arms"),
	TEXT("Legs"),
	TEXT("Accessory")
};

const TArray<FKey> SInventoryWidget::SlotKeyBinds = {
	EKeys::One,
	EKeys::Two,
	EKeys::Three,
	EKeys::Four,
	EKeys::Five
};

void SInventoryWidget::Construct(const FArguments& InArgs)
{
	SlotManagerRef = InArgs._SlotManager;
	SelectedReward = InArgs._SelectedReward;
	OnRewardEquippedDelegate = InArgs._OnRewardEquipped;
	OnBackToRewardSelectionDelegate = InArgs._OnBackToRewardSelection;
	OnCancelEquipDelegate = InArgs._OnCancelEquip;
	
	HoveredSlotIndex = -1;
	
	// Create the inventory modal
	ChildSlot
	[
		SNew(SOverlay)
		
		// Dark background overlay
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0, 0, 0, 0.8f))
		]
		
		// Main inventory panel
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(900.0f)
			.HeightOverride(600.0f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.95f))
				.Padding(FMargin(20.0f))
				[
					SNew(SVerticalBox)
					
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 15)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("EQUIP REWARD")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
						.ColorAndOpacity(FSlateColor(FLinearColor::White))
						.Justification(ETextJustify::Center)
					]
					
					// Main content area
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SHorizontalBox)
						
						// Left: Selected reward details
						+ SHorizontalBox::Slot()
						.FillWidth(0.4f)
						.Padding(0, 0, 15, 0)
						[
							CreateSelectedRewardPanel()
						]
						
						// Right: Slot selection
						+ SHorizontalBox::Slot()
						.FillWidth(0.6f)
						[
							CreateSlotSelectionPanel()
						]
					]
					
					// Instructions
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 15, 0, 0)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Click a slot or press number keys 1-5 to equip the reward")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
						.Justification(ETextJustify::Center)
					]
				]
			]
		]
	];
}

TSharedRef<SWidget> SInventoryWidget::CreateSelectedRewardPanel()
{
	if (!SelectedReward)
	{
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f))
			.Padding(15.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("No reward selected")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
				.Justification(ETextJustify::Center)
			];
	}
	
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f))
		.Padding(15.0f)
		[
			SNew(SVerticalBox)
			
			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Selected Reward")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
			
			// Reward display
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(GetCategoryColor(SelectedReward->Category) * 0.3f)
				.Padding(15.0f)
				[
					SNew(SVerticalBox)
					
					// Reward name
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(SelectedReward->RewardName)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						.ColorAndOpacity(FSlateColor(FLinearColor::White))
						.Justification(ETextJustify::Center)
					]
					
					// Category
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 5, 0, 10)
					[
						SNew(STextBlock)
						.Text(FText::Format(FText::FromString(TEXT("[{0}]")), 
							FText::FromString(GetCategoryName(SelectedReward->Category))))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
						.ColorAndOpacity(FSlateColor(GetCategoryColor(SelectedReward->Category)))
						.Justification(ETextJustify::Center)
					]
					
					// Description
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 10)
					[
						SNew(STextBlock)
						.Text(SelectedReward->Description)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
						.AutoWrapText(true)
						.Justification(ETextJustify::Center)
					]
					
					// Stats
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(GetRewardStats(SelectedReward))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.8f, 1.0f)))
						.Justification(ETextJustify::Center)
					]
				]
			]
		];
}

TSharedRef<SWidget> SInventoryWidget::CreateSlotSelectionPanel()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f))
		.Padding(15.0f)
		[
			SNew(SVerticalBox)
			
			// Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Choose Equipment Slot")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
			
			// Instructions
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Click a slot to equip the reward there")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
			]
			
			// Slots
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(0.2f)
				.Padding(0, 3)
				[
					CreateSlotButton(0)
				]
				+ SVerticalBox::Slot()
				.FillHeight(0.2f)
				.Padding(0, 3)
				[
					CreateSlotButton(1)
				]
				+ SVerticalBox::Slot()
				.FillHeight(0.2f)
				.Padding(0, 3)
				[
					CreateSlotButton(2)
				]
				+ SVerticalBox::Slot()
				.FillHeight(0.2f)
				.Padding(0, 3)
				[
					CreateSlotButton(3)
				]
				+ SVerticalBox::Slot()
				.FillHeight(0.2f)
				.Padding(0, 3)
				[
					CreateSlotButton(4)
				]
			]
			
			// Action buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 15, 0, 0)
			[
				SNew(SHorizontalBox)
				
				// Back button
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0, 0, 5, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Back to Rewards")))
					.HAlign(HAlign_Center)
					.OnClicked(this, &SInventoryWidget::OnBackButtonClicked)
				]
				
				// Cancel button
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(5, 0, 0, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Skip Equipment")))
					.HAlign(HAlign_Center)
					.OnClicked(this, &SInventoryWidget::OnCancelButtonClicked)
				]
			]
		];
}

TSharedRef<SWidget> SInventoryWidget::CreateSlotButton(int32 SlotIndex)
{
	return SNew(SButton)
		.OnClicked(this, &SInventoryWidget::OnSlotClicked, SlotIndex)
		.ButtonColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f))
		.ContentPadding(0)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(this, &SInventoryWidget::GetSlotBorderColor, SlotIndex)
			.Padding(10.0f)
			[
				SNew(SHorizontalBox)
				
				// Key bind
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 10, 0)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f))
					.Padding(5.0f)
					[
						SNew(STextBlock)
						.Text(FText::AsNumber(SlotIndex + 1))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FSlateColor(FLinearColor::White))
					]
				]
				
				// Slot info
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					
					// Slot name
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(SlotNames[SlotIndex]))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
					]
					
					// Current reward
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(this, &SInventoryWidget::GetSlotText, SlotIndex)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(this, &SInventoryWidget::GetSlotTextColor, SlotIndex)
					]
					
					// Stats (if equipped)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text_Lambda([this, SlotIndex]()
						{
							URewardDataAsset* CurrentReward = GetRewardInSlot(SlotIndex);
							return CurrentReward ? GetRewardStats(CurrentReward) : FText::GetEmpty();
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.8f, 1.0f)))
					]
				]
			]
		];
}

void SInventoryWidget::SetSelectedReward(URewardDataAsset* Reward)
{
	SelectedReward = Reward;
	// Invalidate to refresh the display
	Invalidate(EInvalidateWidgetReason::Layout);
}

bool SInventoryWidget::HandleKeyPress(const FKey& Key)
{
	// Check for number keys 1-5
	for (int32 i = 0; i < MaxSlots && i < SlotKeyBinds.Num(); i++)
	{
		if (Key == SlotKeyBinds[i])
		{
			EquipRewardToSlot(i);
			return true;
		}
	}
	
	// ESC to cancel
	if (Key == EKeys::Escape)
	{
		OnCancelButtonClicked();
		return true;
	}
	
	return false;
}

FReply SInventoryWidget::OnSlotClicked(int32 SlotIndex)
{
	EquipRewardToSlot(SlotIndex);
	return FReply::Handled();
}

FReply SInventoryWidget::OnBackButtonClicked()
{
	OnBackToRewardSelectionDelegate.ExecuteIfBound();
	return FReply::Handled();
}

FReply SInventoryWidget::OnCancelButtonClicked()
{
	OnCancelEquipDelegate.ExecuteIfBound();
	return FReply::Handled();
}

void SInventoryWidget::EquipRewardToSlot(int32 SlotIndex)
{
	if (!SlotManagerRef || !SelectedReward || SlotIndex < 0 || SlotIndex >= MaxSlots)
		return;
	
	// Check if same reward is already in this slot (for enhancement)
	URewardDataAsset* CurrentReward = GetRewardInSlot(SlotIndex);
	if (CurrentReward && CurrentReward->RewardTag == SelectedReward->RewardTag)
	{
		// Enhance the existing reward
		SlotManagerRef->EnhanceReward(SelectedReward->RewardTag);
		UE_LOG(LogTemp, Log, TEXT("Enhanced %s in slot %d"), *SelectedReward->RewardName.ToString(), SlotIndex);
	}
	else
	{
		// Equip new reward (will replace if slot is occupied)
		SlotManagerRef->EquipReward(SelectedReward, SlotIndex);
		UE_LOG(LogTemp, Log, TEXT("Equipped %s to slot %d"), *SelectedReward->RewardName.ToString(), SlotIndex);
	}
	
	// Fire the delegate to close the inventory
	OnRewardEquippedDelegate.ExecuteIfBound();
}

URewardDataAsset* SInventoryWidget::GetRewardInSlot(int32 SlotIndex) const
{
	if (!SlotManagerRef || SlotIndex < 0 || SlotIndex >= MaxSlots)
		return nullptr;
	
	return SlotManagerRef->GetRewardInSlot(SlotIndex);
}

FText SInventoryWidget::GetSlotText(int32 SlotIndex) const
{
	URewardDataAsset* Reward = GetRewardInSlot(SlotIndex);
	if (Reward)
	{
		return Reward->RewardName;
	}
	return FText::FromString(TEXT("<Empty>"));
}

FText SInventoryWidget::GetRewardStats(URewardDataAsset* Reward) const
{
	if (!Reward)
		return FText::GetEmpty();
	
	FString StatsString;
	
	// Add stat modifiers
	for (const auto& StatMod : Reward->StatModifiers)
	{
		if (!StatsString.IsEmpty())
			StatsString += TEXT(", ");
		
		if (StatMod.Value > 0)
			StatsString += FString::Printf(TEXT("+%.0f%% %s"), StatMod.Value * 100, *StatMod.Key.ToString());
		else
			StatsString += FString::Printf(TEXT("%.0f%% %s"), StatMod.Value * 100, *StatMod.Key.ToString());
	}
	
	// Add slot cost if > 1
	if (Reward->SlotCost > 1)
	{
		if (!StatsString.IsEmpty())
			StatsString += TEXT(" | ");
		StatsString += FString::Printf(TEXT("Cost: %d slots"), Reward->SlotCost);
	}
	
	return FText::FromString(StatsString);
}

FSlateColor SInventoryWidget::GetSlotBorderColor(int32 SlotIndex) const
{
	if (SlotIndex == HoveredSlotIndex)
	{
		return FSlateColor(FLinearColor(0.2f, 0.6f, 1.0f));
	}
	
	URewardDataAsset* Reward = GetRewardInSlot(SlotIndex);
	if (Reward)
	{
		return FSlateColor(GetCategoryColor(Reward->Category) * 0.5f);
	}
	
	return FSlateColor(FLinearColor(0.1f, 0.1f, 0.1f));
}

FSlateColor SInventoryWidget::GetSlotTextColor(int32 SlotIndex) const
{
	URewardDataAsset* Reward = GetRewardInSlot(SlotIndex);
	if (Reward)
	{
		return FSlateColor(GetCategoryColor(Reward->Category));
	}
	return FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f));
}

FLinearColor SInventoryWidget::GetCategoryColor(ERewardCategory Category) const
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

FString SInventoryWidget::GetCategoryName(ERewardCategory Category) const
{
	switch (Category)
	{
		case ERewardCategory::Offense: return TEXT("Offense");
		case ERewardCategory::Defense: return TEXT("Defense");
		case ERewardCategory::PassiveAbility: return TEXT("Passive Ability");
		case ERewardCategory::PassiveStats: return TEXT("Passive Stats");
		case ERewardCategory::Interactable: return TEXT("Interactable");
		default: return TEXT("Unknown");
	}
}