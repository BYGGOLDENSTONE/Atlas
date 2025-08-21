#include "RunProgressWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Atlas/Components/RunManagerComponent.h"
#include "Atlas/Components/HealthComponent.h"
#include "Atlas/Components/StationIntegrityComponent.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"

// ========================================
// URoomIconWidget Implementation
// ========================================

void URoomIconWidget::InitializeRoomIcon(int32 Number, URoomDataAsset* Data)
{
	RoomNumber = Number;
	RoomData = Data;
	
	if (RoomNumberText)
	{
		RoomNumberText->SetText(FText::AsNumber(RoomNumber));
	}
	
	if (RoomData && RoomIcon)
	{
		RoomIcon->SetBrushFromTexture(RoomData->RoomIcon);
	}
	
	UpdateVisuals();
}

void URoomIconWidget::SetRoomState(ERoomIconState NewState)
{
	ERoomIconState OldState = RoomState;
	RoomState = NewState;
	
	UpdateVisuals();
	
	if (OldState != NewState)
	{
		PlayStateChangeAnimation(NewState);
	}
}

void URoomIconWidget::UpdateVisuals()
{
	// Hide all state indicators
	if (CompletedCheckmark) CompletedCheckmark->SetVisibility(ESlateVisibility::Collapsed);
	if (CurrentIndicator) CurrentIndicator->SetVisibility(ESlateVisibility::Collapsed);
	if (LockedIcon) LockedIcon->SetVisibility(ESlateVisibility::Collapsed);
	if (FailedMark) FailedMark->SetVisibility(ESlateVisibility::Collapsed);
	
	// Show appropriate indicator
	switch (RoomState)
	{
		case ERoomIconState::Locked:
			if (LockedIcon) LockedIcon->SetVisibility(ESlateVisibility::Visible);
			break;
			
		case ERoomIconState::Current:
			if (CurrentIndicator) CurrentIndicator->SetVisibility(ESlateVisibility::Visible);
			break;
			
		case ERoomIconState::Completed:
			if (CompletedCheckmark) CompletedCheckmark->SetVisibility(ESlateVisibility::Visible);
			break;
			
		case ERoomIconState::Failed:
			if (FailedMark) FailedMark->SetVisibility(ESlateVisibility::Visible);
			break;
	}
}

void URoomIconWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateVisuals();
}

// ========================================
// URoomConnectorWidget Implementation
// ========================================

