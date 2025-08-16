#include "CombatComponent.h"
#include "../Data/AttackDataAsset.h"
#include "../Data/CombatRulesDataAsset.h"
#include "DamageCalculator.h"
#include "HealthComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (CombatRules)
    {
        CurrentPoise = CombatRules->CombatRules.MaxPoise;
    }
    else
    {
        CurrentPoise = 100.0f; // Default value if no CombatRules set
    }

    DamageCalculator = NewObject<UDamageCalculator>(this);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CombatRules && !bPoiseRegenActive && CurrentPoise < CombatRules->CombatRules.MaxPoise)
    {
        PoiseRegenDelayTime += DeltaTime;
        if (PoiseRegenDelayTime >= CombatRules->CombatRules.PoiseRegenDelay)
        {
            StartPoiseRegen();
        }
    }
}

bool UCombatComponent::StartAttack(const FGameplayTag& AttackTag)
{
    if (IsStaggered() || IsAttacking())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot attack: Staggered=%s, Attacking=%s"), 
            IsStaggered() ? TEXT("true") : TEXT("false"),
            IsAttacking() ? TEXT("true") : TEXT("false"));
        return false;
    }

    UAttackDataAsset** AttackDataPtr = AttackDataMap.Find(AttackTag);
    if (!AttackDataPtr || !(*AttackDataPtr))
    {
        UE_LOG(LogTemp, Error, TEXT("Attack failed: No AttackData found for tag %s. Make sure AttackDataMap is configured in Blueprint!"), 
            *AttackTag.ToString());
        return false;
    }

    CurrentAttackData = *AttackDataPtr;
    AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")));

    UE_LOG(LogTemp, Warning, TEXT("ATTACK STARTED: %s (Damage: %.1f, Knockback: %.1f)"), 
        *CurrentAttackData->AttackData.AttackName.ToString(),
        CurrentAttackData->AttackData.BaseDamage,
        CurrentAttackData->AttackData.Knockback);

    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (CurrentAttackData->AttackData.AttackMontage)
        {
            Character->PlayAnimMontage(CurrentAttackData->AttackData.AttackMontage);
        }
    }

    OnAttackStarted.Broadcast(AttackTag, CurrentAttackData);
    
    // Auto-end attack after 1 second for testing (will be handled by animation notifies in Phase 2)
    FTimerHandle TempAttackTimer;
    GetWorld()->GetTimerManager().SetTimer(TempAttackTimer, this, &UCombatComponent::EndAttack, 1.0f, false);
    
    return true;
}

void UCombatComponent::EndAttack()
{
    RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")));
    CurrentAttackData = nullptr;
    OnAttackEnded.Broadcast();
    UE_LOG(LogTemp, Warning, TEXT("Attack Ended"));
}

bool UCombatComponent::StartBlock()
{
    if (IsStaggered() || IsAttacking())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot block: Staggered=%s, Attacking=%s"),
            IsStaggered() ? TEXT("true") : TEXT("false"),
            IsAttacking() ? TEXT("true") : TEXT("false"));
        return false;
    }

    AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")));
    OnBlockStarted.Broadcast(true);
    UE_LOG(LogTemp, Warning, TEXT("BLOCK STARTED - Damage will be reduced by 40%%"));
    return true;
}

void UCombatComponent::EndBlock()
{
    RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")));
    OnBlockEnded.Broadcast();
    UE_LOG(LogTemp, Warning, TEXT("Block Ended"));
}

bool UCombatComponent::TryParry()
{
    if (IsStaggered() || IsAttacking() || IsParrying())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot parry: Staggered=%s, Attacking=%s, Parrying=%s"),
            IsStaggered() ? TEXT("true") : TEXT("false"),
            IsAttacking() ? TEXT("true") : TEXT("false"),
            IsParrying() ? TEXT("true") : TEXT("false"));
        return false;
    }

    AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Parrying")));
    
    float ParryDuration = CombatRules ? CombatRules->CombatRules.ParryWindowDuration : 0.3f;
    
    UE_LOG(LogTemp, Warning, TEXT("PARRY WINDOW OPEN for %.2f seconds! Counter enemy attacks now!"), ParryDuration);
    
    GetWorld()->GetTimerManager().SetTimer(
        ParryWindowTimerHandle,
        this,
        &UCombatComponent::EndParryWindow,
        ParryDuration,
        false
    );

    return true;
}

void UCombatComponent::EndParryWindow()
{
    RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Parrying")));
    UE_LOG(LogTemp, Warning, TEXT("Parry Window Closed"));
}

