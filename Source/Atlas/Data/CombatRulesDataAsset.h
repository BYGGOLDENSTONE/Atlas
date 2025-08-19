#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CombatRulesDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FCombatMultiplier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Multiplier")
    FGameplayTag RequiredTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Multiplier")
    float Multiplier = 1.0f;

    FCombatMultiplier() {}
};

USTRUCT(BlueprintType)
struct FCombatRules
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
    float BlockDamageReduction = 0.4f;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vulnerability")
    float VulnerabilityMultiplier = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vulnerability")
    float VulnerabilityDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vulnerability")
    int32 DefaultVulnerabilityCharges = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise")
    float MaxPoise = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise")
    float PoiseRegenRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Poise")
    float PoiseRegenDelay = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stagger")
    float StaggerDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stagger")
    float WallImpactStaggerDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stagger")
    float FloorImpactRagdollDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
    float FocusRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
    float FocusTargetPriorityWeight = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
    float DashDistance = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
    float DashCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
    float DashSpeed = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
    float DashIFrameDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float CombatTimeoutDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback")
    float MinKnockbackForWallImpact = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback")
    float WallImpactCheckRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactables")
    float InteractableCooldown = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactables")
    float VentProjectileSpeed = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactables")
    float VentPoiseDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactables")
    float ValveAoERadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Multipliers")
    TArray<FCombatMultiplier> DamageMultipliers;

    FCombatRules() {}
};

UCLASS(BlueprintType)
class ATLAS_API UCombatRulesDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Rules")
    FCombatRules CombatRules;

    float CalculateFinalDamage(float BaseDamage, const FGameplayTagContainer& AttackerTags, const FGameplayTagContainer& DefenderTags) const;
    bool CanBlock(const FGameplayTagContainer& AttackTags) const;
};