void URoomConnectorWidget::SetConnectionState(bool bCompleted)
{
	if (ProgressFill)
	{
		ProgressFill->SetVisibility(bCompleted ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		
		if (bCompleted)
		{
			PlayCompletionAnimation();
		}
	}
}

// ========================================
// URunProgressWidget Implementation
// ========================================

void URunProgressWidget::InitializeWithComponents(URunManagerComponent* InRunManager, 
	UHealthComponent* InHealth, UStationIntegrityComponent* InIntegrity)
{
	RunManager = InRunManager;
	HealthComponent = InHealth;
	IntegrityComponent = InIntegrity;
	
	CreateRoomProgress();
	
	// Initial updates
	if (RunManager)
	{
		UpdateRoomProgress(RunManager->GetCurrentLevel() - 1);
		UpdateRoomInfo(RunManager->GetCurrentRoom());
	}
	
	if (HealthComponent)
	{
		MaxHealth = HealthComponent->GetMaxHealth();
		CurrentHealth = HealthComponent->GetCurrentHealth();
		UpdateHealthDisplay(CurrentHealth, MaxHealth);
	}
	
	if (IntegrityComponent)
	{
		MaxIntegrity = IntegrityComponent->GetMaxIntegrity();
		CurrentIntegrity = IntegrityComponent->GetCurrentIntegrity();
		UpdateIntegrityDisplay(CurrentIntegrity, MaxIntegrity);
	}
}

void URunProgressWidget::CreateRoomProgress()
{
	if (!RoomProgressContainer || !RoomIconClass || !ConnectorClass)
		return;
		
	RoomProgressContainer->ClearChildren();
	RoomIcons.Empty();
	Connectors.Empty();
	
	// Create 5 room icons with connectors
	for (int32 i = 0; i < 5; ++i)
	{
		// Create room icon
		URoomIconWidget* RoomIcon = CreateWidget<URoomIconWidget>(this, RoomIconClass);
		if (RoomIcon)
		{
			RoomIcon->InitializeRoomIcon(i + 1);
			RoomProgressContainer->AddChild(RoomIcon);
			RoomIcons.Add(RoomIcon);
		}
		
		// Create connector (except after last room)
		if (i < 4)
		{
			URoomConnectorWidget* Connector = CreateWidget<URoomConnectorWidget>(this, ConnectorClass);
			if (Connector)
			{
				RoomProgressContainer->AddChild(Connector);
				Connectors.Add(Connector);
			}
		}
	}
}

void URunProgressWidget::UpdateRoomProgress(int32 NewRoomIndex)
{
	CurrentRoomIndex = FMath::Clamp(NewRoomIndex, 0, 4);
	
	// Update room states
	for (int32 i = 0; i < RoomIcons.Num(); ++i)
	{
		if (RoomIcons[i])
		{
			if (i < CurrentRoomIndex)
			{
				RoomIcons[i]->SetRoomState(ERoomIconState::Completed);
			}
			else if (i == CurrentRoomIndex)
			{
				RoomIcons[i]->SetRoomState(ERoomIconState::Current);
			}
			else
			{
				RoomIcons[i]->SetRoomState(ERoomIconState::Locked);
			}
		}
	}
	
	// Update connectors
	for (int32 i = 0; i < Connectors.Num(); ++i)
	{
		if (Connectors[i])
		{
			Connectors[i]->SetConnectionState(i < CurrentRoomIndex);
		}
	}
	
	// Update level text
	if (CurrentLevelText)
	{
		CurrentLevelText->SetText(FText::Format(NSLOCTEXT("RunProgress", "LevelFormat", "Level {0}/5"), 
			FText::AsNumber(CurrentRoomIndex + 1)));
	}
	
	OnRoomChanged.Broadcast(CurrentRoomIndex);
	PlayRoomTransitionAnimation();
	
	// Start pulsing current room
	PulseCurrentRoom();
}

void URunProgressWidget::UpdateHealthDisplay(float NewHealth, float NewMax)
{
	CurrentHealth = NewHealth;
	MaxHealth = NewMax;
	
	if (HealthBar)
	{
		float Percent = MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.0f;
		HealthBar->SetPercent(Percent);
		
		// Update color based on health percentage
		FLinearColor HealthColor = FLinearColor::Green;
		if (Percent < LowHealthThreshold)
		{
			HealthColor = FLinearColor::Red;
			FlashHealthWarning();
			OnHealthCritical.Broadcast(Percent);
		}
		else if (Percent < 0.5f)
		{
			HealthColor = FLinearColor::Yellow;
		}
		HealthBar->SetFillColorAndOpacity(HealthColor);
	}
	
	if (HealthText)
	{
		HealthText->SetText(FText::Format(NSLOCTEXT("RunProgress", "HealthFormat", "{0}/{1}"), 
			FText::AsNumber(FMath::RoundToInt(CurrentHealth)), 
			FText::AsNumber(FMath::RoundToInt(MaxHealth))));
	}
	
	CheckCriticalStates();
}

void URunProgressWidget::UpdateIntegrityDisplay(float NewIntegrity, float NewMax)
{
	CurrentIntegrity = NewIntegrity;
	MaxIntegrity = NewMax;
	
	if (IntegrityBar)
	{
		float Percent = MaxIntegrity > 0 ? CurrentIntegrity / MaxIntegrity : 0.0f;
		IntegrityBar->SetPercent(Percent);
		
		// Update color based on integrity percentage
		FLinearColor IntegrityColor = FLinearColor(0.0f, 0.8f, 1.0f); // Cyan
		if (Percent < LowIntegrityThreshold)
		{
			IntegrityColor = FLinearColor::Red;
			FlashIntegrityWarning();
			OnIntegrityCritical.Broadcast(Percent);
		}
		else if (Percent < 0.5f)
		{
			IntegrityColor = FLinearColor::Yellow;
		}
		IntegrityBar->SetFillColorAndOpacity(IntegrityColor);
	}
	
	if (IntegrityText)
	{
		IntegrityText->SetText(FText::Format(NSLOCTEXT("RunProgress", "IntegrityFormat", "{0}/{1}"), 
			FText::AsNumber(FMath::RoundToInt(CurrentIntegrity)), 
			FText::AsNumber(FMath::RoundToInt(MaxIntegrity))));
	}
	
	// Show warning if critical
	if (IntegrityWarning)
	{
		float Percent = MaxIntegrity > 0 ? CurrentIntegrity / MaxIntegrity : 0.0f;
		IntegrityWarning->SetVisibility(Percent < LowIntegrityThreshold ? 
			ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	
	CheckCriticalStates();
}

void URunProgressWidget::UpdateRoomInfo(URoomDataAsset* RoomData)
{
	if (!RoomData)
		return;
		
	if (CurrentRoomName)
	{
		CurrentRoomName->SetText(RoomData->RoomName);
	}
	
	if (CurrentEnemyName)
	{
		CurrentEnemyName->SetText(RoomData->EnemyName);
	}
	
	UpdateHazardDisplay(RoomData->EnvironmentalHazard);
	
	// Update current room icon with room data
	if (CurrentRoomIndex >= 0 && CurrentRoomIndex < RoomIcons.Num())
	{
		if (RoomIcons[CurrentRoomIndex])
		{
			RoomIcons[CurrentRoomIndex]->InitializeRoomIcon(CurrentRoomIndex + 1, RoomData);
		}
	}
}

void URunProgressWidget::SetRoomCompleted(int32 RoomIndex)
{
	if (RoomIndex >= 0 && RoomIndex < RoomIcons.Num())
	{
		if (RoomIcons[RoomIndex])
		{
			RoomIcons[RoomIndex]->SetRoomState(ERoomIconState::Completed);
		}
		
		if (RoomIndex < Connectors.Num() && Connectors[RoomIndex])
		{
			Connectors[RoomIndex]->SetConnectionState(true);
		}
	}
}

void URunProgressWidget::SetRoomFailed(int32 RoomIndex)
{
	if (RoomIndex >= 0 && RoomIndex < RoomIcons.Num())
	{
		if (RoomIcons[RoomIndex])
		{
			RoomIcons[RoomIndex]->SetRoomState(ERoomIconState::Failed);
		}
	}
	
	PlayRunFailedAnimation();
}

void URunProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Start pulse timer for current room
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(PulseTimerHandle, 
			this, &URunProgressWidget::PulseCurrentRoom, 2.0f, true);
	}
}

void URunProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Update health and integrity from components
	if (HealthComponent)
	{
		float NewHealth = HealthComponent->GetCurrentHealth();
		float NewMax = HealthComponent->GetMaxHealth();
		if (FMath::Abs(NewHealth - CurrentHealth) > 0.01f || FMath::Abs(NewMax - MaxHealth) > 0.01f)
		{
			UpdateHealthDisplay(NewHealth, NewMax);
		}
	}
	
	if (IntegrityComponent)
	{
		float NewIntegrity = IntegrityComponent->GetCurrentIntegrity();
		float NewMax = IntegrityComponent->GetMaxIntegrity();
		if (FMath::Abs(NewIntegrity - CurrentIntegrity) > 0.01f || FMath::Abs(NewMax - MaxIntegrity) > 0.01f)
		{
			UpdateIntegrityDisplay(NewIntegrity, NewMax);
		}
	}
}

