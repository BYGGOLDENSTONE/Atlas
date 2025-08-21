#include "RewardSelectionComponent.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Characters/GameCharacterBase.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

URewardSelectionComponent::URewardSelectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URewardSelectionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Get player slot manager reference
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AGameCharacterBase* PlayerChar = Cast<AGameCharacterBase>(PC->GetPawn()))
		{
			PlayerSlotManager = PlayerChar->FindComponentByClass<USlotManagerComponent>();
		}
	}
}

void URewardSelectionComponent::PresentRewardChoice(const TArray<URewardDataAsset*>& Rewards)
{
	if (bSelectionActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selection already active"));
		return;
	}
	
	if (Rewards.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No rewards to present"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Presenting %d reward choices"), Rewards.Num());
	
	// Store reward choices
	CurrentRewardChoices = Rewards;
	bSelectionActive = true;
	SelectionStartTime = GetWorld()->GetTimeSeconds();
	
	// Pause game if configured
	if (bPauseDuringSelection)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
	
	// Create and show UI
	CreateSelectionWidget();
	
	// Start timeout timer if configured
	if (SelectionTimeout > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			TimeoutTimerHandle,
			this,
			&URewardSelectionComponent::OnSelectionTimeout,
			SelectionTimeout,
			false
		);
	}
	
	// Broadcast event
	OnSelectionPresented.Broadcast();
}

void URewardSelectionComponent::PresentSingleReward(URewardDataAsset* Reward)
{
	if (!Reward)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid reward"));
		return;
	}
	
	TArray<URewardDataAsset*> SingleChoice;
	SingleChoice.Add(Reward);
	PresentRewardChoice(SingleChoice);
}

void URewardSelectionComponent::CloseSelection()
{
	if (!bSelectionActive)
		return;
		
	UE_LOG(LogTemp, Log, TEXT("Closing reward selection"));
	
	// Clear timer
	GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	
	// Destroy UI
	DestroySelectionWidget();
	
	// Unpause game
	if (bPauseDuringSelection)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
	
	// Clear state
	bSelectionActive = false;
	CurrentRewardChoices.Empty();
	PendingReward = nullptr;
}

void URewardSelectionComponent::ForceTimeout()
{
	if (bSelectionActive)
	{
		OnSelectionTimeout();
	}
}

void URewardSelectionComponent::OnRewardChosen(URewardDataAsset* SelectedReward)
{
	if (!bSelectionActive || !SelectedReward)
		return;
		
	UE_LOG(LogTemp, Log, TEXT("Reward chosen: %s"), *SelectedReward->RewardName.ToString());
	
	// Check if we need slot selection
	if (NeedsSlotSelection(SelectedReward))
	{
		PendingReward = SelectedReward;
		ShowSlotManagement(SelectedReward);
	}
	else
	{
		// Try to auto-equip
		int32 SlotIndex = AutoEquipReward(SelectedReward);
		if (SlotIndex >= 0)
		{
			OnRewardSelected.Broadcast(SelectedReward, SlotIndex);
			FinalizeSelection(ERewardSelectionResult::Selected);
		}
		else
		{
			// No available slots
			UE_LOG(LogTemp, Warning, TEXT("Failed to equip reward - no available slots"));
			// Still show slot management to let player replace
			PendingReward = SelectedReward;
			ShowSlotManagement(SelectedReward);
		}
	}
}

void URewardSelectionComponent::OnSelectionSkipped()
{
	if (!bSelectionActive || !bAllowSkip)
		return;
		
	UE_LOG(LogTemp, Log, TEXT("Reward selection skipped"));
	
	FinalizeSelection(ERewardSelectionResult::Skipped);
}

bool URewardSelectionComponent::ProcessRewardSelection(URewardDataAsset* Reward)
{
	if (!Reward || !PlayerSlotManager)
		return false;
		
	// Check if this would enhance an existing reward
	if (WouldEnhanceExisting(Reward))
	{
		return PlayerSlotManager->EnhanceReward(Reward->RewardTag);
	}
	else
	{
		// Find best slot to equip to
		int32 BestSlot = FindBestSlotForReward(Reward);
		if (BestSlot >= 0)
		{
			return PlayerSlotManager->EquipReward(Reward, BestSlot);
		}
	}
	
	return false;
}

