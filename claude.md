# Atlas - Quick Reference

**Engine**: UE 5.5 | **Genre**: 1v1 Roguelite Dueler | **Updated**: 2025-01-22

## Overview
Single-player roguelite dueling game. 5 rooms per run, persistent rewards, dual fail-states (health/integrity).

## Current Status
✅ **Phase 1-4 Complete**: Core systems, rewards, enemies, environment  
✅ **GameplayTag Migration Complete**: All tags updated to `Action.Combat.*` structure  
🔧 **Next**: Blueprint implementation in Unreal Editor

## Key Systems Implemented
- **Combat**: Animation-driven with parry/vulnerability mechanics
- **Rewards**: 25 rewards across 5 categories with persistence
- **Rooms**: 5 unique themed rooms with specialized enemies
- **Environment**: Hazards, degradation, emergency events
- **AI**: Adaptive difficulty scaling with player power

## Combat Values
- Basic Attack: 5 damage
- Heavy Attack: 15 damage + 500 knockback
- Soul Attack: 50 unblockable damage
- Block: 40% reduction
- Parry: Perfect (100%, 0.2s) / Late (50%, 0.1s)
- Vulnerability: 2x/4x/8x multipliers
- Dash: 400 units, 2s cooldown

## Important Console Commands
```
# Combat Testing
Atlas_AssignAction [Slot] [ActionTag]
Atlas_ShowSlots

# Phase Testing
Atlas.Phase3.StartRun
Atlas.Phase3.GiveReward [name]
Atlas.Phase4.TestAll
Atlas.Phase4.SpawnHazard [Type]
```

## Architecture
```
GameCharacterBase
├── ActionManagerComponent (combat/actions)
├── HealthComponent (health/poise)  
├── VulnerabilityComponent (damage mult)
├── SlotManagerComponent (rewards)
└── StationIntegrityComponent (station)
```

## Technical Rules
- NO GAS - Custom components only
- Data-Driven via DataAssets
- Animation-driven combat timing
- GameplayTags for all states
- `bCanBeInterrupted = false` for attacks

## File Locations
- **Actions**: `/Source/Atlas/Actions/`
- **Components**: `/Source/Atlas/Components/`
- **Data Assets**: `/Source/Atlas/Data/`
- **Console Cmds**: `/Source/Atlas/Debug/`

## Key Documentation
- `GDD.txt` - Full game design
- `IMPLEMENTATION_ROADMAP.md` - Development phases
- `BLUEPRINT_IMPLEMENTATION_ROADMAP.md` - Blueprint guide
- `ATLAS_CONSOLE_COMMANDS.txt` - All 100+ commands
- `GAMEPLAYTAG_MIGRATION_STATUS.md` - Tag migration (COMPLETE)

## Next Steps
1. Create Blueprint assets in Editor
2. Set up test map and player character
3. Implement UI widgets from C++ base classes
4. Create enemy blueprints for 5 room types