void URunProgressWidget::UpdateHazardDisplay(ERoomHazard Hazard)
{
	if (!HazardWarningContainer)
		return;
		
	if (Hazard == ERoomHazard::None)
	{
		HazardWarningContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		HazardWarningContainer->SetVisibility(ESlateVisibility::Visible);
		
		if (HazardIcon)
		{
			UTexture2D* IconTexture = GetHazardIcon(Hazard);
			if (IconTexture)
			{
				HazardIcon->SetBrushFromTexture(IconTexture);
			}
		}
		
		if (HazardText)
		{
			HazardText->SetText(GetHazardText(Hazard));
		}
	}
}

UTexture2D* URunProgressWidget::GetHazardIcon(ERoomHazard Hazard) const
{
	// These would be loaded from assets in a real implementation
	// For now, return nullptr and let Blueprint handle it
	return nullptr;
}

FText URunProgressWidget::GetHazardText(ERoomHazard Hazard) const
{
	switch (Hazard)
	{
		case ERoomHazard::ElectricalSurges:
			return NSLOCTEXT("RunProgress", "ElectricalHazard", "Electrical Surges");
		case ERoomHazard::ToxicLeak:
			return NSLOCTEXT("RunProgress", "ToxicHazard", "Toxic Leak");
		case ERoomHazard::LowGravity:
			return NSLOCTEXT("RunProgress", "GravityHazard", "Low Gravity");
		case ERoomHazard::SystemMalfunction:
			return NSLOCTEXT("RunProgress", "SystemHazard", "System Malfunction");
		case ERoomHazard::HullBreach:
			return NSLOCTEXT("RunProgress", "HullBreach", "Hull Breach");
		default:
			return FText::GetEmpty();
	}
}

void URunProgressWidget::CheckCriticalStates()
{
	// Check for critical health/integrity
	float HealthPercent = MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.0f;
	float IntegrityPercent = MaxIntegrity > 0 ? CurrentIntegrity / MaxIntegrity : 0.0f;
	
	if (HealthPercent <= 0.0f || IntegrityPercent <= 0.0f)
	{
		// Run failed
		if (CurrentRoomIndex >= 0 && CurrentRoomIndex < RoomIcons.Num())
		{
			SetRoomFailed(CurrentRoomIndex);
		}
		PlayRunFailedAnimation();
	}
}

void URunProgressWidget::PulseCurrentRoom()
{
	if (CurrentRoomIndex >= 0 && CurrentRoomIndex < RoomIcons.Num())
	{
		if (RoomIcons[CurrentRoomIndex] && RoomIcons[CurrentRoomIndex]->RoomState == ERoomIconState::Current)
		{
			RoomIcons[CurrentRoomIndex]->PlayPulseAnimation();
		}
	}
}