void UCombatComponent::ApplyVulnerability(int32 Charges)
{
    VulnerabilityCharges = FMath::Max(Charges, 1);
    AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Vulnerable")));
    
    float VulnerabilityDuration = CombatRules ? CombatRules->CombatRules.VulnerabilityDuration : 1.0f;
    
    GetWorld()->GetTimerManager().SetTimer(
        VulnerabilityTimerHandle,
        this,
        &UCombatComponent::EndVulnerability,
        VulnerabilityDuration,
        false
    );

    OnVulnerabilityApplied.Broadcast();
}

void UCombatComponent::ConsumeVulnerabilityCharge()
{
    if (VulnerabilityCharges > 0)
    {
        VulnerabilityCharges--;
        if (VulnerabilityCharges <= 0)
        {
            EndVulnerability();
        }
    }
}

void UCombatComponent::EndVulnerability()
{
    VulnerabilityCharges = 0;
    RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Vulnerable")));
    GetWorld()->GetTimerManager().ClearTimer(VulnerabilityTimerHandle);
}

void UCombatComponent::TakePoiseDamage(float Damage)
{
    CurrentPoise = FMath::Max(0.0f, CurrentPoise - Damage);
    PoiseRegenDelayTime = 0.0f;
    bPoiseRegenActive = false;

    if (CurrentPoise <= 0.0f && !IsStaggered())
    {
        AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Staggered")));
        OnStaggered.Broadcast();

        if (IsAttacking())
        {
            EndAttack();
        }
        if (IsBlocking())
        {
            EndBlock();
        }

        GetWorld()->GetTimerManager().SetTimer(
            StaggerRecoveryTimerHandle,
            this,
            &UCombatComponent::RecoverFromStagger,
            2.0f,
            false
        );
    }
}

void UCombatComponent::RecoverFromStagger()
{
    RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Staggered")));
    ResetPoise();
}

void UCombatComponent::ResetPoise()
{
    CurrentPoise = CombatRules ? CombatRules->CombatRules.MaxPoise : 100.0f;
    PoiseRegenDelayTime = 0.0f;
    bPoiseRegenActive = false;
}

void UCombatComponent::StartPoiseRegen()
{
    bPoiseRegenActive = true;
    GetWorld()->GetTimerManager().SetTimer(
        PoiseRegenTimerHandle,
        this,
        &UCombatComponent::RegenPoise,
        0.1f,
        true
    );
}

void UCombatComponent::RegenPoise()
{
    if (!IsStaggered() && CombatRules)
    {
        CurrentPoise = FMath::Min(
            CurrentPoise + (CombatRules->CombatRules.PoiseRegenRate * 0.1f),
            CombatRules->CombatRules.MaxPoise
        );

        if (CurrentPoise >= CombatRules->CombatRules.MaxPoise)
        {
            bPoiseRegenActive = false;
            GetWorld()->GetTimerManager().ClearTimer(PoiseRegenTimerHandle);
        }
    }
}

void UCombatComponent::ProcessHit(AActor* HitActor, const FGameplayTag& AttackTag)
{
    if (!HitActor || !CurrentAttackData || !DamageCalculator)
    {
        return;
    }

    UCombatComponent* TargetCombat = HitActor->FindComponentByClass<UCombatComponent>();
    if (!TargetCombat)
    {
        return;
    }

    if (TargetCombat->IsParrying() && CombatRules && CombatRules->CanParry(CurrentAttackData->AttackData.AttackTags))
    {
        TargetCombat->OnParrySuccess.Broadcast(GetOwner());
        int32 DefaultCharges = CombatRules ? CombatRules->CombatRules.DefaultVulnerabilityCharges : 1;
        ApplyVulnerability(DefaultCharges);
        return;
    }

    DamageCalculator->ProcessDamage(
        GetOwner(),
        HitActor,
        CurrentAttackData,
        CombatStateTags,
        TargetCombat->CombatStateTags
    );

    TargetCombat->TakePoiseDamage(CurrentAttackData->AttackData.StaggerDamage);
}

bool UCombatComponent::IsAttacking() const
{
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")));
}

bool UCombatComponent::IsBlocking() const
{
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")));
}

bool UCombatComponent::IsParrying() const
{
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Parrying")));
}

bool UCombatComponent::IsVulnerable() const
{
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Vulnerable")));
}

bool UCombatComponent::IsStaggered() const
{
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Staggered")));
}

void UCombatComponent::AddCombatStateTag(const FGameplayTag& Tag)
{
    CombatStateTags.AddTag(Tag);
}

void UCombatComponent::RemoveCombatStateTag(const FGameplayTag& Tag)
{
    CombatStateTags.RemoveTag(Tag);
}

bool UCombatComponent::HasCombatStateTag(const FGameplayTag& Tag) const
{
    return CombatStateTags.HasTag(Tag);
}