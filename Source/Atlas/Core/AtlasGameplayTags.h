#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Singleton containing native gameplay tags for Atlas
 */
struct ATLAS_API FAtlasGameplayTags
{
public:
    static const FAtlasGameplayTags& Get() { return GameplayTags; }
    static void InitializeNativeTags();

    // Attack Type Tags
    FGameplayTag Attack_Type_Jab;
    FGameplayTag Attack_Type_Heavy;
    FGameplayTag Attack_Type_Special;
    FGameplayTag Attack_Type_Soul;
    FGameplayTag Attack_Type_Catch;

    // Attack Property Tags
    FGameplayTag Attack_Property_Blockable;
    FGameplayTag Attack_Property_Parryable;
    FGameplayTag Attack_Property_Unblockable;
    FGameplayTag Attack_Property_Unparryable;

    // Combat State Tags
    FGameplayTag Combat_State_Attacking;
    FGameplayTag Combat_State_Blocking;
    FGameplayTag Combat_State_Parrying;
    FGameplayTag Combat_State_Vulnerable;
    FGameplayTag Combat_State_Staggered;
    FGameplayTag Combat_State_Invincible;
    FGameplayTag Combat_State_Dead;

    // Damage Type Tags
    FGameplayTag Damage_Type_Physical;
    FGameplayTag Damage_Type_Fire;
    FGameplayTag Damage_Type_Electric;
    FGameplayTag Damage_Type_Poison;
    FGameplayTag Damage_Multiplier_Vulnerability;

    // Character Type Tags
    FGameplayTag Character_Type_Player;
    FGameplayTag Character_Type_Enemy;
    FGameplayTag Character_Type_Boss;

    // Ability Tags
    FGameplayTag Ability_Focus;
    FGameplayTag Ability_SoftLock;

    // Interactable Tags
    FGameplayTag Interactable_Type_Vent;
    FGameplayTag Interactable_Type_Valve;
    FGameplayTag Interactable_State_Ready;
    FGameplayTag Interactable_State_Cooldown;

private:
    static FAtlasGameplayTags GameplayTags;
};