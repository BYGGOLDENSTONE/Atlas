# Atlas - Implementation Status

**Engine**: UE 5.5 | **Genre**: 1v1 Roguelite Dueler | **Target**: GDD.txt

## Overview
Single-player roguelite dueling game with 5-room runs, persistent rewards, and dual fail-states (health/integrity).

## ✅ Implemented Features

### Core Combat System
- **Action System**: Unified ActionInstance class with data-driven abilities
- **Animation-Driven Combat**: States managed by animation notifies
- **Damage Application**: Timed with animation frames via AttackNotify
- **Combat Actions**: BasicAttack, HeavyAttack, Block, Dash, Parry
- **State Management**: Attacking, Blocking, Dashing states with proper transitions
- **Hit Detection**: Animation-based collision detection at damage frames

### Room & Run System
- **5-Room Structure**: Bridge, Medical Bay, Engineering Bay, Cargo Hold, Combat Arena
- **Run Manager**: Handles room progression, enemy spawning, completion tracking
- **Equal Randomization**: Fair room selection without level-based bias
- **Auto-Progression**: Automatic advancement on enemy defeat

### Character Components
- **ActionManagerComponent**: 5 action slots, combo system, state tags
- **HealthComponent**: Health, poise, damage calculations
- **VulnerabilityComponent**: Damage multipliers (2x/4x/8x)
- **StationIntegrityComponent**: Secondary fail state system
- **SlotManagerComponent**: Reward equipment system

### Reward System
- **Target: 25 Unique Rewards**: 5 per room type (currently 3 test rewards per room)
- **Slot-Based Equipment**: Head, Body, Arms, Legs, Accessory slots
- **Persistent Upgrades**: Rewards carry between runs
- ✅ **Reward Selection UI**: Pure Slate modal UI after enemy defeat with mouse/console input
- ✅ **Room-Specific Pools**: Each room type offers themed rewards:
  - Bridge → Interactables (3 test rewards)
  - Cargo Hold → Defense (3 test rewards)
  - Medical Bay → Passive Abilities (3 test rewards)
  - Engineering Bay → Passive Stats (3 test rewards)
  - Combat Arena → Offense (3 test rewards)
- ✅ **Selection Blocking**: Room progression blocked until reward selected/cancelled
- ✅ **Test System**: Dynamic reward generation working without DataAssets
- ⏳ **Next Session**: Create proper DataAssets for all 25 rewards (5 per room)

### Environment Systems  
- **Hazards**: Fire, Electric, Toxic, Explosive, Gravity types
- **Destructible Objects**: Multi-stage destruction with debris
- **Interactables**: Valves, vents, terminals with gameplay effects

## ⏳ Not Yet Implemented

### UI System
- Health/Integrity bars
- Action slot indicators  
- Combo window display
- Room progression UI
- ✅ **Reward selection system** - Pure Slate UI for choosing rewards after enemy defeat

### Enemies
- 5 room-specific enemy types (defined but need BP implementation)
- AI behavior trees
- Adaptive difficulty scaling

### Visual Effects
- Combat VFX (hits, blocks, parries)
- Environmental effects
- Ability visuals

### Audio
- Combat sounds
- Environmental audio
- Music system

## Important Console Commands
```
# Run Management
Atlas.StartRun            # Start a new 5-room run
Atlas.CompleteRoom        # Complete current room and trigger reward selection
Atlas.GoToRoom [name]      # Teleport to specific room
Atlas.ShowMap             # Display current run progress

# Reward Selection
Atlas.SelectReward 0      # Select first reward option
Atlas.SelectReward 1      # Select second reward option
Atlas.CancelRewardSelection # Skip reward selection

# Combat Testing
Atlas_AssignAction [Slot] [ActionTag]
Atlas_ShowSlots

# Reward Testing  
Atlas.GiveReward [name]
Atlas.ShowRewards

# Environment Testing
Atlas.SpawnHazard [Type]
Atlas.TestEmergency [Type]
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

## Combat Values (Configured)
- **Basic Attack**: 5 damage, 0.5s duration
- **Heavy Attack**: 15 damage, 500 knockback, 1.0s duration  
- **Soul Attack**: 50 unblockable damage (not yet assigned)
- **Block**: 40% damage reduction
- **Parry Windows**: Perfect (100% reduction, 0.2s) / Late (50%, 0.1s)
- **Vulnerability Multipliers**: 2x/4x/8x based on stacks
- **Dash**: 400 units distance, 0.3s duration, 2s cooldown

## Next Session Plans
1. **Design Final 25 Rewards**: Define 5 unique rewards per room type
2. **Create Reward DataAssets**: Implement all rewards as UE5 DataAssets
3. **Room Reward Pools**: Configure each room's DataAsset with its 5 rewards
4. **Remove Test Rewards**: Replace temporary code with DataAsset references

## Next Steps
1. Create Blueprint assets in Editor
2. Set up test map and player character
3. Implement UI widgets from C++ base classes
4. Create enemy blueprints for 5 room types