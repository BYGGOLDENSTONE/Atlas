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
- **Parry System**: Perfect (0.2s) and late (0.1s) windows, counter-vulnerability
- **Soul Attack**: 50 damage unblockable, bypasses station integrity
- **Dash System**: 4-directional, 400 units, 2s cooldown, i-frames
- **Vulnerability System**: 3-tier (Stunned 2x, Crippled 4x, Exposed 8x)
- **Station Integrity**: Dual fail-state mechanic
- **Focus Mode**: Environmental targeting (2000 unit range)
- **Wall Impact**: Collision-based bonus damage
- **Poise System**: 100 max, stagger at 0
- **Data-Driven Design**: All values in ActionDataAssets
- **Reward System Architecture**: RewardDataAsset, SlotManagerComponent, RoomDataAsset

### ✅ Phase 2 Completed (2025-01-21)
- **Reward System**: Full architecture with 5 categories implemented
- **Slot Management**: SlotManagerComponent with equip/enhance/replace logic
- **Run Structure**: RunManagerComponent manages 5-room progression
- **Dynamic Enemy AI**: AIDifficultyComponent scales with player power (slots + 1)
- **Reward Persistence**: SaveManagerSubsystem with full save/load functionality
- **Adaptive AI**: Enemy learns and counters player patterns

### ✅ Phase 3 Complete - Content Framework (2025-01-21)
- **RewardDataAssetFactory**: 25 rewards defined across 5 categories:
  - Defense (5): IronSkin, ReactiveArmor, ShieldBoost, Evasion, DamageReduction
  - Offense (5): Berserker, CriticalStrikes, DoubleStrike, Bleed, ArmorPiercing  
  - Passive Stats (5): HealthBoost, PoiseBoost, StaminaBoost, SpeedBoost, CooldownReduction
  - Passive Abilities (5): Lifesteal, Thorns, Regeneration, CounterStrike, LastStand
  - Interactables (5): ExplosiveBarrel, GravityWell, TurretHack, PowerSurge, EmergencyVent
- **RoomDataAssetFactory**: 5 unique rooms with themed enemies and reward pools:
  - Engineering Bay: Mechanic enemy, Defense/Interactable rewards
  - Combat Arena: Gladiator enemy, Offense/Defense rewards
  - Medical Bay: Medic enemy, Passive Stats/Abilities rewards
  - Cargo Hold: Brute enemy, Offense/Passive Stats rewards
  - Bridge: Captain enemy, all reward categories
- **UI Widget Framework**: Complete C++ base classes for UI systems:
  - RewardSelectionWidget: Handles reward choice presentation
  - SlotManagerWidget: Manages 6 reward slots with drag-drop
  - RunProgressWidget: Displays room progression and health/integrity
- **Console Commands**: 40+ commands registered in GameMode for testing all systems
- **Content Guide**: PHASE_3_CONTENT_GUIDE.md provides Blueprint creation instructions
- **Status**: Code compiled and console commands working, ready for Blueprint creation

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
| Soul Attack | 50 damage, unblockable | ✅ Correct |
| Block | 40% reduction | ✅ Correct |
| Parry | 100% (perfect), 50% (late) | ✅ Correct |
| Vulnerability | 2x/4x/8x multiplier | ✅ Correct |
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

# Phase 3 Testing (NEW)
Atlas.Phase3.ListRewards         # List all 25 rewards
Atlas.Phase3.GiveReward [name]   # Give specific reward
Atlas.Phase3.ShowSlots           # Show equipped rewards
Atlas.Phase3.ListRooms           # List all 5 rooms
Atlas.Phase3.StartRun            # Begin new run
Atlas.Phase3.RunTests            # Validate Phase 3
Atlas.Phase3.QuickTest [type]    # Quick test features
```

## Current Phase: Phase 4
**Environmental Systems & Interactables**
1. Implement station integrity visual feedback (hull breaches, sparks, alarms)
2. Create environmental hazard system (electrical surges, toxic leaks, gravity failures)
3. Implement 5 interactable types (valves, gravity wells, turrets, vents, panels)
4. Add destructible environment elements
5. Create station degradation effects over time
6. Implement emergency events (power failures, lockdowns)
7. Add ambient station sounds and atmosphere

## Recently Completed 

### Phase 3 (2025-01-21)
- **Complete C++ Implementation**: All Phase 3 systems coded and compiled
- **UI Widget Framework**: RewardSelectionWidget, SlotManagerWidget, RunProgressWidget
- **Factory Classes**: RewardDataAssetFactory and RoomDataAssetFactory with all definitions
- **Console Commands**: 40+ commands for testing all Phase 3 features
- **Documentation**: Comprehensive guides for content creation

### Phase 2 (2025-01-21)
- **RunManagerComponent**: Complete room progression system
- **RewardSelectionComponent**: Reward UI management and selection flow
- **SaveManagerSubsystem**: Full persistence system with validation
- **AIDifficultyComponent**: Adaptive AI with player pattern analysis
- **AtlasSaveGame**: Comprehensive save data structure

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
│   ├── ActionManagerComponent
│   ├── SlotManagerComponent     # Reward slot management
│   ├── RunManagerComponent      # Room progression
│   ├── RewardSelectionComponent # Reward UI logic
│   └── AIDifficultyComponent    # Enemy scaling
├── Core/            # Game framework classes
│   ├── AtlasSaveGame            # Save data structure
│   └── SaveManagerSubsystem     # Persistence system
├── Data/            # DataAssets
│   ├── RewardDataAsset          # Reward definitions
│   ├── RoomDataAsset            # Room configurations
│   ├── RewardDataAssetFactory   # NEW: Phase 3 reward definitions
│   └── RoomDataAssetFactory     # NEW: Phase 3 room definitions
├── Debug/           # Console commands
│   └── Phase3ConsoleCommands    # NEW: Phase 3 testing commands
├── UI/              # Widget base classes
│   ├── RewardSelectionWidget    # NEW: Reward choice UI
│   ├── SlotManagerWidget        # NEW: Slot management UI
│   └── RunProgressWidget        # NEW: Run progress UI
└── Interfaces/      # Core interfaces
```

## Important Notes
- `bCanBeInterrupted = false` required for attack DataAssets
- Animation notifies control combat timing (not timers)
- Function map routing for O(1) action execution
- All abilities equal, any slot assignment allowed
- Enemy Power = Player Equipped Slots + 1 (core scaling rule)
- Rewards persist between runs via SaveManagerSubsystem
- AI adapts to player patterns after 10+ actions

## Documentation
- `GDD.txt` - Complete game design document
- `IMPLEMENTATION_ROADMAP.md` - Phased development plan with completion status
- `PHASE_3_CONTENT_GUIDE.md` - Blueprint/DataAsset creation guide for Phase 3 content
- `PHASE_4_IMPLEMENTATION_PLAN.md` - Current phase detailed implementation plan
- `CLAUDE.md` - This file, current project status and quick reference