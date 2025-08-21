#include "SlotManagerWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/UniformGridSlot.h"
#include "Blueprint/DragDropOperation.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Data/RewardDataAsset.h"

// ========================================
// USlotWidget Implementation
// ========================================

void USlotWidget::InitializeSlot(int32 Index)
{
	SlotIndex = Index;
	UpdateVisuals();
	
	if (SlotNumberText)
	{
		SlotNumberText->SetText(FText::AsNumber(SlotIndex + 1));
	}
}

void USlotWidget::SetReward(URewardDataAsset* Reward, int32 InStackLevel)
{
	EquippedReward = Reward;
	StackLevel = InStackLevel;
	bIsOccupied = (Reward != nullptr);
	
	if (EquippedReward)
	{
		if (RewardIcon && EquippedReward->Icon)
		{
			RewardIcon->SetBrushFromTexture(EquippedReward->Icon);
			RewardIcon->SetVisibility(ESlateVisibility::Visible);
		}
		
		// RewardNameText would be set in Blueprint if needed
		
		if (StackLevelText)
		{
			if (StackLevel > 1)
			{
				StackLevelText->SetText(FText::Format(NSLOCTEXT("SlotWidget", "StackLevel", "Lv.{0}"), 
					FText::AsNumber(StackLevel)));
				StackLevelText->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				StackLevelText->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		
		// Show multi-slot indicator if needed
		if (MultiSlotConnector && EquippedReward->SlotCost > 1)
		{
			MultiSlotConnector->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		ClearSlot();
	}
	
	UpdateVisuals();
}

void USlotWidget::ClearSlot()
{
	EquippedReward = nullptr;
	StackLevel = 0;
	bIsOccupied = false;
	
	if (RewardIcon) RewardIcon->SetVisibility(ESlateVisibility::Collapsed);
	if (StackLevelText) StackLevelText->SetVisibility(ESlateVisibility::Collapsed);
	if (MultiSlotConnector) MultiSlotConnector->SetVisibility(ESlateVisibility::Collapsed);
	
	UpdateVisuals();
}

void USlotWidget::UpdateVisuals()
{
	if (SlotBorder)
	{
		// Update border style based on state
		FLinearColor BorderColor = FLinearColor::Gray;
		
		if (bIsSelected)
		{
			BorderColor = FLinearColor::Yellow;
		}
		else if (bIsHighlighted)
		{
			BorderColor = bIsValidDropTarget ? FLinearColor::Green : FLinearColor::Red;
		}
		else if (bIsOccupied)
		{
			// Color based on reward category
			if (EquippedReward)
			{
				switch (EquippedReward->Category)
				{
					case ERewardCategory::Defense:
						BorderColor = FLinearColor::Blue;
						break;
					case ERewardCategory::Offense:
						BorderColor = FLinearColor::Red;
						break;
					case ERewardCategory::PassiveStats:
						BorderColor = FLinearColor::Green;
						break;
					case ERewardCategory::PassiveAbility:
						BorderColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange
						break;
					case ERewardCategory::Interactable:
						BorderColor = FLinearColor(0.5f, 0.0f, 1.0f); // Purple
						break;
				}
			}
		}
		
		SlotBorder->SetBrushColor(BorderColor);
	}
	
	// Update empty slot visuals
	if (EmptySlotImage)
	{
		EmptySlotImage->SetVisibility(bIsOccupied ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

bool USlotWidget::CanAcceptReward(URewardDataAsset* Reward) const
{
	if (!Reward)
		return false;
		
	// Can accept if slot is empty
	if (!bIsOccupied)
		return true;
		
	// Can accept if same reward (for stacking)
	if (EquippedReward && EquippedReward->RewardTag == Reward->RewardTag)
	{
		return StackLevel < EquippedReward->MaxStackLevel;
	}
	
	return false;
}

void USlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateVisuals();
}

FReply USlotWidget::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bIsOccupied && EquippedReward)
		{
			// Detect drag for occupied slots
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}
		else
		{
			// Just handle the click for empty slots
			OnSlotClicked.Broadcast(SlotIndex);
			return FReply::Handled();
		}
	}
	
	return FReply::Unhandled();
}

void USlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!bIsOccupied || !EquippedReward)
		return;
		
	// Create drag drop operation
	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();
	DragOp->Payload = this;
	DragOp->DefaultDragVisual = CreateDragVisual();
	DragOp->Pivot = EDragPivot::CenterCenter;
	
	OutOperation = DragOp;
	OnDragStarted.Broadcast(SlotIndex);
	
	// Set dragging visual state
	SetRenderOpacity(0.5f);
}

