#include "CombatComponent.h"
#include "../Data/AttackDataAsset.h"
#include "../Data/CombatRulesDataAsset.h"
#include "DamageCalculator.h"
#include "HealthComponent.h"
#include "VulnerabilityComponent.h"
#include "../Characters/GameCharacterBase.h"
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
    
    VulnerabilityComponent = GetOwner()->FindComponentByClass<UVulnerabilityComponent>();
    if (!VulnerabilityComponent)
    {
        VulnerabilityComponent = NewObject<UVulnerabilityComponent>(GetOwner(), UVulnerabilityComponent::StaticClass(), TEXT("VulnerabilityComponent"));
        VulnerabilityComponent->RegisterComponent();
    }
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
    LastCombatActionTime = GetWorld()->GetTimeSeconds();


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
}

bool UCombatComponent::StartBlock()
{
    if (IsStaggered() || IsAttacking())
    {
        return false;
    }

    AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")));
    LastCombatActionTime = GetWorld()->GetTimeSeconds();
    OnBlockStarted.Broadcast(true);
    return true;
}

void UCombatComponent::EndBlock()
{
    RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")));
    OnBlockEnded.Broadcast();
}



void UCombatComponent::ApplyVulnerability(int32 Charges)
{
    if (VulnerabilityComponent)
    {
        float Duration = CombatRules ? CombatRules->CombatRules.VulnerabilityDuration : 1.0f;
        VulnerabilityComponent->ApplyVulnerability(Charges, Duration);
        OnVulnerabilityApplied.Broadcast();
    }
    else
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
}

void UCombatComponent::ConsumeVulnerabilityCharge()
{
    if (VulnerabilityComponent)
    {
        VulnerabilityComponent->ConsumeCharge();
    }
    else
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
    LastCombatActionTime = GetWorld()->GetTimeSeconds();

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


    DamageCalculator->ProcessDamage(
        GetOwner(),
        HitActor,
        CurrentAttackData,
        CombatStateTags,
        TargetCombat->CombatStateTags
    );

    TargetCombat->TakePoiseDamage(CurrentAttackData->AttackData.StaggerDamage);
}

void UCombatComponent::ProcessHitFromAnimation(AGameCharacterBase* HitCharacter)
{
    if (!HitCharacter || !CurrentAttackData || !DamageCalculator)
    {
        return;
    }

    UCombatComponent* TargetCombat = HitCharacter->FindComponentByClass<UCombatComponent>();
    if (!TargetCombat)
    {
        return;
    }


    DamageCalculator->ProcessDamage(
        GetOwner(),
        HitCharacter,
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


bool UCombatComponent::IsVulnerable() const
{
    if (VulnerabilityComponent)
    {
        return VulnerabilityComponent->IsVulnerable();
    }
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Vulnerable")));
}

bool UCombatComponent::HasIFrames() const
{
    if (VulnerabilityComponent)
    {
        return VulnerabilityComponent->HasIFrames();
    }
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.IFrames")));
}

void UCombatComponent::ApplyVulnerabilityWithIFrames(int32 Charges, bool bGrantIFrames)
{
    if (VulnerabilityComponent)
    {
        float Duration = CombatRules ? CombatRules->CombatRules.VulnerabilityDuration : 1.0f;
        VulnerabilityComponent->ApplyVulnerability(Charges, Duration);
        
        if (bGrantIFrames && VulnerabilityComponent->bEnableIFrames)
        {
            VulnerabilityComponent->StartIFrames();
        }
        
        OnVulnerabilityApplied.Broadcast();
    }
    else
    {
        ApplyVulnerability(Charges);
    }
}


bool UCombatComponent::IsStaggered() const
{
    return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Staggered")));
}

bool UCombatComponent::IsInCombat() const
{
    // Check if we're actively attacking, blocking, or recently damaged
    if (IsAttacking() || IsBlocking() || IsStaggered())
    {
        return true;
    }
    
    // Check if we've been in combat within the last 3 seconds
    float TimeSinceLastAction = GetTimeSinceLastCombatAction();
    return TimeSinceLastAction < 3.0f;
}

float UCombatComponent::GetTimeSinceLastCombatAction() const
{
    if (LastCombatActionTime <= 0.0f)
    {
        return 999.0f; // Never been in combat
    }
    
    return GetWorld() ? GetWorld()->GetTimeSeconds() - LastCombatActionTime : 999.0f;
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

void UCombatComponent::StartBlocking()
{
    StartBlock();
}

void UCombatComponent::StopBlocking()
{
    EndBlock();
}


void UCombatComponent::DealDamageToTarget(AActor* Target, float Damage, const FGameplayTagContainer& AttackTags)
{
    if (!Target || !DamageCalculator)
    {
        return;
    }

    UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();
    if (TargetHealth)
    {
        TargetHealth->TakeDamage(Damage, GetOwner());
    }

    UCombatComponent* TargetCombat = Target->FindComponentByClass<UCombatComponent>();
    if (TargetCombat)
    {
        if (AttackTags.HasTag(FGameplayTag::RequestGameplayTag("Combat.Status.Stunned")))
        {
            TargetCombat->AddCombatStateTag(FGameplayTag::RequestGameplayTag("Combat.State.Staggered"));
        }
    }
}

UAnimMontage* UCombatComponent::GetAttackMontage(const FGameplayTag& AttackTag) const
{
    if (AttackDataMap.Contains(AttackTag))
    {
        UAttackDataAsset* AttackData = AttackDataMap[AttackTag];
        if (AttackData)
        {
            return AttackData->AttackData.AttackMontage;
        }
    }
    return nullptr;
}