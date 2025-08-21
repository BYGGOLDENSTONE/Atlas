# Atlas - Current Project Status

**Engine**: UE 5.5 | **Genre**: 1v1 Roguelite Dueler | **Date**: 2025-01-21

## Game Overview
Atlas is a single-player, run-based dueling game on a decaying space station. Players fight through 5 randomized rooms, defeating unique enemies and choosing rewards to build their character. Runs end when health or station integrity reaches zero. Equipped rewards persist between runs for meta-progression.

## Core Gameplay Loop
1. Start run at Level 1 in random room
2. Fight room's unique enemy
3. Choose 1 of 2 rewards from room's themed pool
4. Equip reward (replace if slots full)
5. Progress to next level
6. Complete 5 rooms or fail (death/station destroyed)
7. Equipped rewards carry to next run

## Current Implementation Status

### ✅ Completed Systems
- **Unified ActionManagerComponent**: All combat/actions in single component
- **4 Core Interfaces**: ICombatInterface, IHealthInterface, IActionInterface, IInteractable
- **Animation-Driven Combat**: Attack timing via notifies
- **Basic Combat**: Attack, Block, Heavy Attack functional
- **Dash System**: 4-directional, 400 units, 2s cooldown, i-frames
- **Vulnerability System**: 3-tier (Stunned 2x, Crippled 4x, Exposed 8x)
- **Station Integrity**: Dual fail-state mechanic
- **Focus Mode**: Environmental targeting (2000 unit range)
- **Wall Impact**: Collision-based bonus damage
- **Poise System**: 100 max, stagger at 0
- **Data-Driven Design**: All values in ActionDataAssets

### ⚠️ Needs Implementation (Per GDD)
- **Parry System**: Was removed, GDD requires re-implementation
- **Soul Attack**: Unblockable, no station damage
- **Reward System**: 5 categories (Defense, Offense, Passives, Abilities, Interactables)
- **Slot Management**: Limited slots, enhancement stacking
- **Run Structure**: 5 rooms, randomized order
- **Dynamic Enemy AI**: Scales with player power (slots + 1)
- **Reward Persistence**: Cross-run progression
- **Enhanced Interactables**: Exploding valves, vents, etc.

## Architecture Overview
```
GameCharacterBase
├── ActionManagerComponent (all combat/actions)
├── HealthComponent (health/poise)
├── VulnerabilityComponent (damage multipliers)
├── FocusModeComponent (targeting)
└── StationIntegrityComponent (station health)
```

## Combat Values (Target per GDD)
| System | Current | GDD Target |
|--------|---------|------------|
| Basic Attack | 5 damage | ✅ Correct |
| Heavy Attack | 15 damage, 500 knockback | ✅ Correct |
| Soul Attack | Not implemented | 50 damage, unblockable |
| Block | 40% reduction | ✅ Correct |
| Parry | Removed | Needs re-implementation |
| Vulnerability | 8x multiplier | ✅ Correct |
| Poise | 100 max, 2s stagger | ✅ Correct |
| Dash | 400 units, 2s cooldown | ✅ Correct |

## Console Commands
```
# Action Management
Atlas_AssignAction [Slot] [ActionTag]
Atlas_ClearSlot [Slot]
Atlas_SwapSlots [Slot1] [Slot2]
Atlas_ListActions
Atlas_ShowSlots

# Debug
Atlas.DamageIntegrity [amount]
Atlas.SetIntegrityPercent [percent]
Atlas.ShowIntegrityStatus
Atlas.ToggleFocusMode
```

## Next Implementation Phase
**Phase 1: Core Combat Polish & Parry System**
1. Re-implement parry mechanic (required by GDD)
2. Create Soul Attack ability
3. Tune combat values to match GDD
4. Begin reward system architecture

## Technical Rules
- **NO GAS**: Custom component system only
- **Data-Driven**: All values in DataAssets
- **Interface-Based**: Use 4 core interfaces
- **Animation-Driven**: Combat timing via notifies
- **Tag-Based**: GameplayTags for all conditions
- **No Circular Dependencies**: Clean architecture

## File Structure
```
Source/Atlas/
├── Actions/          # BaseAction, UniversalAction
├── Animation/        # Combat notifies
├── Characters/       # Player, Enemy, Base
├── Components/       # Core gameplay components
├── Core/            # Game framework classes
├── Data/            # DataAssets
├── Debug/           # Console commands
└── Interfaces/      # Core interfaces
```

## Important Notes
- `bCanBeInterrupted = false` required for attack DataAssets
- Animation notifies control combat timing (not timers)
- Function map routing for O(1) action execution
- All abilities equal, any slot assignment allowed

## Documentation
- `GDD.txt` - Complete game design document
- `IMPLEMENTATION_ROADMAP.md` - Phased development plan
- `CLAUDE.md` - This file, current status reference