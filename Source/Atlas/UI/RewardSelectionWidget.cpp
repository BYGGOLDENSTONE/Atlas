#include "RewardSelectionWidget.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Atlas/Components/RewardSelectionComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Border.h"
#include "TimerManager.h"

// ========================================
// REWARD CARD WIDGET
// ========================================

void URewardCardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &URewardCardWidget::OnSelectButtonClicked);
		SelectButton->OnHovered.AddDynamic(this, &URewardCardWidget::OnSelectButtonHovered);
	}
}

void URewardCardWidget::SetupCard(URewardDataAsset* InReward, bool bEnhancing, int32 StackLevel)
{
	if (!InReward) return;
	
	RewardData = InReward;
	bIsEnhancement = bEnhancing;
	CurrentStackLevel = StackLevel;
	
	// Set icon
	if (RewardIcon && RewardData->Icon)
	{
		RewardIcon->SetBrushFromTexture(RewardData->Icon);
	}
	
	// Set name
	if (RewardName)
	{
		if (bIsEnhancement)
		{
			FText EnhancedName = FText::Format(
				NSLOCTEXT("RewardCard", "EnhancedName", "{0} +{1}"),
				RewardData->RewardName,
				FText::AsNumber(CurrentStackLevel)
			);
			RewardName->SetText(EnhancedName);
		}
		else
		{
			RewardName->SetText(RewardData->RewardName);
		}
	}
	
	// Set description
	if (RewardDescription)
	{
		if (bIsEnhancement && CurrentStackLevel > 1)
		{
			RewardDescription->SetText(RewardData->GetStackedDescription(CurrentStackLevel));
		}
		else
		{
			RewardDescription->SetText(RewardData->Description);
		}
	}
	
	// Set slot cost
	if (SlotCostText)
	{
		FText SlotText = FText::Format(
			NSLOCTEXT("RewardCard", "SlotCost", "{0} Slot{0}|plural(one=,other=s)"),
			FText::AsNumber(RewardData->SlotCost)
		);
		SlotCostText->SetText(SlotText);
	}
	
	// Set stack level indicator
	if (StackLevelText)
	{
		if (bIsEnhancement)
		{
			StackLevelText->SetVisibility(ESlateVisibility::Visible);
			FText StackText = FText::Format(
				NSLOCTEXT("RewardCard", "StackLevel", "Level {0}/{1}"),
				FText::AsNumber(CurrentStackLevel),
				FText::AsNumber(RewardData->MaxStackLevel)
			);
			StackLevelText->SetText(StackText);
		}
		else
		{
			StackLevelText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	// Set category color
	if (CategoryColorBar)
	{
		CategoryColorBar->SetColorAndOpacity(GetCategoryColor());
	}
}

FLinearColor URewardCardWidget::GetCategoryColor() const
{
	if (!RewardData) return FLinearColor::White;
	
	switch (RewardData->Category)
	{
		case ERewardCategory::Defense:
			return FLinearColor(0.2f, 0.4f, 1.0f, 1.0f); // Blue
		case ERewardCategory::Offense:
			return FLinearColor(1.0f, 0.2f, 0.2f, 1.0f); // Red
		case ERewardCategory::PassiveStats:
			return FLinearColor(0.2f, 1.0f, 0.2f, 1.0f); // Green
		case ERewardCategory::PassiveAbility:
			return FLinearColor(1.0f, 0.8f, 0.2f, 1.0f); // Yellow
		case ERewardCategory::Interactable:
			return FLinearColor(0.8f, 0.2f, 1.0f, 1.0f); // Purple
		default:
			return FLinearColor::White;
	}
}

void URewardCardWidget::OnSelectButtonClicked()
{
	// Find the parent selection widget in the widget tree
	UWidget* Parent = GetParent();
	while (Parent)
	{
		if (URewardSelectionWidget* ParentWidget = Cast<URewardSelectionWidget>(Parent))
		{
			PlaySelectionAnimation();
			ParentWidget->SelectReward(RewardData);
			break;
		}
		Parent = Parent->GetParent();
	}
}

void URewardCardWidget::OnSelectButtonHovered()
{
	PlayHoverAnimation();
}

// ========================================
// REWARD SELECTION WIDGET
// ========================================

void URewardSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (SkipButton)
	{
		SkipButton->OnClicked.AddDynamic(this, &URewardSelectionWidget::OnSkipButtonClicked);
		
		// Hide skip button if not allowed
		if (!bAllowSkip)
		{
			SkipButton->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	// Initialize timer
	TimeRemaining = SelectionTimeout;
	UpdateTimerDisplay();
}

void URewardSelectionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Update timer if active
	if (SelectionTimeout > 0 && TimeRemaining > 0)
	{
		TimeRemaining -= InDeltaTime;
		UpdateTimerDisplay();
		
		if (TimeRemaining <= 0)
		{
			HandleTimeout();
		}
	}
}

void URewardSelectionWidget::InitializeWithComponent(URewardSelectionComponent* Component)
{
	SelectionComponent = Component;
	
	if (SelectionComponent)
	{
		// Bind to component events if needed
	}
}

void URewardSelectionWidget::PresentRewardChoices(const TArray<URewardDataAsset*>& Rewards)
{
	CurrentChoices = Rewards;
	
	// Clear existing cards
	ClearSelection();
	
	// Create new cards
	CreateRewardCards();
	
	// Reset timer
	TimeRemaining = SelectionTimeout;
	UpdateTimerDisplay();
	
	// Play intro animation
	PlayIntroAnimation();
	
	// Start timeout timer if applicable
	if (SelectionTimeout > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			TimeoutTimerHandle,
			this,
			&URewardSelectionWidget::HandleTimeout,
			SelectionTimeout,
			false
		);
	}
}

void URewardSelectionWidget::SelectReward(URewardDataAsset* Reward)
{
	if (!Reward) return;
	
	// Cancel timeout timer
	GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	
	// Play selection animation
	PlaySelectionMadeAnimation();
	
	// Notify listeners
	OnRewardSelected.Broadcast(Reward);
	
	// Notify component
	if (SelectionComponent)
	{
		SelectionComponent->OnRewardChosen(Reward);
	}
	
	// Play outro animation and remove widget
	PlayOutroAnimation();
	
	// Remove widget after animation
	FTimerHandle RemoveHandle;
	GetWorld()->GetTimerManager().SetTimer(
		RemoveHandle,
		[this]()
		{
			RemoveFromParent();
		},
		1.0f, // Outro animation duration
		false
	);
}

void URewardSelectionWidget::SkipSelection()
{
	// Cancel timeout timer
	GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	
	// Notify listeners
	OnSelectionSkipped.Broadcast();
	
	// Notify component
	if (SelectionComponent)
	{
		SelectionComponent->OnSelectionSkipped();
	}
	
	// Play outro animation and remove widget
	PlayOutroAnimation();
	
	// Remove widget after animation
	FTimerHandle RemoveHandle;
	GetWorld()->GetTimerManager().SetTimer(
		RemoveHandle,
		[this]()
		{
			RemoveFromParent();
		},
		1.0f,
		false
	);
}

void URewardSelectionWidget::ClearSelection()
{
	// Remove all existing cards
	if (CardsContainer)
	{
		CardsContainer->ClearChildren();
	}
	
	RewardCards.Empty();
}

void URewardSelectionWidget::CreateRewardCards()
{
	if (!CardsContainer || !RewardCardClass) return;
	
	for (URewardDataAsset* Reward : CurrentChoices)
	{
		if (!Reward) continue;
		
		// Create card widget
		URewardCardWidget* NewCard = CreateWidget<URewardCardWidget>(this, RewardCardClass);
		if (!NewCard) continue;
		
		// Check if this is an enhancement (player already has this reward)
		bool bIsEnhancement = false;
		int32 CurrentStack = 1;
		
		// TODO: Check with SlotManager if player already has this reward
		// if (SlotManager && SlotManager->HasReward(Reward->RewardTag))
		// {
		//     bIsEnhancement = true;
		//     CurrentStack = SlotManager->GetRewardStackLevel(Reward->RewardTag);
		// }
		
		// Setup card
		NewCard->SetupCard(Reward, bIsEnhancement, CurrentStack);
		
		// Add to container
		UHorizontalBoxSlot* CardSlot = CardsContainer->AddChildToHorizontalBox(NewCard);
		if (CardSlot)
		{
			CardSlot->SetPadding(FMargin(10.0f));
			CardSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}
		
		// Store reference
		RewardCards.Add(NewCard);
	}
}

void URewardSelectionWidget::OnSkipButtonClicked()
{
	SkipSelection();
}

void URewardSelectionWidget::HandleTimeout()
{
	// Auto-select first reward or skip
	if (CurrentChoices.Num() > 0)
	{
		// Auto-select first reward
		SelectReward(CurrentChoices[0]);
	}
	else
	{
		// Skip if no rewards available
		SkipSelection();
	}
	
	// Notify listeners
	OnSelectionTimeout.Broadcast();
}

void URewardSelectionWidget::UpdateTimerDisplay()
{
	if (TimerBar)
	{
		float Progress = (SelectionTimeout > 0) ? (TimeRemaining / SelectionTimeout) : 1.0f;
		TimerBar->SetPercent(Progress);
	}
	
	if (TimeRemainingText)
	{
		int32 SecondsRemaining = FMath::CeilToInt(TimeRemaining);
		FText TimeText = FText::Format(
			NSLOCTEXT("RewardSelection", "TimeRemaining", "{0}s"),
			FText::AsNumber(SecondsRemaining)
		);
		TimeRemainingText->SetText(TimeText);
		
		// Change color as time runs out
		if (TimeRemaining < 5.0f)
		{
			TimeRemainingText->SetColorAndOpacity(FLinearColor(1.0f, 0.2f, 0.2f, 1.0f)); // Red
		}
		else if (TimeRemaining < 10.0f)
		{
			TimeRemainingText->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 0.2f, 1.0f)); // Yellow
		}
		else
		{
			TimeRemainingText->SetColorAndOpacity(FLinearColor::White);
		}
	}
}