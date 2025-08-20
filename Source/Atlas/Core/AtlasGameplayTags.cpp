#include "AtlasGameplayTags.h"
#include "GameplayTagsManager.h"

FAtlasGameplayTags FAtlasGameplayTags::GameplayTags;

void FAtlasGameplayTags::InitializeNativeTags()
{
    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

    // ===== ACTIONS (15 abilities) =====
    GameplayTags.Action_BasicAttack = Manager.AddNativeGameplayTag(FName("Action.BasicAttack"), FString("Basic Attack"));
    GameplayTags.Action_HeavyAttack = Manager.AddNativeGameplayTag(FName("Action.HeavyAttack"), FString("Heavy Attack"));
    GameplayTags.Action_Block = Manager.AddNativeGameplayTag(FName("Action.Block"), FString("Block"));
    GameplayTags.Action_Dash = Manager.AddNativeGameplayTag(FName("Action.Dash"), FString("Dash"));
    GameplayTags.Action_FocusMode = Manager.AddNativeGameplayTag(FName("Action.FocusMode"), FString("Focus Mode"));
    GameplayTags.Action_KineticPulse = Manager.AddNativeGameplayTag(FName("Action.KineticPulse"), FString("Kinetic Pulse"));
    GameplayTags.Action_DebrisPull = Manager.AddNativeGameplayTag(FName("Action.DebrisPull"), FString("Debris Pull"));
    GameplayTags.Action_CoolantSpray = Manager.AddNativeGameplayTag(FName("Action.CoolantSpray"), FString("Coolant Spray"));
    GameplayTags.Action_SystemHack = Manager.AddNativeGameplayTag(FName("Action.SystemHack"), FString("System Hack"));
    GameplayTags.Action_FloorDestabilizer = Manager.AddNativeGameplayTag(FName("Action.FloorDestabilizer"), FString("Floor Destabilizer"));
    GameplayTags.Action_ImpactGauntlet = Manager.AddNativeGameplayTag(FName("Action.ImpactGauntlet"), FString("Impact Gauntlet"));
    GameplayTags.Action_LocalizedEMP = Manager.AddNativeGameplayTag(FName("Action.LocalizedEMP"), FString("Localized EMP"));
    GameplayTags.Action_SeismicStamp = Manager.AddNativeGameplayTag(FName("Action.SeismicStamp"), FString("Seismic Stamp"));
    GameplayTags.Action_GravityAnchor = Manager.AddNativeGameplayTag(FName("Action.GravityAnchor"), FString("Gravity Anchor"));
    GameplayTags.Action_AirlockBreach = Manager.AddNativeGameplayTag(FName("Action.AirlockBreach"), FString("Airlock Breach"));

    // ===== COMBAT STATES =====
    GameplayTags.State_Attacking = Manager.AddNativeGameplayTag(FName("State.Attacking"), FString("Attacking"));
    GameplayTags.State_Blocking = Manager.AddNativeGameplayTag(FName("State.Blocking"), FString("Blocking"));
    GameplayTags.State_Dashing = Manager.AddNativeGameplayTag(FName("State.Dashing"), FString("Dashing"));
    GameplayTags.State_Vulnerable = Manager.AddNativeGameplayTag(FName("State.Vulnerable"), FString("Vulnerable"));
    GameplayTags.State_Staggered = Manager.AddNativeGameplayTag(FName("State.Staggered"), FString("Staggered"));
    GameplayTags.State_Dead = Manager.AddNativeGameplayTag(FName("State.Dead"), FString("Dead"));
    GameplayTags.State_IFrames = Manager.AddNativeGameplayTag(FName("State.IFrames"), FString("Invincible"));

    // ===== ATTACK TYPES (for AI/legacy) =====
    GameplayTags.Attack_Jab = Manager.AddNativeGameplayTag(FName("Attack.Jab"), FString("Jab Attack"));
    GameplayTags.Attack_Heavy = Manager.AddNativeGameplayTag(FName("Attack.Heavy"), FString("Heavy Attack"));
    GameplayTags.Attack_Unblockable = Manager.AddNativeGameplayTag(FName("Attack.Unblockable"), FString("Unblockable"));

    // ===== ANIMATION =====
    GameplayTags.Anim_HitLight = Manager.AddNativeGameplayTag(FName("Anim.HitLight"), FString("Light Hit Reaction"));
    GameplayTags.Anim_HitHeavy = Manager.AddNativeGameplayTag(FName("Anim.HitHeavy"), FString("Heavy Hit Reaction"));
    GameplayTags.Anim_Stagger = Manager.AddNativeGameplayTag(FName("Anim.Stagger"), FString("Stagger Animation"));
    GameplayTags.Anim_BlockSuccess = Manager.AddNativeGameplayTag(FName("Anim.BlockSuccess"), FString("Block Success"));
    GameplayTags.Anim_BlockImpact = Manager.AddNativeGameplayTag(FName("Anim.BlockImpact"), FString("Block Impact"));
    GameplayTags.Anim_Death = Manager.AddNativeGameplayTag(FName("Anim.Death"), FString("Death Animation"));

    // ===== STATION RISK =====
    GameplayTags.Risk_High = Manager.AddNativeGameplayTag(FName("Risk.High"), FString("High Risk"));
    GameplayTags.Risk_Medium = Manager.AddNativeGameplayTag(FName("Risk.Medium"), FString("Medium Risk"));
    GameplayTags.Risk_Low = Manager.AddNativeGameplayTag(FName("Risk.Low"), FString("Low Risk"));
}