bool USlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (USlotWidget* DraggedSlot = Cast<USlotWidget>(InOperation->Payload))
	{
		if (DraggedSlot != this)
		{
			OnDropReceived.Broadcast(DraggedSlot->SlotIndex, SlotIndex);
			return true;
		}
	}
	
	return false;
}

void USlotWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (USlotWidget* DraggedSlot = Cast<USlotWidget>(InOperation->Payload))
	{
		bIsHighlighted = true;
		bIsValidDropTarget = (DraggedSlot != this);
		UpdateVisuals();
	}
}

void USlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	bIsHighlighted = false;
	bIsValidDropTarget = false;
	UpdateVisuals();
}

UWidget* USlotWidget::CreateDragVisual()
{
	// Create a simple drag visual - in a real implementation, 
	// this would be a proper widget showing the reward icon
	UImage* DragImage = NewObject<UImage>(this);
	if (EquippedReward && EquippedReward->Icon)
	{
		DragImage->SetBrushFromTexture(EquippedReward->Icon);
	}
	return DragImage;
}

// ========================================
// USlotManagerWidget Implementation
// ========================================

void USlotManagerWidget::InitializeWithComponent(USlotManagerComponent* Component)
{
	SlotManagerComponent = Component;
	
	if (SlotManagerComponent)
	{
		// Bind to slot manager events
		SlotManagerComponent->OnRewardEquipped.AddDynamic(this, &USlotManagerWidget::OnRewardEquippedHandler);
		SlotManagerComponent->OnRewardRemoved.AddDynamic(this, &USlotManagerWidget::OnRewardRemovedHandler);
		SlotManagerComponent->OnRewardEnhanced.AddDynamic(this, &USlotManagerWidget::OnRewardEnhancedHandler);
		SlotManagerComponent->OnSlotsChanged.AddDynamic(this, &USlotManagerWidget::RefreshSlots);
	}
	
	CreateSlots();
	RefreshSlots();
}

void USlotManagerWidget::CreateSlots()
{
	if (!SlotGrid || !SlotWidgetClass)
		return;
		
	SlotGrid->ClearChildren();
	SlotWidgets.Empty();
	
	// Create 6 slots in a 2x3 grid
	for (int32 i = 0; i < MaxSlots; ++i)
	{
		USlotWidget* NewSlot = CreateWidget<USlotWidget>(this, SlotWidgetClass);
		if (NewSlot)
		{
			NewSlot->InitializeSlot(i);
			
			// Bind slot events
			NewSlot->OnSlotClicked.AddDynamic(this, &USlotManagerWidget::OnSlotClickedHandler);
			NewSlot->OnDragStarted.AddDynamic(this, &USlotManagerWidget::OnSlotDragStartedHandler);
			NewSlot->OnDropReceived.AddDynamic(this, &USlotManagerWidget::OnSlotDropReceivedHandler);
			
			// Add to grid (2 columns, 3 rows)
			int32 Row = i / 2;
			int32 Column = i % 2;
			UUniformGridSlot* GridSlot = SlotGrid->AddChildToUniformGrid(NewSlot, Row, Column);
			if (GridSlot)
			{
				GridSlot->SetHorizontalAlignment(HAlign_Fill);
				GridSlot->SetVerticalAlignment(VAlign_Fill);
			}
			
			SlotWidgets.Add(NewSlot);
		}
	}
}

void USlotManagerWidget::RefreshSlots()
{
	if (!SlotManagerComponent)
		return;
		
	// Update each slot widget with current data
	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		if (SlotWidgets[i])
		{
			URewardDataAsset* Reward = SlotManagerComponent->GetRewardInSlot(i);
			int32 StackLevel = SlotManagerComponent->GetRewardStackLevel(i);
			SlotWidgets[i]->SetReward(Reward, StackLevel);
		}
	}
	
	UpdateInfoPanel();
	UpdateActionButtons();
}

void USlotManagerWidget::SelectSlot(int32 SlotIndex)
{
	// Deselect all slots
	for (USlotWidget* SlotWidget : SlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->SetSelected(false);
		}
	}
	
	// Select the specified slot
	if (SlotIndex >= 0 && SlotIndex < SlotWidgets.Num())
	{
		SelectedSlotIndex = SlotIndex;
		if (SlotWidgets[SlotIndex])
		{
			SlotWidgets[SlotIndex]->SetSelected(true);
		}
	}
	else
	{
		SelectedSlotIndex = -1;
	}
	
	UpdateInfoPanel();
	UpdateActionButtons();
}