void URewardSelectionComponent::ShowSlotManagement(URewardDataAsset* Reward)
{
	if (!Reward)
		return;
		
	UE_LOG(LogTemp, Log, TEXT("Showing slot management for reward: %s"), *Reward->RewardName.ToString());
	
	// Create slot management widget if we have the class
	if (SlotManagementWidgetClass)
	{
		if (!ActiveSlotWidget)
		{
			ActiveSlotWidget = CreateWidget<UUserWidget>(GetWorld(), SlotManagementWidgetClass);
		}
		
		if (ActiveSlotWidget)
		{
			ActiveSlotWidget->AddToViewport(10);
			
			// TODO: Pass reward data to widget for display
			// This would typically be done through a custom widget interface
		}
	}
	else
	{
		// Fallback to auto-equip if no slot UI
		UE_LOG(LogTemp, Warning, TEXT("No slot management widget class set"));
		int32 SlotIndex = AutoEquipReward(Reward);
		if (SlotIndex >= 0)
		{
			OnRewardSelected.Broadcast(Reward, SlotIndex);
			FinalizeSelection(ERewardSelectionResult::Selected);
		}
	}
}

int32 URewardSelectionComponent::AutoEquipReward(URewardDataAsset* Reward)
{
	if (!Reward || !PlayerSlotManager)
		return -1;
		
	// First check if this would enhance an existing reward
	if (WouldEnhanceExisting(Reward))
	{
		if (PlayerSlotManager->EnhanceReward(Reward->RewardTag))
		{
			UE_LOG(LogTemp, Log, TEXT("Enhanced existing reward: %s"), *Reward->RewardName.ToString());
			return 0; // Return success indicator
		}
	}
	
	// Find best slot for new reward
	int32 BestSlot = FindBestSlotForReward(Reward);
	if (BestSlot >= 0)
	{
		if (PlayerSlotManager->EquipReward(Reward, BestSlot))
		{
			UE_LOG(LogTemp, Log, TEXT("Auto-equipped reward to slot %d"), BestSlot);
			return BestSlot;
		}
	}
	
	// If no empty slots, try to replace a lower priority reward
	for (int32 i = 0; i < 6; i++) // Assuming max 6 slots
	{
		URewardDataAsset* ExistingReward = PlayerSlotManager->GetRewardInSlot(i);
		if (ExistingReward && CanReplaceReward(Reward, ExistingReward))
		{
			if (PlayerSlotManager->ReplaceReward(i, Reward))
			{
				UE_LOG(LogTemp, Log, TEXT("Replaced reward in slot %d"), i);
				return i;
			}
		}
	}
	
	return -1;
}

void URewardSelectionComponent::OnSlotSelected(int32 SlotIndex)
{
	if (!PendingReward || !PlayerSlotManager)
		return;
		
	UE_LOG(LogTemp, Log, TEXT("Slot %d selected for reward: %s"), 
		SlotIndex, *PendingReward->RewardName.ToString());
	
	// Try to equip to selected slot
	bool bSuccess = false;
	
	// Check if slot is empty
	URewardDataAsset* ExistingReward = PlayerSlotManager->GetRewardInSlot(SlotIndex);
	if (!ExistingReward)
	{
		bSuccess = PlayerSlotManager->EquipReward(PendingReward, SlotIndex);
	}
	else
	{
		// Replace existing reward
		bSuccess = PlayerSlotManager->ReplaceReward(SlotIndex, PendingReward);
	}
	
	if (bSuccess)
	{
		OnRewardSelected.Broadcast(PendingReward, SlotIndex);
		FinalizeSelection(ERewardSelectionResult::Selected);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to equip reward to slot %d"), SlotIndex);
	}
	
	// Close slot widget
	if (ActiveSlotWidget)
	{
		ActiveSlotWidget->RemoveFromParent();
	}
	
	PendingReward = nullptr;
}

