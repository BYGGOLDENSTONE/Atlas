#include "SRunProgressWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Atlas/Components/RunManagerComponent.h"
#include "Atlas/Components/HealthComponent.h"
#include "Atlas/Components/StationIntegrityComponent.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "Styling/SlateColor.h"

void SRunProgressWidget::Construct(const FArguments& InArgs)
{
	RunManagerRef = InArgs._RunManager;
	HealthComponentRef = InArgs._HealthComponent;
	IntegrityComponentRef = InArgs._IntegrityComponent;
	
	CurrentRoomIndex = 0;
	CurrentHealth = 100.0f;
	MaxHealth = 100.0f;
	CurrentPoise = 100.0f;
	MaxPoise = 100.0f;
	CurrentIntegrity = 100.0f;
	MaxIntegrity = 100.0f;
	
	RoomCompletionStatus.SetNum(TotalRooms);
	RoomFailureStatus.SetNum(TotalRooms);
	
	ChildSlot
	[
		SNew(SVerticalBox)
		
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(10.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(FMargin(10.0f))
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			[
				SNew(SBox)
				.WidthOverride(350)
				[
					SNew(SVerticalBox)
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("RUN PROGRESS")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				CreateRoomProgressBar()
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				SNew(STextBlock)
				.Text(this, &SRunProgressWidget::GetCurrentRoomName)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 10, 0, 5)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("HEALTH")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				CreateHealthBar()
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 5, 0, 5)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("POISE")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 5)
			[
				CreatePoiseBar()
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 5, 0, 5)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("STATION INTEGRITY")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateIntegrityBar()
			]
				]
			]
		]
		
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNullWidget::NullWidget
		]
	];
}

TSharedRef<SWidget> SRunProgressWidget::CreateRoomProgressBar()
{
	TSharedRef<SHorizontalBox> ProgressBar = SNew(SHorizontalBox);
	
	for (int32 i = 0; i < TotalRooms; i++)
	{
		ProgressBar->AddSlot()
		.FillWidth(1.0f)
		.Padding(2, 0)
		[
			CreateRoomIcon(i)
		];
		
		if (i < TotalRooms - 1)
		{
			ProgressBar->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(20)
				.HeightOverride(2)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(this, &SRunProgressWidget::GetRoomIconColor, i + 1)
				]
			];
		}
	}
	
	return ProgressBar;
}

TSharedRef<SWidget> SRunProgressWidget::CreateRoomIcon(int32 RoomIndex)
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(this, &SRunProgressWidget::GetRoomIconColor, RoomIndex)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(40)
			.HeightOverride(40)
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(RoomIndex + 1))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
		];
}

TSharedRef<SWidget> SRunProgressWidget::CreateHealthBar()
{
	return SNew(SBox)
		.HeightOverride(20)
		[
			SNew(SProgressBar)
			.Percent(this, &SRunProgressWidget::GetHealthPercent)
			.FillColorAndOpacity(this, &SRunProgressWidget::GetHealthBarColor)
			.BackgroundImage(FAppStyle::GetBrush("ProgressBar.Background"))
		];
}

TSharedRef<SWidget> SRunProgressWidget::CreatePoiseBar()
{
	return SNew(SBox)
		.HeightOverride(20)
		[
			SNew(SProgressBar)
			.Percent(this, &SRunProgressWidget::GetPoisePercent)
			.FillColorAndOpacity(this, &SRunProgressWidget::GetPoiseBarColor)
			.BackgroundImage(FAppStyle::GetBrush("ProgressBar.Background"))
		];
}

TSharedRef<SWidget> SRunProgressWidget::CreateIntegrityBar()
{
	return SNew(SBox)
		.HeightOverride(20)
		[
			SNew(SProgressBar)
			.Percent(this, &SRunProgressWidget::GetIntegrityPercent)
			.FillColorAndOpacity(this, &SRunProgressWidget::GetIntegrityBarColor)
			.BackgroundImage(FAppStyle::GetBrush("ProgressBar.Background"))
		];
}

void SRunProgressWidget::UpdateRoomProgress(int32 InCurrentRoomIndex, const TArray<URoomDataAsset*>& CompletedRooms)
{
	CurrentRoomIndex = FMath::Clamp(InCurrentRoomIndex, 0, TotalRooms - 1);
	
	for (int32 i = 0; i < CurrentRoomIndex && i < TotalRooms; i++)
	{
		RoomCompletionStatus[i] = true;
	}
}

void SRunProgressWidget::UpdateHealth(float InCurrentHealth, float InMaxHealth)
{
	CurrentHealth = InCurrentHealth;
	MaxHealth = InMaxHealth;
}

void SRunProgressWidget::UpdatePoise(float InCurrentPoise, float InMaxPoise)
{
	CurrentPoise = InCurrentPoise;
	MaxPoise = InMaxPoise;
}

void SRunProgressWidget::UpdateIntegrity(float InCurrentIntegrity, float InMaxIntegrity)
{
	CurrentIntegrity = InCurrentIntegrity;
	MaxIntegrity = InMaxIntegrity;
}

void SRunProgressWidget::UpdateCurrentRoomInfo(URoomDataAsset* RoomData)
{
	CurrentRoomData = RoomData;
}

void SRunProgressWidget::SetRoomCompleted(int32 RoomIndex)
{
	if (RoomIndex >= 0 && RoomIndex < TotalRooms)
	{
		RoomCompletionStatus[RoomIndex] = true;
		RoomFailureStatus[RoomIndex] = false;
	}
}

