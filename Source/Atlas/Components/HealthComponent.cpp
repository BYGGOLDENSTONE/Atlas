#include "HealthComponent.h"
#include "CombatComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    bIsDead = false;
}

void UHealthComponent::TakeDamage(float DamageAmount, AActor* DamageInstigator)
{
    if (bIsDead || DamageAmount <= 0.0f || bIsInvincible)
    {
        return;
    }

    float ActualDamage = FMath::Min(DamageAmount, CurrentHealth);
    CurrentHealth -= ActualDamage;
    LastDamageInstigator = DamageInstigator;

    OnDamageTaken.Broadcast(ActualDamage, DamageInstigator);
    BroadcastHealthChange(-ActualDamage);

    UE_LOG(LogTemp, Log, TEXT("%s took %.1f damage. Health: %.1f/%.1f"), 
        *GetOwner()->GetName(), ActualDamage, CurrentHealth, MaxHealth);

    if (GEngine)
    {
        FString DebugMessage = FString::Printf(TEXT("%s Health: %.0f/%.0f (-%0.f)"),
            *GetOwner()->GetName(), CurrentHealth, MaxHealth, ActualDamage);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMessage);
    }

    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        HandleDeath(DamageInstigator);
    }
}

void UHealthComponent::Heal(float HealAmount, AActor* HealInstigator)
{
    if (bIsDead || HealAmount <= 0.0f || CurrentHealth >= MaxHealth)
    {
        return;
    }

    float ActualHeal = FMath::Min(HealAmount, MaxHealth - CurrentHealth);
    CurrentHealth += ActualHeal;

    OnHealed.Broadcast(ActualHeal, HealInstigator);
    BroadcastHealthChange(ActualHeal);

    UE_LOG(LogTemp, Log, TEXT("%s healed for %.1f. Health: %.1f/%.1f"), 
        *GetOwner()->GetName(), ActualHeal, CurrentHealth, MaxHealth);

    if (GEngine)
    {
        FString DebugMessage = FString::Printf(TEXT("%s Health: %.0f/%.0f (+%.0f)"),
            *GetOwner()->GetName(), CurrentHealth, MaxHealth, ActualHeal);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, DebugMessage);
    }
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth, bool bScaleCurrentHealth)
{
    if (NewMaxHealth <= 0.0f)
    {
        return;
    }

    float OldMaxHealth = MaxHealth;
    MaxHealth = NewMaxHealth;

    if (bScaleCurrentHealth && OldMaxHealth > 0.0f)
    {
        float HealthPercent = CurrentHealth / OldMaxHealth;
        CurrentHealth = MaxHealth * HealthPercent;
    }
    else
    {
        CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);
    }

    BroadcastHealthChange(0.0f);
}

void UHealthComponent::ReviveWithHealth(float ReviveHealth)
{
    if (!bIsDead)
    {
        return;
    }

    bIsDead = false;
    CurrentHealth = FMath::Clamp(ReviveHealth, 1.0f, MaxHealth);
    LastDamageInstigator = nullptr;

    if (UCombatComponent* CombatComp = GetOwner()->FindComponentByClass<UCombatComponent>())
    {
        CombatComp->CombatStateTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Dead")));
    }

    OnRevived.Broadcast();
    BroadcastHealthChange(CurrentHealth);

    UE_LOG(LogTemp, Log, TEXT("%s revived with %.1f health"), *GetOwner()->GetName(), CurrentHealth);
}

float UHealthComponent::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void UHealthComponent::HandleDeath(AActor* KilledBy)
{
    bIsDead = true;
    CurrentHealth = 0.0f;

    if (UCombatComponent* CombatComp = GetOwner()->FindComponentByClass<UCombatComponent>())
    {
        CombatComp->CombatStateTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Dead")));
        
        if (CombatComp->IsBlocking())
        {
            CombatComp->EndBlock();
        }
        if (CombatComp->IsAttacking())
        {
            CombatComp->EndAttack();
        }
    }

    OnDeath.Broadcast(KilledBy);

    UE_LOG(LogTemp, Warning, TEXT("%s has died! Killed by: %s"), 
        *GetOwner()->GetName(), 
        KilledBy ? *KilledBy->GetName() : TEXT("Unknown"));

    if (GEngine)
    {
        FString DebugMessage = FString::Printf(TEXT("%s DIED!"), *GetOwner()->GetName());
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, DebugMessage);
    }
}

void UHealthComponent::BroadcastHealthChange(float HealthDelta)
{
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, HealthDelta);
}