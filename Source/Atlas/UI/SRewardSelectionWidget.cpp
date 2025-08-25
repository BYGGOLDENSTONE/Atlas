#include "SRewardSelectionWidget.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Atlas/Components/RunManagerComponent.h"
#include "Slate.h"
#include "Engine/Engine.h"

void SRewardSelectionWidget::Construct(const FArguments& InArgs)
{
	RewardChoices = InArgs._RewardChoices;
	OnRewardSelectedDelegate = InArgs._OnRewardSelected;
	OnSelectionCancelledDelegate = InArgs._OnSelectionCancelled;
	RunManagerRef = InArgs._RunManager;

	// Create the main layout
	ChildSlot
	[
		// Full screen overlay with semi-transparent background
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0, 0, 0, 0.7f)) // Semi-transparent black background
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				// Main reward selection panel
				SNew(SBox)
				.WidthOverride(800.0f)
				.HeightOverride(400.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.9f)) // Dark panel background
					.BorderImage(FCoreStyle::Get().GetBrush("Border"))
					[
						SNew(SVerticalBox)
						
						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(20.0f, 20.0f, 20.0f, 10.0f)
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString("Choose Your Reward"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
							.ColorAndOpacity(FLinearColor::White)
						]
						
						// Reward selection area
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.Padding(20.0f, 10.0f)
						[
							CreateRewardSelectionArea()
						]
						
						// Bottom buttons
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(20.0f, 10.0f, 20.0f, 20.0f)
						.HAlign(HAlign_Center)
						[
							SNew(SHorizontalBox)
							
							// Cancel button
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(10.0f, 0.0f)
							[
								SNew(SButton)
								.Text(FText::FromString("Cancel (Skip Reward)"))
								.OnClicked(this, &SRewardSelectionWidget::OnCancelButtonClicked)
								.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
							]
						]
					]
				]
			]
		]
	];
}

TSharedRef<SWidget> SRewardSelectionWidget::CreateRewardSelectionArea()
{
	TSharedRef<SHorizontalBox> RewardBox = SNew(SHorizontalBox);
	
	// Create reward buttons
	for (int32 i = 0; i < RewardChoices.Num(); i++)
	{
		if (RewardChoices[i])
		{
			RewardBox->AddSlot()
			.FillWidth(1.0f)
			.Padding(10.0f, 0.0f)
			[
				CreateRewardButton(RewardChoices[i], i)
			];
		}
	}
	
	return RewardBox;
}

TSharedRef<SWidget> SRewardSelectionWidget::CreateRewardButton(URewardDataAsset* Reward, int32 Index)
{
	if (!Reward)
	{
		return SNew(SBox);
	}
	
	return SNew(SButton)
		.OnClicked(this, &SRewardSelectionWidget::OnRewardButtonClicked, Index)
		.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
		.ContentPadding(FMargin(20.0f, 15.0f))
		[
			SNew(SVerticalBox)
			
			// Reward name
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				SNew(STextBlock)
				.Text(Reward->RewardName)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
				.ColorAndOpacity(FLinearColor::White)
				.Justification(ETextJustify::Center)
			]
			
			// Reward description
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(Reward->Description)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
			]
			
			// Category
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(GetCategoryDisplayText(Reward->Category))
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 12))
				.ColorAndOpacity(GetCategoryColor(Reward->Category))
				.Justification(ETextJustify::Center)
			]
		];
}

FText SRewardSelectionWidget::GetCategoryDisplayText(ERewardCategory Category) const
{
	switch (Category)
	{
		case ERewardCategory::Defense:
			return FText::FromString("Defense");
		case ERewardCategory::Offense:
			return FText::FromString("Offense");
		case ERewardCategory::PassiveStats:
			return FText::FromString("Passive Stats");
		case ERewardCategory::PassiveAbility:
			return FText::FromString("Passive Ability");
		case ERewardCategory::Interactable:
			return FText::FromString("Interactable");
		default:
			return FText::FromString("Unknown");
	}
}

FLinearColor SRewardSelectionWidget::GetCategoryColor(ERewardCategory Category) const
{
	switch (Category)
	{
		case ERewardCategory::Defense:
			return FLinearColor::Blue;
		case ERewardCategory::Offense:
			return FLinearColor::Red;
		case ERewardCategory::PassiveStats:
			return FLinearColor::Green;
		case ERewardCategory::PassiveAbility:
			return FLinearColor::Yellow;
		case ERewardCategory::Interactable:
			return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
		default:
			return FLinearColor::White;
	}
}

FReply SRewardSelectionWidget::OnRewardButtonClicked(int32 RewardIndex)
{
	if (RewardIndex >= 0 && RewardIndex < RewardChoices.Num())
	{
		SelectedRewardIndex = RewardIndex;
		
		// Log the selection
		if (RewardChoices[RewardIndex])
		{
			UE_LOG(LogTemp, Warning, TEXT("Player selected reward: %s"), *RewardChoices[RewardIndex]->RewardName.ToString());
		}
		
		// Call the run manager to handle the selection
		if (RunManagerRef)
		{
			RunManagerRef->SelectReward(RewardIndex);
		}
		
		// Execute callback
		OnRewardSelectedDelegate.ExecuteIfBound();
	}
	
	return FReply::Handled();
}

FReply SRewardSelectionWidget::OnCancelButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Player cancelled reward selection"));
	
	// Call the run manager to handle cancellation
	if (RunManagerRef)
	{
		RunManagerRef->CancelRewardSelection();
	}
	
	// Execute callback
	OnSelectionCancelledDelegate.ExecuteIfBound();
	
	return FReply::Handled();
}