#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Atlas Gameplay Tags - Simplified
 * 
 * All tags are now defined in Config/DefaultGameplayTags.ini
 * This class remains for potential future native tag needs
 * 
 * Current Tag Structure (from DefaultGameplayTags.ini):
 * - Action.Combat.*        = Combat actions
 * - Action.Slot.*          = Slot assignments
 * - State.Combat.*         = Combat states
 * - State.Status.*         = Status effects
 * - Reward.Category.*      = Reward categories
 * - Reward.Tier.*          = Reward tiers
 * - Room.Type.*            = Room types
 * - Enemy.Type.*           = Enemy types
 * - Hazard.Type.*          = Hazard types
 * - UI.*                   = UI states
 */
struct ATLAS_API FAtlasGameplayTags
{
public:
    static const FAtlasGameplayTags& Get() { return GameplayTags; }
    static void InitializeNativeTags();

    // No native tags currently - all tags come from DefaultGameplayTags.ini
    // If native tags are needed in future, declare them here

private:
    static FAtlasGameplayTags GameplayTags;
};