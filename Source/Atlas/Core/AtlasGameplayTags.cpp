#include "AtlasGameplayTags.h"
#include "GameplayTagsManager.h"

FAtlasGameplayTags FAtlasGameplayTags::GameplayTags;

void FAtlasGameplayTags::InitializeNativeTags()
{
    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

    // Attack Type Tags
    GameplayTags.Attack_Type_Jab = Manager.AddNativeGameplayTag(FName("Attack.Type.Jab"), FString("Basic jab attack"));
    GameplayTags.Attack_Type_Heavy = Manager.AddNativeGameplayTag(FName("Attack.Type.Heavy"), FString("Heavy attack with knockback"));
    GameplayTags.Attack_Type_Special = Manager.AddNativeGameplayTag(FName("Attack.Type.Special"), FString("Special attack"));
    GameplayTags.Attack_Type_Soul = Manager.AddNativeGameplayTag(FName("Attack.Type.Soul"), FString("Soul attack - unblockable"));
    GameplayTags.Attack_Type_Catch = Manager.AddNativeGameplayTag(FName("Attack.Type.Catch"), FString("Catch special - anti-kiting attack"));

    // Attack Property Tags
    GameplayTags.Attack_Property_Blockable = Manager.AddNativeGameplayTag(FName("Attack.Property.Blockable"), FString("Attack can be blocked"));
    GameplayTags.Attack_Property_Unblockable = Manager.AddNativeGameplayTag(FName("Attack.Property.Unblockable"), FString("Attack cannot be blocked"));

    // Combat State Tags
    GameplayTags.Combat_State_Attacking = Manager.AddNativeGameplayTag(FName("Combat.State.Attacking"), FString("Character is currently attacking"));
    GameplayTags.Combat_State_Blocking = Manager.AddNativeGameplayTag(FName("Combat.State.Blocking"), FString("Character is blocking"));
    GameplayTags.Combat_State_Vulnerable = Manager.AddNativeGameplayTag(FName("Combat.State.Vulnerable"), FString("Character is vulnerable 8x damage"));
    GameplayTags.Combat_State_Staggered = Manager.AddNativeGameplayTag(FName("Combat.State.Staggered"), FString("Character is staggered/poise broken"));
    GameplayTags.Combat_State_Invincible = Manager.AddNativeGameplayTag(FName("Combat.State.Invincible"), FString("Character has invincibility frames"));
    GameplayTags.Combat_State_Dead = Manager.AddNativeGameplayTag(FName("Combat.State.Dead"), FString("Character is dead"));

    // Damage Type Tags
    GameplayTags.Damage_Type_Physical = Manager.AddNativeGameplayTag(FName("Damage.Type.Physical"), FString("Physical damage type"));
    GameplayTags.Damage_Type_Fire = Manager.AddNativeGameplayTag(FName("Damage.Type.Fire"), FString("Fire damage type"));
    GameplayTags.Damage_Type_Electric = Manager.AddNativeGameplayTag(FName("Damage.Type.Electric"), FString("Electric damage type"));
    GameplayTags.Damage_Type_Poison = Manager.AddNativeGameplayTag(FName("Damage.Type.Poison"), FString("Poison damage type"));
    GameplayTags.Damage_Multiplier_Vulnerability = Manager.AddNativeGameplayTag(FName("Damage.Multiplier.Vulnerability"), FString("Vulnerability damage multiplier active"));

    // Character Type Tags
    GameplayTags.Character_Type_Player = Manager.AddNativeGameplayTag(FName("Character.Type.Player"), FString("Player character"));
    GameplayTags.Character_Type_Enemy = Manager.AddNativeGameplayTag(FName("Character.Type.Enemy"), FString("Enemy character"));
    GameplayTags.Character_Type_Boss = Manager.AddNativeGameplayTag(FName("Character.Type.Boss"), FString("Boss character"));

    // Ability Tags
    GameplayTags.Ability_Focus = Manager.AddNativeGameplayTag(FName("Ability.Focus"), FString("Focus mode active"));
    GameplayTags.Ability_SoftLock = Manager.AddNativeGameplayTag(FName("Ability.SoftLock"), FString("Soft lock targeting active"));
    
    // Station Integrity Risk Tags
    GameplayTags.Ability_Risk_High = Manager.AddNativeGameplayTag(FName("Ability.Risk.High"), FString("High-risk ability that damages station integrity"));
    GameplayTags.Ability_Risk_Medium = Manager.AddNativeGameplayTag(FName("Ability.Risk.Medium"), FString("Medium-risk ability with minor integrity cost"));
    GameplayTags.Ability_Risk_Low = Manager.AddNativeGameplayTag(FName("Ability.Risk.Low"), FString("Low-risk ability with no integrity cost"));

    // Interactable Tags
    GameplayTags.Interactable_Type_Vent = Manager.AddNativeGameplayTag(FName("Interactable.Type.Vent"), FString("Vent interactable object"));
    GameplayTags.Interactable_Type_Valve = Manager.AddNativeGameplayTag(FName("Interactable.Type.Valve"), FString("Valve interactable object"));
    GameplayTags.Interactable_State_Ready = Manager.AddNativeGameplayTag(FName("Interactable.State.Ready"), FString("Interactable is ready to use"));
    GameplayTags.Interactable_State_Cooldown = Manager.AddNativeGameplayTag(FName("Interactable.State.Cooldown"), FString("Interactable is on cooldown"));

    // NEW Action System Tags - All treated as abilities
    // Core Combat Abilities
    GameplayTags.Action_Ability_Dash = Manager.AddNativeGameplayTag(FName("Action.Ability.Dash"), FString("Dash - movement ability"));
    GameplayTags.Action_Ability_Block = Manager.AddNativeGameplayTag(FName("Action.Ability.Block"), FString("Block - defensive ability"));
    GameplayTags.Action_Ability_BasicAttack = Manager.AddNativeGameplayTag(FName("Action.Ability.BasicAttack"), FString("Basic Attack - jab"));
    GameplayTags.Action_Ability_HeavyAttack = Manager.AddNativeGameplayTag(FName("Action.Ability.HeavyAttack"), FString("Heavy Attack - knockback"));
    
    // Low Risk Abilities
    GameplayTags.Action_Ability_KineticPulse = Manager.AddNativeGameplayTag(FName("Action.Ability.KineticPulse"), FString("Kinetic Pulse - force push"));
    GameplayTags.Action_Ability_DebrisPull = Manager.AddNativeGameplayTag(FName("Action.Ability.DebrisPull"), FString("Debris Pull - magnetic pull"));
    GameplayTags.Action_Ability_CoolantSpray = Manager.AddNativeGameplayTag(FName("Action.Ability.CoolantSpray"), FString("Coolant Spray - area hazard"));
    GameplayTags.Action_Ability_SystemHack = Manager.AddNativeGameplayTag(FName("Action.Ability.SystemHack"), FString("System Hack - remote interaction"));
    
    // Medium Risk Abilities
    GameplayTags.Action_Ability_FloorDestabilizer = Manager.AddNativeGameplayTag(FName("Action.Ability.FloorDestabilizer"), FString("Floor Destabilizer - medium risk"));
    GameplayTags.Action_Ability_ImpactGauntlet = Manager.AddNativeGameplayTag(FName("Action.Ability.ImpactGauntlet"), FString("Impact Gauntlet - charged punch"));
    GameplayTags.Action_Ability_LocalizedEMP = Manager.AddNativeGameplayTag(FName("Action.Ability.LocalizedEMP"), FString("Localized EMP - disable tech"));
    
    // High Risk Abilities
    GameplayTags.Action_Ability_SeismicStamp = Manager.AddNativeGameplayTag(FName("Action.Ability.SeismicStamp"), FString("Seismic Stamp - ground smash"));
    GameplayTags.Action_Ability_GravityAnchor = Manager.AddNativeGameplayTag(FName("Action.Ability.GravityAnchor"), FString("Gravity Anchor - object launcher"));
    GameplayTags.Action_Ability_AirlockBreach = Manager.AddNativeGameplayTag(FName("Action.Ability.AirlockBreach"), FString("Airlock Breach - decompression"));

    // Vulnerability Tier Tags
    GameplayTags.Vulnerability_Tier_Stunned = Manager.AddNativeGameplayTag(FName("Vulnerability.Tier.Stunned"), FString("Yellow tier - 2x damage"));
    GameplayTags.Vulnerability_Tier_Crippled = Manager.AddNativeGameplayTag(FName("Vulnerability.Tier.Crippled"), FString("Orange tier - 4x damage"));
    GameplayTags.Vulnerability_Tier_Exposed = Manager.AddNativeGameplayTag(FName("Vulnerability.Tier.Exposed"), FString("Red tier - 8x damage"));
}