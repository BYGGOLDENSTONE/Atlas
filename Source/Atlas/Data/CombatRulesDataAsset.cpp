#include "CombatRulesDataAsset.h"

float UCombatRulesDataAsset::CalculateFinalDamage(float BaseDamage, const FGameplayTagContainer& AttackerTags, const FGameplayTagContainer& DefenderTags) const
{
    float FinalDamage = BaseDamage;

    if (DefenderTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking"))))
    {
        FinalDamage *= (1.0f - CombatRules.BlockDamageReduction);
    }

    if (DefenderTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Vulnerable"))))
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

bool UCombatRulesDataAsset::CanParry(const FGameplayTagContainer& AttackTags) const
{
    return !AttackTags.HasTag(FGameplayTag::RequestGameplayTag(FName("Attack.Property.Unparryable")));
}