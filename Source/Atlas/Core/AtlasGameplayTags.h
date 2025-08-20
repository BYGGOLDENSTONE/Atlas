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
    FGameplayTag Attack_Property_Unblockable;

    // Combat State Tags
    FGameplayTag Combat_State_Attacking;
    FGameplayTag Combat_State_Blocking;
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
    
    // Station Integrity Risk Tags
    FGameplayTag Ability_Risk_High;
    FGameplayTag Ability_Risk_Medium;
    FGameplayTag Ability_Risk_Low;

    // Interactable Tags
    FGameplayTag Interactable_Type_Vent;
    FGameplayTag Interactable_Type_Valve;
    FGameplayTag Interactable_State_Ready;
    FGameplayTag Interactable_State_Cooldown;

    // NEW Action System Tags - All abilities
    FGameplayTag Action_Ability_Dash;
    FGameplayTag Action_Ability_Block;
    FGameplayTag Action_Ability_BasicAttack;
    FGameplayTag Action_Ability_HeavyAttack;
    FGameplayTag Action_Ability_KineticPulse;
    FGameplayTag Action_Ability_DebrisPull;
    FGameplayTag Action_Ability_CoolantSpray;
    FGameplayTag Action_Ability_SystemHack;
    FGameplayTag Action_Ability_FloorDestabilizer;
    FGameplayTag Action_Ability_ImpactGauntlet;
    FGameplayTag Action_Ability_LocalizedEMP;
    FGameplayTag Action_Ability_SeismicStamp;
    FGameplayTag Action_Ability_GravityAnchor;
    FGameplayTag Action_Ability_AirlockBreach;

    // Vulnerability Tier Tags
    FGameplayTag Vulnerability_Tier_Stunned;
    FGameplayTag Vulnerability_Tier_Crippled;
    FGameplayTag Vulnerability_Tier_Exposed;

private:
    static FAtlasGameplayTags GameplayTags;
};