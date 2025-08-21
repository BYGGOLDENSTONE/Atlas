#include "HealthComponent.h"
#include "ActionManagerComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    CurrentPoise = MaxPoise;
    bIsDead = false;
    bIsStaggered = false;
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

    // Damage taken - will add UI later

    // Will add damage UI feedback later

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

    if (UActionManagerComponent* ActionManager = GetOwner()->FindComponentByClass<UActionManagerComponent>())
    {
        ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Dead")));
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

    if (UActionManagerComponent* ActionManager = GetOwner()->FindComponentByClass<UActionManagerComponent>())
    {
        ActionManager->AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Dead")));
        
        // Interrupt any ongoing actions
        ActionManager->InterruptCurrentAction();
        if (ActionManager->IsBlocking())
        {
            ActionManager->EndBlock();
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

void UHealthComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bPoiseRegenActive && CurrentPoise < MaxPoise && !bIsStaggered)
    {
        PoiseRegenDelayTime += DeltaTime;
        if (PoiseRegenDelayTime >= PoiseRegenDelay)
        {
            StartPoiseRegen();
        }
    }
}

void UHealthComponent::TakePoiseDamage(float PoiseDamage, AActor* DamageInstigator)
{
    if (bIsStaggered || PoiseDamage <= 0.0f)
    {
        return;
    }
    
    float ActualDamage = FMath::Min(PoiseDamage, CurrentPoise);
    CurrentPoise = FMath::Max(0.0f, CurrentPoise - ActualDamage);
    PoiseRegenDelayTime = 0.0f;
    bPoiseRegenActive = false;
    
    BroadcastPoiseChange(-ActualDamage);
    
    // Poise damage taken - will add UI later
    
    if (CurrentPoise <= 0.0f && !bIsStaggered)
    {
        bIsStaggered = true;
        OnStaggered.Broadcast();
        
        if (UActionManagerComponent* ActionManager = GetOwner()->FindComponentByClass<UActionManagerComponent>())
        {
            ActionManager->AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Staggered")));
            
            // Interrupt any ongoing actions
            ActionManager->InterruptCurrentAction();
            if (ActionManager->IsBlocking())
            {
                ActionManager->EndBlock();
            }
        }
        
        PlayHitReaction();
        
        GetWorld()->GetTimerManager().SetTimer(
            StaggerRecoveryTimerHandle,
            this,
            &UHealthComponent::RecoverFromStagger,
            StaggerDuration,
            false
        );
        
        if (GEngine)
        {
            FString DebugMessage = FString::Printf(TEXT("%s STAGGERED!"), *GetOwner()->GetName());
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, DebugMessage);
        }
    }
}

void UHealthComponent::ResetPoise()
{
    CurrentPoise = MaxPoise;
    PoiseRegenDelayTime = 0.0f;
    bPoiseRegenActive = false;
    BroadcastPoiseChange(MaxPoise - CurrentPoise);
}

float UHealthComponent::GetPoisePercent() const
{
    return MaxPoise > 0.0f ? CurrentPoise / MaxPoise : 0.0f;
}

void UHealthComponent::PlayHitReaction()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
        {
            UAnimMontage* HitMontage = nullptr;
            
            if (bIsStaggered)
            {
                AnimInstance->Montage_Play(HitMontage, 1.0f);
            }
            else
            {
                AnimInstance->Montage_Play(HitMontage, 1.0f);
            }
        }
    }
}

void UHealthComponent::BroadcastPoiseChange(float PoiseDelta)
{
    OnPoiseChanged.Broadcast(CurrentPoise, MaxPoise, PoiseDelta);
}

void UHealthComponent::StartPoiseRegen()
{
    bPoiseRegenActive = true;
    GetWorld()->GetTimerManager().SetTimer(
        PoiseRegenTimerHandle,
        this,
        &UHealthComponent::RegenPoise,
        0.1f,
        true
    );
}

void UHealthComponent::RegenPoise()
{
    if (!bIsStaggered)
    {
        float RegenAmount = PoiseRegenRate * 0.1f;
        float OldPoise = CurrentPoise;
        CurrentPoise = FMath::Min(CurrentPoise + RegenAmount, MaxPoise);
        
        if (CurrentPoise != OldPoise)
        {
            BroadcastPoiseChange(CurrentPoise - OldPoise);
        }
        
        if (CurrentPoise >= MaxPoise)
        {
            bPoiseRegenActive = false;
            GetWorld()->GetTimerManager().ClearTimer(PoiseRegenTimerHandle);
        }
    }
}

void UHealthComponent::RecoverFromStagger()
{
    bIsStaggered = false;
    
    if (UActionManagerComponent* ActionManager = GetOwner()->FindComponentByClass<UActionManagerComponent>())
    {
        ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Staggered")));
    }
    
    OnStaggerRecovered.Broadcast();
    ResetPoise();
    
    if (GEngine)
    {
        FString DebugMessage = FString::Printf(TEXT("%s recovered from stagger"), *GetOwner()->GetName());
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, DebugMessage);
    }
}