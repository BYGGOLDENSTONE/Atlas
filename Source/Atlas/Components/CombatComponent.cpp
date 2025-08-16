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

    DamageCalculator = NewObject<UDamageCalculator>(this);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bPoiseRegenActive && CurrentPoise < CombatRules->CombatRules.MaxPoise)
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
        return false;
    }

    UAttackDataAsset** AttackDataPtr = AttackDataMap.Find(AttackTag);
    if (!AttackDataPtr || !(*AttackDataPtr))
    {
        return false;
    }

    CurrentAttackData = *AttackDataPtr;
    AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")));

    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (CurrentAttackData->AttackData.AttackMontage)
        {
            Character->PlayAnimMontage(CurrentAttackData->AttackData.AttackMontage);
        }
    }

    OnAttackStarted.Broadcast(AttackTag, CurrentAttackData);
    return true;
}

void UCombatComponent::EndAttack()
{
    RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")));
    CurrentAttackData = nullptr;
    OnAttackEnded.Broadcast();
}

bool UCombatComponent::StartBlock()
{
    if (IsStaggered() || IsAttacking())
    {
        return false;
    }

    AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")));
    OnBlockStarted.Broadcast(true);
    return true;
}

void UCombatComponent::EndBlock()
{
    RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")));
    OnBlockEnded.Broadcast();
}

bool UCombatComponent::TryParry()
{
    if (IsStaggered() || IsAttacking() || IsParrying())
    {
        return false;
    }

    AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Parrying")));
    
    GetWorld()->GetTimerManager().SetTimer(
        ParryWindowTimerHandle,
        this,
        &UCombatComponent::EndParryWindow,
        CombatRules->CombatRules.ParryWindowDuration,
        false
    );

    return true;
}

void UCombatComponent::EndParryWindow()
{
    RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Parrying")));
}

void UCombatComponent::ApplyVulnerability(int32 Charges)
{
    VulnerabilityCharges = FMath::Max(Charges, 1);
    AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Vulnerable")));
    
    GetWorld()->GetTimerManager().SetTimer(
        VulnerabilityTimerHandle,
        this,
        &UCombatComponent::EndVulnerability,
        CombatRules->CombatRules.VulnerabilityDuration,
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
    CurrentPoise = CombatRules->CombatRules.MaxPoise;
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
    if (!IsStaggered())
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

    if (TargetCombat->IsParrying() && CombatRules->CanParry(CurrentAttackData->AttackData.AttackTags))
    {
        TargetCombat->OnParrySuccess.Broadcast(GetOwner());
        ApplyVulnerability(CombatRules->CombatRules.DefaultVulnerabilityCharges);
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