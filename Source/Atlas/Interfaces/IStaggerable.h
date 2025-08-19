#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IStaggerable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UStaggerable : public UInterface
{
    GENERATED_BODY()
};

class ATLAS_API IStaggerable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stagger")
    void ApplyStagger(float Duration, AActor* Instigator);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stagger")
    bool IsStaggered() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stagger")
    void RecoverFromStagger();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stagger")
    float GetStaggerTimeRemaining() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poise")
    void TakePoiseDamage(float PoiseDamage, AActor* DamageInstigator);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poise")
    float GetCurrentPoise() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poise")
    float GetMaxPoise() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Poise")
    void ResetPoise();
};