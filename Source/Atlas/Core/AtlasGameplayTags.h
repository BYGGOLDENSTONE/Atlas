#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Simplified gameplay tags for Atlas - keep it clean and flat
 */
struct ATLAS_API FAtlasGameplayTags
{
public:
    static const FAtlasGameplayTags& Get() { return GameplayTags; }
    static void InitializeNativeTags();

    // ===== ACTIONS (15 abilities, all equal) =====
    FGameplayTag Action_BasicAttack;
    FGameplayTag Action_HeavyAttack;
    FGameplayTag Action_Block;
    FGameplayTag Action_Dash;
    FGameplayTag Action_FocusMode;
    FGameplayTag Action_KineticPulse;
    FGameplayTag Action_DebrisPull;
    FGameplayTag Action_CoolantSpray;
    FGameplayTag Action_SystemHack;
    FGameplayTag Action_FloorDestabilizer;
    FGameplayTag Action_ImpactGauntlet;
    FGameplayTag Action_LocalizedEMP;
    FGameplayTag Action_SeismicStamp;
    FGameplayTag Action_GravityAnchor;
    FGameplayTag Action_AirlockBreach;

    // ===== COMBAT STATES =====
    FGameplayTag State_Attacking;
    FGameplayTag State_Blocking;
    FGameplayTag State_Dashing;
    FGameplayTag State_Vulnerable;
    FGameplayTag State_Staggered;
    FGameplayTag State_Dead;
    FGameplayTag State_IFrames;

    // ===== ATTACK TYPES (for AI/legacy) =====
    FGameplayTag Attack_Jab;
    FGameplayTag Attack_Heavy;
    FGameplayTag Attack_Unblockable;

    // ===== ANIMATION =====
    FGameplayTag Anim_HitLight;
    FGameplayTag Anim_HitHeavy;
    FGameplayTag Anim_Stagger;
    FGameplayTag Anim_BlockSuccess;
    FGameplayTag Anim_BlockImpact;
    FGameplayTag Anim_Death;

    // ===== STATION RISK =====
    FGameplayTag Risk_High;
    FGameplayTag Risk_Medium;
    FGameplayTag Risk_Low;

private:
    static FAtlasGameplayTags GameplayTags;
};