bool URewardSelectionComponent::NeedsSlotSelection(URewardDataAsset* Reward) const
{
	if (!Reward || !PlayerSlotManager)
		return false;
		
	// Check if this would enhance existing (no slot selection needed)
	if (WouldEnhanceExisting(Reward))
		return false;
		
	// Check if we have enough free slots
	if (PlayerSlotManager->HasSlotsForReward(Reward))
	{
		// We have space, but might want to let player choose where
		// This could be a configuration option
		return false; // Auto-place for now
	}
	
	// Not enough free slots - need player to choose what to replace
	return true;
}

float URewardSelectionComponent::GetTimeRemaining() const
{
	if (!bSelectionActive || SelectionTimeout <= 0.0f)
		return -1.0f;
		
	float ElapsedTime = GetWorld()->GetTimeSeconds() - SelectionStartTime;
	return FMath::Max(0.0f, SelectionTimeout - ElapsedTime);
}

void URewardSelectionComponent::CreateSelectionWidget()
{
	if (!RewardSelectionWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("No reward selection widget class set"));
		return;
	}
	
	if (!ActiveSelectionWidget)
	{
		ActiveSelectionWidget = CreateWidget<UUserWidget>(GetWorld(), RewardSelectionWidgetClass);
	}
	
	if (ActiveSelectionWidget)
	{
		ActiveSelectionWidget->AddToViewport(5);
		
		// TODO: Pass reward choices to widget
		// This would typically be done through a custom widget interface
		UpdateSelectionWidget();
	}
}

void URewardSelectionComponent::DestroySelectionWidget()
{
	if (ActiveSelectionWidget)
	{
		ActiveSelectionWidget->RemoveFromParent();
		ActiveSelectionWidget = nullptr;
	}
	
	if (ActiveSlotWidget)
	{
		ActiveSlotWidget->RemoveFromParent();
		ActiveSlotWidget = nullptr;
	}
}

void URewardSelectionComponent::UpdateSelectionWidget()
{
	// TODO: Update widget with current reward choices
	// This would interface with the actual widget blueprint
	
	if (ActiveSelectionWidget)
	{
		// Example of how this might work with a custom interface:
		// if (IRewardSelectionInterface* Interface = Cast<IRewardSelectionInterface>(ActiveSelectionWidget))
		// {
		//     Interface->SetRewardChoices(CurrentRewardChoices);
		// }
	}
}

FText URewardSelectionComponent::GetRewardComparison(URewardDataAsset* Reward) const
{
	if (!Reward || !PlayerSlotManager)
		return FText::GetEmpty();
		
	FString ComparisonText;
	
	// Check if this would enhance existing
	if (WouldEnhanceExisting(Reward))
	{
		FEquippedReward Existing = PlayerSlotManager->FindRewardByTag(Reward->RewardTag);
		if (Existing.RewardData)
		{
			int32 NewLevel = Existing.StackLevel + 1;
			ComparisonText = FString::Printf(
				TEXT("Enhance: Level %d -> %d\n"),
				Existing.StackLevel,
				NewLevel
			);
			
			// Show stat improvements
			float OldMultiplier = Existing.RewardData->GetStackMultiplier(Existing.StackLevel);
			float NewMultiplier = Existing.RewardData->GetStackMultiplier(NewLevel);
			ComparisonText += FString::Printf(
				TEXT("Effect: x%.1f -> x%.1f"),
				OldMultiplier,
				NewMultiplier
			);
		}
	}
	else
	{
		// New reward
		ComparisonText = TEXT("New Reward\n");
		ComparisonText += FString::Printf(TEXT("Slot Cost: %d\n"), Reward->SlotCost);
		
		// Check available slots
		int32 AvailableSlots = PlayerSlotManager->GetAvailableSlotCount();
		if (AvailableSlots < Reward->SlotCost)
		{
			ComparisonText += FString::Printf(
				TEXT("Warning: Not enough slots (%d/%d)"),
				AvailableSlots,
				Reward->SlotCost
			);
		}
	}
	
	return FText::FromString(ComparisonText);
}