void SRunProgressWidget::SetRoomFailed(int32 RoomIndex)
{
	if (RoomIndex >= 0 && RoomIndex < TotalRooms)
	{
		RoomFailureStatus[RoomIndex] = true;
		RoomCompletionStatus[RoomIndex] = false;
	}
}

FText SRunProgressWidget::GetRoomProgressText() const
{
	return FText::Format(NSLOCTEXT("RunProgress", "RoomProgress", "Room {0} of {1}"), 
		FText::AsNumber(CurrentRoomIndex + 1), FText::AsNumber(TotalRooms));
}

FText SRunProgressWidget::GetHealthText() const
{
	return FText::Format(NSLOCTEXT("RunProgress", "Health", "{0}/{1}"), 
		FText::AsNumber(FMath::RoundToInt(CurrentHealth)), 
		FText::AsNumber(FMath::RoundToInt(MaxHealth)));
}

FText SRunProgressWidget::GetPoiseText() const
{
	return FText::Format(NSLOCTEXT("RunProgress", "Poise", "{0}/{1}"), 
		FText::AsNumber(FMath::RoundToInt(CurrentPoise)), 
		FText::AsNumber(FMath::RoundToInt(MaxPoise)));
}

FText SRunProgressWidget::GetIntegrityText() const
{
	return FText::Format(NSLOCTEXT("RunProgress", "Integrity", "{0}/{1}"), 
		FText::AsNumber(FMath::RoundToInt(CurrentIntegrity)), 
		FText::AsNumber(FMath::RoundToInt(MaxIntegrity)));
}

FText SRunProgressWidget::GetCurrentRoomName() const
{
	if (CurrentRoomData)
	{
		return CurrentRoomData->RoomName;
	}
	return FText::FromString(TEXT("Unknown Room"));
}

FText SRunProgressWidget::GetHazardText(ERoomHazard Hazard) const
{
	switch (Hazard)
	{
		case ERoomHazard::LowGravity: return FText::FromString(TEXT("Low Gravity"));
		case ERoomHazard::ElectricalSurges: return FText::FromString(TEXT("Electrical Surges"));
		case ERoomHazard::HullBreach: return FText::FromString(TEXT("Hull Breach"));
		case ERoomHazard::ToxicLeak: return FText::FromString(TEXT("Toxic Leak"));
		case ERoomHazard::SystemMalfunction: return FText::FromString(TEXT("System Malfunction"));
		case ERoomHazard::None:
		default: 
			return FText::FromString(TEXT("No Hazard"));
	}
}

FSlateColor SRunProgressWidget::GetHealthBarColor() const
{
	float HealthPercent = MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.0f;
	if (HealthPercent <= LowHealthThreshold)
	{
		return FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f));
	}
	else if (HealthPercent <= 0.5f)
	{
		return FSlateColor(FLinearColor(1.0f, 0.8f, 0.2f));
	}
	return FSlateColor(FLinearColor(0.2f, 1.0f, 0.2f));
}

FSlateColor SRunProgressWidget::GetPoiseBarColor() const
{
	float PoisePercent = MaxPoise > 0 ? CurrentPoise / MaxPoise : 0.0f;
	if (CurrentPoise <= 0.0f)
	{
		return FSlateColor(FLinearColor(0.5f, 0.0f, 0.5f));
	}
	else if (PoisePercent <= 0.25f)
	{
		return FSlateColor(FLinearColor(0.8f, 0.4f, 0.8f));
	}
	else if (PoisePercent <= 0.5f)
	{
		return FSlateColor(FLinearColor(0.6f, 0.2f, 0.8f));
	}
	return FSlateColor(FLinearColor(0.8f, 0.6f, 1.0f));
}

FSlateColor SRunProgressWidget::GetIntegrityBarColor() const
{
	float IntegrityPercent = MaxIntegrity > 0 ? CurrentIntegrity / MaxIntegrity : 0.0f;
	if (IntegrityPercent <= LowIntegrityThreshold)
	{
		return FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f));
	}
	else if (IntegrityPercent <= 0.5f)
	{
		return FSlateColor(FLinearColor(1.0f, 0.6f, 0.2f));
	}
	return FSlateColor(FLinearColor(0.2f, 0.6f, 1.0f));
}

FSlateColor SRunProgressWidget::GetRoomIconColor(int32 RoomIndex) const
{
	if (RoomIndex < 0 || RoomIndex >= TotalRooms)
	{
		return FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f));
	}
	
	if (RoomFailureStatus[RoomIndex])
	{
		return FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f));
	}
	else if (RoomCompletionStatus[RoomIndex])
	{
		return FSlateColor(FLinearColor(0.2f, 1.0f, 0.2f));
	}
	else if (RoomIndex == CurrentRoomIndex)
	{
		return FSlateColor(FLinearColor(1.0f, 1.0f, 0.2f));
	}
	else
	{
		return FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f));
	}
}

TOptional<float> SRunProgressWidget::GetHealthPercent() const
{
	return MaxHealth > 0 ? CurrentHealth / MaxHealth : 0.0f;
}

TOptional<float> SRunProgressWidget::GetPoisePercent() const
{
	return MaxPoise > 0 ? CurrentPoise / MaxPoise : 0.0f;
}

TOptional<float> SRunProgressWidget::GetIntegrityPercent() const
{
	return MaxIntegrity > 0 ? CurrentIntegrity / MaxIntegrity : 0.0f;
}