#include "SEnemyHealthWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
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
    
    // Initialize health values if component is valid
    if (UHealthComponent* HealthComp = EnemyHealthComponentRef.Get())
    {
        CurrentEnemyHealth = HealthComp->GetCurrentHealth();
        MaxEnemyHealth = HealthComp->GetMaxHealth();
        CurrentEnemyPoise = HealthComp->GetCurrentPoise();
        MaxEnemyPoise = HealthComp->GetMaxPoise();
        bIsVisible = true;
        
    }
    else
    {
    }
    
    ChildSlot
    [
        SNew(SOverlay)
        
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Top)
        .Padding(0, 100, 0, 0)
        [
            SNew(SBox)
            .WidthOverride(500)
            .HeightOverride(220)
            .Visibility(this, &SEnemyHealthWidget::GetWidgetVisibility)
            [
                SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.95f))  // Dark background
                .Padding(FMargin(20.0f, 15.0f))
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
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SEnemyHealthWidget::UpdateEnemyPoise(float CurrentPoise, float MaxPoise)
{
    CurrentEnemyPoise = CurrentPoise;
    MaxEnemyPoise = MaxPoise;
    Invalidate(EInvalidateWidgetReason::Paint);
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
        
        Invalidate(EInvalidateWidgetReason::Paint);
    }
    else
    {
        bIsVisible = false;
        Invalidate(EInvalidateWidgetReason::Visibility);
    }
}

void SEnemyHealthWidget::ShowWidget()
{
    bIsVisible = true;
    Invalidate(EInvalidateWidgetReason::Visibility);
}

void SEnemyHealthWidget::HideWidget()
{
    bIsVisible = false;
    Invalidate(EInvalidateWidgetReason::Visibility);
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
    // Always visible if we have health data
    if (CurrentEnemyHealth > 0 || MaxEnemyHealth > 0)
    {
        return EVisibility::SelfHitTestInvisible;
    }
    
    EVisibility Vis = bIsVisible ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
    
    return Vis;
}