FText URewardSelectionComponent::GetSelectionPreview(URewardDataAsset* Reward) const
{
	if (!Reward)
		return FText::GetEmpty();
		
	FString PreviewText = Reward->Description.ToString();
	
	// Add category info
	PreviewText += TEXT("\n\nCategory: ");
	switch (Reward->Category)
	{
		case ERewardCategory::Defense:
			PreviewText += TEXT("Defense");
			break;
		case ERewardCategory::Offense:
			PreviewText += TEXT("Offense");
			break;
		case ERewardCategory::PassiveStats:
			PreviewText += TEXT("Passive Stats");
			break;
		case ERewardCategory::PassiveAbility:
			PreviewText += TEXT("Passive Ability");
			break;
		case ERewardCategory::Interactable:
			PreviewText += TEXT("Interactable");
			break;
	}
	
	// Add slot cost
	PreviewText += FString::Printf(TEXT("\nSlot Cost: %d"), Reward->SlotCost);
	
	// Add max stack info
	if (Reward->MaxStackLevel > 1)
	{
		PreviewText += FString::Printf(TEXT("\nMax Stack: %d"), Reward->MaxStackLevel);
	}
	
	return FText::FromString(PreviewText);
}

bool URewardSelectionComponent::WouldEnhanceExisting(URewardDataAsset* Reward) const
{
	if (!Reward || !PlayerSlotManager)
		return false;
		
	// Check if already equipped
	FEquippedReward Existing = PlayerSlotManager->FindRewardByTag(Reward->RewardTag);
	if (Existing.RewardData)
	{
		// Check if can stack further
		return Existing.StackLevel < Existing.RewardData->MaxStackLevel;
	}
	
	return false;
}

void URewardSelectionComponent::OnSelectionTimeout()
{
	if (!bSelectionActive)
		return;
		
	UE_LOG(LogTemp, Log, TEXT("Reward selection timed out"));
	
	if (bAutoSelectOnTimeout && CurrentRewardChoices.Num() > 0)
	{
		// Select random reward
		int32 RandomIndex = FMath::RandRange(0, CurrentRewardChoices.Num() - 1);
		URewardDataAsset* RandomReward = CurrentRewardChoices[RandomIndex];
		
		UE_LOG(LogTemp, Log, TEXT("Auto-selecting reward: %s"), *RandomReward->RewardName.ToString());
		OnRewardChosen(RandomReward);
	}
	else
	{
		FinalizeSelection(ERewardSelectionResult::TimedOut);
	}
}

void URewardSelectionComponent::FinalizeSelection(ERewardSelectionResult Result)
{
	UE_LOG(LogTemp, Log, TEXT("Finalizing selection with result: %s"), 
		*UEnum::GetValueAsString(Result));
	
	// Broadcast completion event
	OnSelectionComplete.Broadcast(Result);
	
	// Close selection UI
	CloseSelection();
}

int32 URewardSelectionComponent::FindBestSlotForReward(URewardDataAsset* Reward) const
{
	if (!Reward || !PlayerSlotManager)
		return -1;
		
	// Look for empty slots first
	for (int32 i = 0; i < 6; i++) // Assuming max 6 slots
	{
		if (!PlayerSlotManager->GetRewardInSlot(i))
		{
			// Check if we have enough consecutive empty slots for multi-slot rewards
			if (Reward->SlotCost == 1)
			{
				return i;
			}
			else
			{
				// Check if next slots are also empty
				bool bHasSpace = true;
				for (int32 j = 1; j < Reward->SlotCost && (i + j) < 6; j++)
				{
					if (PlayerSlotManager->GetRewardInSlot(i + j))
					{
						bHasSpace = false;
						break;
					}
				}
				
				if (bHasSpace)
					return i;
			}
		}
	}
	
	return -1;
}

bool URewardSelectionComponent::CanReplaceReward(URewardDataAsset* NewReward, URewardDataAsset* ExistingReward) const
{
	if (!NewReward || !ExistingReward)
		return false;
		
	// Simple priority system - could be expanded
	// For now, only replace if new reward is higher tier (more slot cost)
	return NewReward->SlotCost > ExistingReward->SlotCost;
}