#include "SEnemyHealthWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Atlas/Components/HealthComponent.h"
#include "Styling/SlateColor.h"
#include "Engine/Engine.h"

void SEnemyHealthWidget::Construct(const FArguments& InArgs)
{
    EnemyHealthComponentRef = InArgs._EnemyHealthComponent;
    EnemyName = InArgs._EnemyName;
    
    ChildSlot
    [
        SNew(SBox)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Top)
        .Padding(FMargin(0, 20, 0, 0))
        .Visibility(this, &SEnemyHealthWidget::GetWidgetVisibility)
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
            .BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f))
            .Padding(FMargin(15.0f, 10.0f))
            [
                SNew(SBox)
                .WidthOverride(400)
                [
                    SNew(SVerticalBox)
                    
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Center)
                    .Padding(0, 0, 0, 8)
                    [
                        SAssignNew(EnemyNameText, STextBlock)
                        .Text(this, &SEnemyHealthWidget::GetEnemyNameText)
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
                        .ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f)))
                        .Justification(ETextJustify::Center)
                    ]
                    
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0, 0, 0, 3)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("HEALTH")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
                    ]
                    
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0, 0, 0, 3)
                    [
                        CreateEnemyHealthBar()
                    ]
                    
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Center)
                    .Padding(0, 0, 0, 8)
                    [
                        SAssignNew(HealthText, STextBlock)
                        .Text(this, &SEnemyHealthWidget::GetHealthText)
                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)))
                    ]
                    
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0, 0, 0, 3)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString(TEXT("POISE")))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
                    ]
                    
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0, 0, 0, 3)
                    [
                        CreateEnemyPoiseBar()
                    ]
                    
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Center)
                    [
                        SAssignNew(PoiseText, STextBlock)
                        .Text(this, &SEnemyHealthWidget::GetPoiseText)
                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
                        .ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)))
                    ]
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SEnemyHealthWidget::CreateEnemyHealthBar()
{
    return SNew(SBox)
        .HeightOverride(24)
        [
            SNew(SOverlay)
            
            + SOverlay::Slot()
            [
                SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush("ProgressBar.Background"))
                .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.9f))
            ]
            
            + SOverlay::Slot()
            [
                SAssignNew(HealthBar, SProgressBar)
                .Percent(this, &SEnemyHealthWidget::GetHealthPercent)
                .FillColorAndOpacity(this, &SEnemyHealthWidget::GetHealthBarColor)
                .BackgroundImage(FAppStyle::GetBrush("NoBrush"))
            ]
        ];
}

TSharedRef<SWidget> SEnemyHealthWidget::CreateEnemyPoiseBar()
{
    return SNew(SBox)
        .HeightOverride(16)
        [
            SNew(SOverlay)
            
            + SOverlay::Slot()
            [
                SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush("ProgressBar.Background"))
                .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.9f))
            ]
            
            + SOverlay::Slot()
            [
                SAssignNew(PoiseBar, SProgressBar)
                .Percent(this, &SEnemyHealthWidget::GetPoisePercent)
                .FillColorAndOpacity(this, &SEnemyHealthWidget::GetPoiseBarColor)
                .BackgroundImage(FAppStyle::GetBrush("NoBrush"))
            ]
        ];
}

void SEnemyHealthWidget::UpdateEnemyHealth(float CurrentHealth, float MaxHealth)
{
    CurrentEnemyHealth = CurrentHealth;
    MaxEnemyHealth = MaxHealth;
}

void SEnemyHealthWidget::UpdateEnemyPoise(float CurrentPoise, float MaxPoise)
{
    CurrentEnemyPoise = CurrentPoise;
    MaxEnemyPoise = MaxPoise;
}

void SEnemyHealthWidget::SetEnemyName(const FText& Name)
{
    EnemyName = Name;
}

void SEnemyHealthWidget::SetEnemyHealthComponent(UHealthComponent* HealthComp)
{
    EnemyHealthComponentRef = HealthComp;
    
    if (HealthComp)
    {
        CurrentEnemyHealth = HealthComp->GetCurrentHealth();
        MaxEnemyHealth = HealthComp->GetMaxHealth();
        CurrentEnemyPoise = HealthComp->GetCurrentPoise();
        MaxEnemyPoise = HealthComp->GetMaxPoise();
        bIsVisible = true;
        
        UE_LOG(LogTemp, Warning, TEXT("SEnemyHealthWidget: Set health component - Health: %.0f/%.0f, Poise: %.0f/%.0f"),
            CurrentEnemyHealth, MaxEnemyHealth, CurrentEnemyPoise, MaxEnemyPoise);
    }
    else
    {
        bIsVisible = false;
        UE_LOG(LogTemp, Warning, TEXT("SEnemyHealthWidget: Health component is null"));
    }
}