void USlotManagerWidget::PresentRewardForPlacement(URewardDataAsset* Reward)
{
	PendingReward = Reward;
	bIsPlacingReward = true;
	
	// Update UI state
	if (InfoTitleText)
	{
		InfoTitleText->SetText(NSLOCTEXT("SlotManager", "PlaceReward", "Select a slot for this reward"));
	}
	
	if (InfoDescriptionText && Reward)
	{
		InfoDescriptionText->SetText(Reward->Description);
	}
	
	// Highlight valid slots
	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		if (SlotWidgets[i])
		{
			bool bCanPlace = false;
			
			// Check if this slot can accept the reward
			if (!SlotManagerComponent->GetRewardInSlot(i))
			{
				// Check if enough consecutive slots are available
				if (i + Reward->SlotCost <= MaxSlots)
				{
					bCanPlace = true;
					for (int32 j = i; j < i + Reward->SlotCost; ++j)
					{
						if (SlotManagerComponent->GetRewardInSlot(j))
						{
							bCanPlace = false;
							break;
						}
					}
				}
			}
			else if (SlotManagerComponent->GetRewardInSlot(i)->RewardTag == Reward->RewardTag)
			{
				// Can enhance existing reward
				bCanPlace = SlotManagerComponent->GetRewardStackLevel(i) < Reward->MaxStackLevel;
			}
			
			SlotWidgets[i]->SetHighlighted(bCanPlace);
		}
	}
	
	UpdateActionButtons();
}

void USlotManagerWidget::EquipToSelectedSlot()
{
	if (!SlotManagerComponent || !PendingReward || SelectedSlotIndex < 0)
		return;
		
	if (SlotManagerComponent->EquipReward(PendingReward, SelectedSlotIndex))
	{
		PendingReward = nullptr;
		bIsPlacingReward = false;
		RefreshSlots();
		OnRewardEquipped.Broadcast(PendingReward, SelectedSlotIndex);
	}
}

void USlotManagerWidget::ReplaceSelectedSlot()
{
	if (!SlotManagerComponent || !PendingReward || SelectedSlotIndex < 0)
		return;
		
	if (SlotManagerComponent->ReplaceReward(SelectedSlotIndex, PendingReward))
	{
		PendingReward = nullptr;
		bIsPlacingReward = false;
		RefreshSlots();
		OnRewardReplaced.Broadcast(PendingReward, SelectedSlotIndex);
	}
}

void USlotManagerWidget::ClearSelectedSlot()
{
	if (!SlotManagerComponent || SelectedSlotIndex < 0)
		return;
		
	if (SlotManagerComponent->RemoveReward(SelectedSlotIndex))
	{
		RefreshSlots();
		SelectSlot(-1);
	}
}

void USlotManagerWidget::CancelOperation()
{
	PendingReward = nullptr;
	bIsPlacingReward = false;
	SelectSlot(-1);
	
	// Clear all highlights
	for (USlotWidget* SlotWidget : SlotWidgets)
	{
		if (SlotWidget)
		{
			SlotWidget->SetHighlighted(false);
		}
	}
	
	UpdateInfoPanel();
	UpdateActionButtons();
}

void USlotManagerWidget::SwapSlots(int32 SlotA, int32 SlotB)
{
	if (!SlotManagerComponent)
		return;
		
	if (SlotManagerComponent->SwapRewards(SlotA, SlotB))
	{
		RefreshSlots();
		PlaySwapAnimation(SlotA, SlotB);
	}
}

