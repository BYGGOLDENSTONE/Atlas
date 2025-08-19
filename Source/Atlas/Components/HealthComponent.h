#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth, float, HealthDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, DamageAmount, AActor*, DamageInstigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealed, float, HealAmount, AActor*, HealInstigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, KilledBy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRevived);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPoiseChanged, float, CurrentPoise, float, MaxPoise, float, PoiseDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaggered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaggerRecovered);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bIsDead = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bIsInvincible = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise", meta = (ClampMin = "1.0"))
    float MaxPoise = 100.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Poise")
    float CurrentPoise;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise")
    float PoiseRegenRate = 15.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise")
    float PoiseRegenDelay = 1.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise")
    float StaggerDuration = 2.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Poise")
    bool bIsStaggered = false;

    UFUNCTION(BlueprintCallable, Category = "Health")
    void TakeDamage(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount, AActor* HealInstigator);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetMaxHealth(float NewMaxHealth, bool bScaleCurrentHealth = true);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void ReviveWithHealth(float ReviveHealth);

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsAlive() const { return !bIsDead; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetMaxHealth() const { return MaxHealth; }
    
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetInvincible(bool bInvincible) { bIsInvincible = bInvincible; }
    
    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsInvincible() const { return bIsInvincible; }
    
    UFUNCTION(BlueprintCallable, Category = "Poise")
    void TakePoiseDamage(float PoiseDamage, AActor* DamageInstigator = nullptr);
    
    UFUNCTION(BlueprintCallable, Category = "Poise")
    void ResetPoise();
    
    UFUNCTION(BlueprintCallable, Category = "Poise")
    float GetPoisePercent() const;
    
    UFUNCTION(BlueprintCallable, Category = "Poise")
    bool IsStaggered() const { return bIsStaggered; }
    
    UFUNCTION(BlueprintCallable, Category = "Poise")
    float GetCurrentPoise() const { return CurrentPoise; }
    
    UFUNCTION(BlueprintCallable, Category = "Poise")
    float GetMaxPoise() const { return MaxPoise; }
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayHitReaction();

    UPROPERTY(BlueprintAssignable, Category = "Health Events")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health Events")
    FOnDamageTaken OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category = "Health Events")
    FOnHealed OnHealed;

    UPROPERTY(BlueprintAssignable, Category = "Health Events")
    FOnDeath OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "Health Events")
    FOnRevived OnRevived;
    
    UPROPERTY(BlueprintAssignable, Category = "Poise Events")
    FOnPoiseChanged OnPoiseChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Poise Events")
    FOnStaggered OnStaggered;
    
    UPROPERTY(BlueprintAssignable, Category = "Poise Events")
    FOnStaggerRecovered OnStaggerRecovered;

private:
    AActor* LastDamageInstigator;
    
    FTimerHandle PoiseRegenTimerHandle;
    FTimerHandle StaggerRecoveryTimerHandle;
    float PoiseRegenDelayTime = 0.0f;
    bool bPoiseRegenActive = false;

    void HandleDeath(AActor* KilledBy);
    void BroadcastHealthChange(float HealthDelta);
    void BroadcastPoiseChange(float PoiseDelta);
    void StartPoiseRegen();
    void RegenPoise();
    void RecoverFromStagger();
};