#include "StationIntegrityDataAsset.h"

float UStationIntegrityDataAsset::GetIntegrityCostForAbility(const FGameplayTag& AbilityTag) const
{
    for (const FAbilityIntegrityCost& HighRisk : HighRiskAbilities)
    {
        if (HighRisk.AbilityTag.MatchesTagExact(AbilityTag))
        {
            return HighRisk.IntegrityCost;
        }
    }

    for (const FAbilityIntegrityCost& MediumRisk : MediumRiskAbilities)
    {
        if (MediumRisk.AbilityTag.MatchesTagExact(AbilityTag))
        {
            return MediumRisk.IntegrityCost;
        }
    }

    for (const FAbilityIntegrityCost& LowRisk : LowRiskAbilities)
    {
        if (LowRisk.AbilityTag.MatchesTagExact(AbilityTag))
        {
            return LowRisk.IntegrityCost;
        }
    }

    return 0.0f;
}

bool UStationIntegrityDataAsset::IsHighRiskAbility(const FGameplayTag& AbilityTag) const
{
    for (const FAbilityIntegrityCost& HighRisk : HighRiskAbilities)
    {
        if (HighRisk.AbilityTag.MatchesTagExact(AbilityTag))
        {
            return true;
        }
    }
    return false;
}

bool UStationIntegrityDataAsset::IsMediumRiskAbility(const FGameplayTag& AbilityTag) const
{
    for (const FAbilityIntegrityCost& MediumRisk : MediumRiskAbilities)
    {
        if (MediumRisk.AbilityTag.MatchesTagExact(AbilityTag))
        {
            return true;
        }
    }
    return false;
}

bool UStationIntegrityDataAsset::IsLowRiskAbility(const FGameplayTag& AbilityTag) const
{
    for (const FAbilityIntegrityCost& LowRisk : LowRiskAbilities)
    {
        if (LowRisk.AbilityTag.MatchesTagExact(AbilityTag))
        {
            return true;
        }
    }
    return false;
}