void SEnemyHealthWidget::ShowWidget()
{
    bIsVisible = true;
    UE_LOG(LogTemp, Warning, TEXT("SEnemyHealthWidget: ShowWidget called - now visible"));
}

void SEnemyHealthWidget::HideWidget()
{
    bIsVisible = false;
    UE_LOG(LogTemp, Warning, TEXT("SEnemyHealthWidget: HideWidget called - now hidden"));
}

FText SEnemyHealthWidget::GetEnemyNameText() const
{
    return EnemyName;
}

FText SEnemyHealthWidget::GetHealthText() const
{
    return FText::Format(NSLOCTEXT("EnemyHealth", "Health", "{0}/{1}"), 
        FText::AsNumber(FMath::RoundToInt(CurrentEnemyHealth)), 
        FText::AsNumber(FMath::RoundToInt(MaxEnemyHealth)));
}

FText SEnemyHealthWidget::GetPoiseText() const
{
    if (CurrentEnemyPoise <= 0.0f)
    {
        return FText::FromString(TEXT("STAGGERED"));
    }
    
    return FText::Format(NSLOCTEXT("EnemyHealth", "Poise", "{0}/{1}"), 
        FText::AsNumber(FMath::RoundToInt(CurrentEnemyPoise)), 
        FText::AsNumber(FMath::RoundToInt(MaxEnemyPoise)));
}

TOptional<float> SEnemyHealthWidget::GetHealthPercent() const
{
    return MaxEnemyHealth > 0 ? CurrentEnemyHealth / MaxEnemyHealth : 0.0f;
}

TOptional<float> SEnemyHealthWidget::GetPoisePercent() const
{
    return MaxEnemyPoise > 0 ? CurrentEnemyPoise / MaxEnemyPoise : 0.0f;
}

FSlateColor SEnemyHealthWidget::GetHealthBarColor() const
{
    float HealthPercent = MaxEnemyHealth > 0 ? CurrentEnemyHealth / MaxEnemyHealth : 0.0f;
    
    if (HealthPercent <= 0.25f)
    {
        return FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f));
    }
    else if (HealthPercent <= 0.5f)
    {
        return FSlateColor(FLinearColor(1.0f, 0.6f, 0.2f));
    }
    else if (HealthPercent <= 0.75f)
    {
        return FSlateColor(FLinearColor(0.8f, 0.8f, 0.2f));
    }
    
    return FSlateColor(FLinearColor(0.8f, 0.2f, 0.2f));
}

FSlateColor SEnemyHealthWidget::GetPoiseBarColor() const
{
    if (CurrentEnemyPoise <= 0.0f)
    {
        return FSlateColor(FLinearColor(0.5f, 0.0f, 0.5f));
    }
    
    float PoisePercent = MaxEnemyPoise > 0 ? CurrentEnemyPoise / MaxEnemyPoise : 0.0f;
    
    if (PoisePercent <= 0.25f)
    {
        return FSlateColor(FLinearColor(0.8f, 0.4f, 0.8f));
    }
    else if (PoisePercent <= 0.5f)
    {
        return FSlateColor(FLinearColor(0.6f, 0.2f, 0.8f));
    }
    
    return FSlateColor(FLinearColor(0.8f, 0.6f, 1.0f));
}

EVisibility SEnemyHealthWidget::GetWidgetVisibility() const
{
    EVisibility Vis = bIsVisible ? EVisibility::Visible : EVisibility::Collapsed;
    // Debug log every few frames to avoid spam
    static int32 FrameCounter = 0;
    if (++FrameCounter % 60 == 0)  // Log once per second at 60 FPS
    {
        UE_LOG(LogTemp, Warning, TEXT("SEnemyHealthWidget: Visibility = %s, Health: %.0f/%.0f"),
            bIsVisible ? TEXT("Visible") : TEXT("Collapsed"),
            CurrentEnemyHealth, MaxEnemyHealth);
    }
    return Vis;
}