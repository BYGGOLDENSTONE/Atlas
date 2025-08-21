#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Atlas Gameplay Tags - Organized for easy reference
 * 
 * TAG STRUCTURE:
 * - Action.*       = Player abilities (15 total, assignable to any slot)
 * - State.*        = Combat/character states  
 * - Anim.*         = Animation triggers
 * - Risk.*         = Station integrity risk levels
 * - Interactable.* = Environmental interaction tags
 */
struct ATLAS_API FAtlasGameplayTags
{
public:
    static const FAtlasGameplayTags& Get() { return GameplayTags; }
    static void InitializeNativeTags();

    // ========================================
    // PLAYER ACTIONS (15 abilities)
    // All assignable to slots except FocusMode (Q key only)
    // ========================================
    
    // Core Combat (5)
    FGameplayTag Action_BasicAttack;      // Light attack - low damage, fast
    FGameplayTag Action_HeavyAttack;      // Heavy attack - high damage, slow  
    FGameplayTag Action_Block;            // Defensive stance - reduces damage
    FGameplayTag Action_Parry;            // Perfect timing defense - counters attacks
    FGameplayTag Action_SoulAttack;       // Unblockable special - 50 damage, no station damage
    
    // Movement (1)
    FGameplayTag Action_Dash;             // Quick dodge with i-frames
    
    // Special System (1) 
    FGameplayTag Action_FocusMode;        // Target lock system (Q key only)
    
    // Station Damage Abilities (10)
    FGameplayTag Action_KineticPulse;     // AoE knockback
    FGameplayTag Action_DebrisPull;       // Ranged projectile
    FGameplayTag Action_CoolantSpray;     // DoT area denial
    FGameplayTag Action_SystemHack;       // Enemy debuff
    FGameplayTag Action_FloorDestabilizer;// Ground hazard
    FGameplayTag Action_ImpactGauntlet;   // Melee burst
    FGameplayTag Action_LocalizedEMP;     // Electronics disable
    FGameplayTag Action_SeismicStamp;     // Shockwave
    FGameplayTag Action_GravityAnchor;    // Pull/slow field
    FGameplayTag Action_AirlockBreach;    // Ultimate vacuum

    // ========================================
    // CHARACTER STATES
    // Applied during combat for state tracking
    // ========================================
    FGameplayTag State_Attacking;         // Currently in attack animation
    FGameplayTag State_Blocking;          // Active block stance
    FGameplayTag State_Dashing;           // Mid-dash movement
    FGameplayTag State_Vulnerable;        // Taking extra damage
    FGameplayTag State_Staggered;         // Poise broken, stunned
    FGameplayTag State_Dead;              // Character defeated
    FGameplayTag State_IFrames;           // Invincibility frames
    
    // Combat substates
    FGameplayTag Combat_State_SoulAttacking; // Performing soul attack

    // ========================================
    // ANIMATION EVENTS
    // ========================================
    FGameplayTag Anim_HitLight;           // Play light hit reaction
    FGameplayTag Anim_HitHeavy;           // Play heavy hit reaction
    FGameplayTag Anim_Stagger;            // Play stagger animation
    FGameplayTag Anim_BlockSuccess;       // Successful block visual
    FGameplayTag Anim_BlockImpact;        // Block impact effect
    FGameplayTag Anim_Death;              // Death animation

    // ========================================
    // STATION INTEGRITY RISK LEVELS
    // ========================================
    FGameplayTag Risk_High;               // Major station damage
    FGameplayTag Risk_Medium;             // Moderate station damage
    FGameplayTag Risk_Low;                // Minor station damage

    // ========================================
    // INTERACTABLES
    // ========================================
    FGameplayTag Interactable_State_Ready;    // Can be interacted with
    FGameplayTag Interactable_State_Cooldown; // On cooldown
    FGameplayTag Interactable_Type_Valve;     // Valve interaction
    FGameplayTag Interactable_Type_Vent;      // Vent interaction

private:
    static FAtlasGameplayTags GameplayTags;
};