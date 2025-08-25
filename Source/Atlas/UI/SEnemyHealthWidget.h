#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class UHealthComponent;
class SProgressBar;
class STextBlock;

class ATLAS_API SEnemyHealthWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SEnemyHealthWidget) 
        : _EnemyName(FText::FromString("Enemy"))
    {}
        SLATE_ARGUMENT(UHealthComponent*, EnemyHealthComponent)
        SLATE_ARGUMENT(FText, EnemyName)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    
    void UpdateEnemyHealth(float CurrentHealth, float MaxHealth);
    void UpdateEnemyPoise(float CurrentPoise, float MaxPoise);
    void SetEnemyName(const FText& Name);
    void SetEnemyHealthComponent(UHealthComponent* HealthComp);
    void ShowWidget();
    void HideWidget();
    
private:
    TWeakObjectPtr<UHealthComponent> EnemyHealthComponentRef;
    
    float CurrentEnemyHealth = 100.0f;
    float MaxEnemyHealth = 100.0f;
    float CurrentEnemyPoise = 100.0f;
    float MaxEnemyPoise = 100.0f;
    FText EnemyName;
    
    TSharedPtr<STextBlock> EnemyNameText;
    TSharedPtr<SProgressBar> HealthBar;
    TSharedPtr<SProgressBar> PoiseBar;
    TSharedPtr<STextBlock> HealthText;
    TSharedPtr<STextBlock> PoiseText;
    
    TSharedRef<SWidget> CreateEnemyHealthBar();
    TSharedRef<SWidget> CreateEnemyPoiseBar();
    
    FText GetEnemyNameText() const;
    FText GetHealthText() const;
    FText GetPoiseText() const;
    TOptional<float> GetHealthPercent() const;
    TOptional<float> GetPoisePercent() const;
    FSlateColor GetHealthBarColor() const;
    FSlateColor GetPoiseBarColor() const;
    EVisibility GetWidgetVisibility() const;
    
    bool bIsVisible = false;
};