void USlotManagerWidget::UpdateInfoPanel()
{
	if (!InfoPanel)
		return;
		
	if (bIsPlacingReward && PendingReward)
	{
		// Show pending reward info
		InfoPanel->SetVisibility(ESlateVisibility::Visible);
		
		if (InfoTitleText)
		{
			InfoTitleText->SetText(PendingReward->RewardName);
		}
		
		if (InfoDescriptionText)
		{
			InfoDescriptionText->SetText(PendingReward->Description);
		}
		
		if (InfoStatsText)
		{
			FString StatsString;
			for (const auto& Stat : PendingReward->StatModifiers)
			{
				StatsString += FString::Printf(TEXT("%s: +%.0f%%\n"), 
					*Stat.Key.ToString(), Stat.Value * 100.0f);
			}
			InfoStatsText->SetText(FText::FromString(StatsString));
		}
	}
	else if (SelectedSlotIndex >= 0 && SelectedSlotIndex < SlotWidgets.Num())
	{
		// Show selected slot info
		URewardDataAsset* SelectedReward = SlotManagerComponent ? SlotManagerComponent->GetRewardInSlot(SelectedSlotIndex) : nullptr;
		
		if (SelectedReward)
		{
			InfoPanel->SetVisibility(ESlateVisibility::Visible);
			
			if (InfoTitleText)
			{
				int32 StackLevel = SlotManagerComponent->GetRewardStackLevel(SelectedSlotIndex);
				if (StackLevel > 1)
				{
					InfoTitleText->SetText(FText::Format(NSLOCTEXT("SlotManager", "RewardWithLevel", "{0} (Lv.{1})"), 
						SelectedReward->RewardName, FText::AsNumber(StackLevel)));
				}
				else
				{
					InfoTitleText->SetText(SelectedReward->RewardName);
				}
			}
			
			if (InfoDescriptionText)
			{
				InfoDescriptionText->SetText(SelectedReward->Description);
			}
			
			if (InfoStatsText)
			{
				int32 StackLevel = SlotManagerComponent->GetRewardStackLevel(SelectedSlotIndex);
				float StackMultiplier = (StackLevel > 0 && StackLevel <= SelectedReward->StackMultipliers.Num()) ? 
					SelectedReward->StackMultipliers[StackLevel - 1] : 1.0f;
				
				FString StatsString;
				for (const auto& Stat : SelectedReward->StatModifiers)
				{
					StatsString += FString::Printf(TEXT("%s: +%.0f%%\n"), 
						*Stat.Key.ToString(), Stat.Value * StackMultiplier * 100.0f);
				}
				InfoStatsText->SetText(FText::FromString(StatsString));
			}
		}
		else
		{
			InfoPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		InfoPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USlotManagerWidget::UpdateActionButtons()
{
	bool bHasSelectedSlot = (SelectedSlotIndex >= 0);
	bool bSlotIsOccupied = bHasSelectedSlot && SlotManagerComponent && SlotManagerComponent->GetRewardInSlot(SelectedSlotIndex);
	
	if (EquipButton)
	{
		EquipButton->SetIsEnabled(bIsPlacingReward && bHasSelectedSlot && !bSlotIsOccupied);
	}
	
	if (ReplaceButton)
	{
		ReplaceButton->SetIsEnabled(bIsPlacingReward && bHasSelectedSlot && bSlotIsOccupied);
	}
	
	if (ClearButton)
	{
		ClearButton->SetIsEnabled(!bIsPlacingReward && bHasSelectedSlot && bSlotIsOccupied);
	}
	
	if (CancelButton)
	{
		CancelButton->SetIsEnabled(bIsPlacingReward);
	}
}

void USlotManagerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Bind button events
	if (EquipButton)
	{
		EquipButton->OnClicked.AddDynamic(this, &USlotManagerWidget::OnEquipButtonClicked);
	}
	
	if (ReplaceButton)
	{
		ReplaceButton->OnClicked.AddDynamic(this, &USlotManagerWidget::OnReplaceButtonClicked);
	}
	
	if (ClearButton)
	{
		ClearButton->OnClicked.AddDynamic(this, &USlotManagerWidget::OnClearButtonClicked);
	}
	
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &USlotManagerWidget::OnCancelButtonClicked);
	}
	
	CreateSlots();
}

void USlotManagerWidget::OnSlotClickedHandler(int32 SlotIndex)
{
	SelectSlot(SlotIndex);
	
	if (bIsPlacingReward && PendingReward)
	{
		// Check if we can place the reward here
		if (!SlotManagerComponent->GetRewardInSlot(SlotIndex))
		{
			EquipToSelectedSlot();
		}
		else if (SlotManagerComponent->GetRewardInSlot(SlotIndex)->RewardTag == PendingReward->RewardTag)
		{
			// Enhance existing reward
			if (SlotManagerComponent->EnhanceReward(PendingReward->RewardTag))
			{
				PendingReward = nullptr;
				bIsPlacingReward = false;
				RefreshSlots();
			}
		}
	}
}

void USlotManagerWidget::OnSlotDragStartedHandler(int32 SlotIndex)
{
	// Visual feedback handled in SlotWidget
}

void USlotManagerWidget::OnSlotDropReceivedHandler(int32 SourceSlot, int32 TargetSlot)
{
	SwapSlots(SourceSlot, TargetSlot);
}

void USlotManagerWidget::OnRewardEquippedHandler(int32 SlotIndex, URewardDataAsset* Reward)
{
	RefreshSlots();
}

void USlotManagerWidget::OnRewardRemovedHandler(int32 SlotIndex, URewardDataAsset* Reward)
{
	RefreshSlots();
}

void USlotManagerWidget::OnRewardEnhancedHandler(int32 SlotIndex, URewardDataAsset* Reward, int32 NewStackLevel)
{
	RefreshSlots();
	PlayEnhanceAnimation();
}

void USlotManagerWidget::OnEquipButtonClicked()
{
	EquipToSelectedSlot();
}

void USlotManagerWidget::OnReplaceButtonClicked()
{
	ReplaceSelectedSlot();
}

void USlotManagerWidget::OnClearButtonClicked()
{
	ClearSelectedSlot();
}

void USlotManagerWidget::OnCancelButtonClicked()
{
	CancelOperation();
}