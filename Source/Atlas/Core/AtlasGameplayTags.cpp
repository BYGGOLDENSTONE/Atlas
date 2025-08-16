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
    GameplayTags.Attack_Type_Soul = Manager.AddNativeGameplayTag(FName("Attack.Type.Soul"), FString("Soul attack - unblockable and unparryable"));
    GameplayTags.Attack_Type_Catch = Manager.AddNativeGameplayTag(FName("Attack.Type.Catch"), FString("Catch special - anti-kiting attack"));

    // Attack Property Tags
    GameplayTags.Attack_Property_Blockable = Manager.AddNativeGameplayTag(FName("Attack.Property.Blockable"), FString("Attack can be blocked"));
    GameplayTags.Attack_Property_Parryable = Manager.AddNativeGameplayTag(FName("Attack.Property.Parryable"), FString("Attack can be parried"));
    GameplayTags.Attack_Property_Unblockable = Manager.AddNativeGameplayTag(FName("Attack.Property.Unblockable"), FString("Attack cannot be blocked"));
    GameplayTags.Attack_Property_Unparryable = Manager.AddNativeGameplayTag(FName("Attack.Property.Unparryable"), FString("Attack cannot be parried"));

    // Combat State Tags
    GameplayTags.Combat_State_Attacking = Manager.AddNativeGameplayTag(FName("Combat.State.Attacking"), FString("Character is currently attacking"));
    GameplayTags.Combat_State_Blocking = Manager.AddNativeGameplayTag(FName("Combat.State.Blocking"), FString("Character is blocking"));
    GameplayTags.Combat_State_Parrying = Manager.AddNativeGameplayTag(FName("Combat.State.Parrying"), FString("Character is parrying"));
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

    // Interactable Tags
    GameplayTags.Interactable_Type_Vent = Manager.AddNativeGameplayTag(FName("Interactable.Type.Vent"), FString("Vent interactable object"));
    GameplayTags.Interactable_Type_Valve = Manager.AddNativeGameplayTag(FName("Interactable.Type.Valve"), FString("Valve interactable object"));
    GameplayTags.Interactable_State_Ready = Manager.AddNativeGameplayTag(FName("Interactable.State.Ready"), FString("Interactable is ready to use"));
    GameplayTags.Interactable_State_Cooldown = Manager.AddNativeGameplayTag(FName("Interactable.State.Cooldown"), FString("Interactable is on cooldown"));
}