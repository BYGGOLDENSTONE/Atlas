#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth, float, HealthDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTaken, float, DamageAmount, AActor*, DamageInstigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealed, float, HealAmount, AActor*, HealInstigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, KilledBy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRevived);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bIsDead = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bIsInvincible = false;

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

private:
    AActor* LastDamageInstigator;

    void HandleDeath(AActor* KilledBy);
    void BroadcastHealthChange(float HealthDelta);
};