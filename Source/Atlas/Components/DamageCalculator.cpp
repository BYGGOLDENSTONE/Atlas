#include "DamageCalculator.h"
#include "../Data/AttackDataAsset.h"
#include "../Data/CombatRulesDataAsset.h"
#include "CombatComponent.h"
#include "VulnerabilityComponent.h"
#include "HealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"

FDamageInfo UDamageCalculator::ProcessDamage(
    AActor* Attacker,
    AActor* Target,
    const UAttackDataAsset* AttackData,
    const FGameplayTagContainer& AttackerTags,
    const FGameplayTagContainer& DefenderTags)
{
    FDamageInfo DamageInfo;
    
    if (!Attacker || !Target || !AttackData)
    {
        return DamageInfo;
    }

    DamageInfo.Attacker = Attacker;
    DamageInfo.Target = Target;
    DamageInfo.BaseDamage = AttackData->AttackData.BaseDamage;
    DamageInfo.Knockback = AttackData->AttackData.Knockback;
    DamageInfo.bCausedRagdoll = AttackData->AttackData.bCausesRagdoll;
    DamageInfo.AttackTags = AttackData->AttackData.AttackTags;

    UCombatComponent* TargetCombat = Target->FindComponentByClass<UCombatComponent>();
    UVulnerabilityComponent* TargetVuln = Target->FindComponentByClass<UVulnerabilityComponent>();
    
    if (TargetVuln && TargetVuln->HasIFrames())
    {
        UE_LOG(LogTemp, Warning, TEXT("Attack avoided due to I-Frames!"));
        DamageInfo.FinalDamage = 0.0f;
        return DamageInfo;
    }
    
    if (TargetCombat && TargetCombat->CombatRules)
    {
        bool bWasBlocked = false;
        bool bWasVulnerable = false;
        
        float BaseDamageValue = DamageInfo.BaseDamage;
        
        if (TargetVuln && TargetVuln->IsVulnerable())
        {
            BaseDamageValue *= TargetVuln->GetDamageMultiplier();
            bWasVulnerable = true;
        }
        
        DamageInfo.FinalDamage = CalculateFinalDamage(
            BaseDamageValue,
            TargetCombat->CombatRules,
            AttackerTags,
            DefenderTags,
            bWasBlocked,
            bWasVulnerable
        );

        DamageInfo.bWasBlocked = bWasBlocked;
        DamageInfo.bWasVulnerable = bWasVulnerable;

        if (bWasVulnerable && TargetVuln)
        {
            TargetVuln->ConsumeCharge();
        }
    }
    else
    {
        DamageInfo.FinalDamage = DamageInfo.BaseDamage;
    }

    ApplyDamageToTarget(Target, DamageInfo);
    
    if (DamageInfo.Knockback > 0.0f)
    {
        ApplyKnockback(Target, Attacker, DamageInfo.Knockback, DamageInfo.bCausedRagdoll);
    }

    OnDamageCalculated.Broadcast(DamageInfo);
    LogDamageInfo(DamageInfo);

    return DamageInfo;
}

float UDamageCalculator::CalculateFinalDamage(
    float BaseDamage,
    const UCombatRulesDataAsset* CombatRules,
    const FGameplayTagContainer& AttackerTags,
    const FGameplayTagContainer& DefenderTags,
    bool& bWasBlocked,
    bool& bWasVulnerable)
{
    if (!CombatRules)
    {
        return BaseDamage;
    }

    float FinalDamage = BaseDamage;

    if (DefenderTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking"))))
    {
        bWasBlocked = true;
        FinalDamage *= (1.0f - CombatRules->CombatRules.BlockDamageReduction);
    }


    for (const FCombatMultiplier& Multiplier : CombatRules->CombatRules.DamageMultipliers)
    {
        if (DefenderTags.HasTag(Multiplier.RequiredTag) || AttackerTags.HasTag(Multiplier.RequiredTag))
        {
            FinalDamage *= Multiplier.Multiplier;
        }
    }

    return FinalDamage;
}

void UDamageCalculator::ApplyDamageToTarget(AActor* Target, const FDamageInfo& DamageInfo)
{
    if (!Target)
    {
        return;
    }

    UHealthComponent* HealthComp = Target->FindComponentByClass<UHealthComponent>();
    if (HealthComp)
    {
        HealthComp->TakeDamage(DamageInfo.FinalDamage, DamageInfo.Attacker);
    }
}

void UDamageCalculator::ApplyKnockback(AActor* Target, AActor* Attacker, float KnockbackForce, bool bCauseRagdoll)
{
    if (!Target || !Attacker)
    {
        return;
    }

    ACharacter* TargetCharacter = Cast<ACharacter>(Target);
    if (!TargetCharacter)
    {
        return;
    }

    FVector KnockbackDirection = (Target->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal();
    KnockbackDirection.Z = 0.3f;
    KnockbackDirection.Normalize();

    if (bCauseRagdoll)
    {
        if (UCharacterMovementComponent* MovementComp = TargetCharacter->GetCharacterMovement())
        {
            MovementComp->SetMovementMode(MOVE_Falling);
        }

        if (UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent())
        {
            CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        }

        TargetCharacter->LaunchCharacter(KnockbackDirection * KnockbackForce * 2.0f, true, true);
    }
    else
    {
        TargetCharacter->LaunchCharacter(KnockbackDirection * KnockbackForce, true, false);
    }
}

void UDamageCalculator::LogDamageInfo(const FDamageInfo& DamageInfo)
{
    FString LogMessage = FString::Printf(
        TEXT("Damage Dealt: %.1f (Base: %.1f) | Attacker: %s | Target: %s | Blocked: %s | Vulnerable: %s"),
        DamageInfo.FinalDamage,
        DamageInfo.BaseDamage,
        DamageInfo.Attacker ? *DamageInfo.Attacker->GetName() : TEXT("None"),
        DamageInfo.Target ? *DamageInfo.Target->GetName() : TEXT("None"),
        DamageInfo.bWasBlocked ? TEXT("Yes") : TEXT("No"),
        DamageInfo.bWasVulnerable ? TEXT("Yes") : TEXT("No")
    );

    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, LogMessage);
    }
}