#include "DamageCalculator.h"
#include "../Data/AttackDataAsset.h"
#include "../Data/CombatRulesDataAsset.h"
#include "CombatComponent.h"
#include "VulnerabilityComponent.h"
#include "HealthComponent.h"
#include "WallImpactComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

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

        if (bWasVulnerable && TargetVuln && DamageInfo.FinalDamage > 0.0f)
        {
            // New tier system: notify of critical hit instead of consuming charge
            TargetVuln->OnCriticalHitLanded();
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
    KnockbackDirection.Z = KnockbackUpwardForce;  // Configurable upward force
    KnockbackDirection.Normalize();
    
    // Debug visualization of knockback direction
    #if WITH_EDITOR
    if (UWorld* World = Target->GetWorld())
    {
        FVector StartPos = Target->GetActorLocation();
        FVector EndPos = StartPos + (KnockbackDirection * 300.0f);
        DrawDebugDirectionalArrow(World, StartPos, EndPos, 
            50.0f, FColor::Magenta, false, 3.0f, 0, 5.0f);
        DrawDebugString(World, StartPos + FVector(0, 0, 200), 
            FString::Printf(TEXT("KNOCKBACK\nForce: %.0f\nDirection: %.2f, %.2f, %.2f"), 
                KnockbackForce, KnockbackDirection.X, KnockbackDirection.Y, KnockbackDirection.Z),
            nullptr, FColor::Magenta, 3.0f, true, 1.5f);
    }
    #endif
    
    UE_LOG(LogTemp, Warning, TEXT("Knockback Applied: Force=%.1f, Direction=(%.2f, %.2f, %.2f), Ragdoll=%s"),
        KnockbackForce, KnockbackDirection.X, KnockbackDirection.Y, KnockbackDirection.Z, 
        bCauseRagdoll ? TEXT("Yes") : TEXT("No"));

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

        TargetCharacter->LaunchCharacter(KnockbackDirection * KnockbackForce * RagdollKnockbackMultiplier, true, true);
        
        // Start collision tracking for wall/floor impact detection
        if (KnockbackForce > 150.0f)
        {
            // Try to get WallImpactComponent from the attacker
            if (!WallImpactComponent)
            {
                WallImpactComponent = Attacker->FindComponentByClass<UWallImpactComponent>();
            }
            
            // If not on attacker, create one temporarily
            if (!WallImpactComponent)
            {
                WallImpactComponent = NewObject<UWallImpactComponent>(this);
                WallImpactComponent->RegisterComponent();
            }
            
            if (WallImpactComponent)
            {
                WallImpactComponent->StartKnockbackTracking(Target, KnockbackForce);
            }
        }
    }
    else
    {
        TargetCharacter->LaunchCharacter(KnockbackDirection * KnockbackForce * StandardKnockbackMultiplier, true, false);
        
        // Start collision tracking for wall/floor impact detection (non-ragdoll)
        if (KnockbackForce > 150.0f)
        {
            // Try to get WallImpactComponent from the attacker
            if (!WallImpactComponent)
            {
                WallImpactComponent = Attacker->FindComponentByClass<UWallImpactComponent>();
            }
            
            // If not on attacker, create one temporarily
            if (!WallImpactComponent)
            {
                WallImpactComponent = NewObject<UWallImpactComponent>(this);
                WallImpactComponent->RegisterComponent();
            }
            
            if (WallImpactComponent)
            {
                WallImpactComponent->StartKnockbackTracking(Target, KnockbackForce);
            }
        }
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