#include "CombatRulesDataAsset.h"

float UCombatRulesDataAsset::CalculateFinalDamage(float BaseDamage, const FGameplayTagContainer& AttackerTags, const FGameplayTagContainer& DefenderTags) const
{
    float FinalDamage = BaseDamage;

    if (DefenderTags.HasTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.Blocking"))))
    {
        FinalDamage *= (1.0f - CombatRules.BlockDamageReduction);
    }

    if (DefenderTags.HasTag(FGameplayTag::RequestGameplayTag(FName("State.Status.Vulnerable"))))
    {
        FinalDamage *= CombatRules.VulnerabilityMultiplier;
    }

    for (const FCombatMultiplier& Multiplier : CombatRules.DamageMultipliers)
    {
        if (DefenderTags.HasTag(Multiplier.RequiredTag) || AttackerTags.HasTag(Multiplier.RequiredTag))
        {
            FinalDamage *= Multiplier.Multiplier;
        }
    }

    return FinalDamage;
}

bool UCombatRulesDataAsset::CanBlock(const FGameplayTagContainer& AttackTags) const
{
    return !AttackTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Attack.Property.Unblockable")));
}

