# Atlas - Combat System Development

## Project Overview
**Engine**: UE 5.5 | **Type**: Single-player roguelite | **Genre**: 1v1 duels on decaying space station

## Core Concept
Strategic combat where players manage both health and station integrity. Enemies only take significant damage when vulnerable. Powerful abilities damage the station (dual-fail state).

## Unified Action System ✅
**COMPLETED**: Single `UniversalAction` class with data-driven configuration via `ActionDataAsset`
- **5 Customizable Slots**: Any ability → any slot (LMB, RMB, E, R, Space)
- **15 Abilities Total**: All equal, no "core" vs "special" distinction
- **Flexible Balance**: IntegrityCost configurable per ability (YOU control risk/reward)

### Console Commands
```bash
Atlas_AssignAction [Slot] [ActionTag]  # Assign ability to slot
Atlas_ClearSlot [Slot]                 # Clear a slot
Atlas_SwapSlots [Slot1] [Slot2]       # Swap two slots
Atlas_ListActions                      # List all abilities
Atlas_ShowSlots                        # Show current setup
Atlas_ResetSlots                       # Reset to defaults
```

## Combat Values
- **Basic Attack**: 5 damage, 100 knockback, 20 poise
- **Heavy Attack**: 15 damage, 500 knockback + ragdoll
- **Block**: 40% damage reduction
- **Vulnerability**: 8x damage, 1s duration
- **Poise**: 100 max, breaks at 0, 2s stagger
- **Dash**: 400 units, i-frames, 2s cooldown
- **Focus**: 2000 unit range, screen-space targeting

## Implemented Systems ✅
- **P0-P12**: Core combat, animations, vulnerability tiers, station integrity
- **P14-P16**: All 15 abilities (configurable integrity costs)
- **Unified Actions**: Data-driven system replacing old components
- **Knockback**: Direction-based with wall/floor impact detection

## Key Components
- `UniversalAction`: Handles all action types via function map routing (O(1) lookup)
- `ActionManagerComponent`: Manages 5 action slots
- `ActionDataAsset`: Fully data-driven config (timing, behavior, interrupts)
- `HealthComponent`: Health + poise management
- `CombatComponent`: Legacy combat (migrate remaining to UniversalAction)
- `DamageCalculator`: Centralized damage math
- `StationIntegrityComponent`: Station health tracking

## Architecture (Latest Refactor)
- **Function Map Routing**: Replaced if/else chains with efficient map lookup
- **Interface Layer**: ICombatInterface, IHealthInterface, IActionInterface
- **No Circular Dependencies**: Components communicate via interfaces
- **Data-Driven Everything**: All timings, durations, behaviors in DataAssets
- **Unified Timer**: Single ActionTimer replaces multiple timer variables

## Required DataAssets (20 Total)
### Actions (15) - in Content/Data/Actions/
- BasicAttack, HeavyAttack, Block, Dash, FocusMode
- KineticPulse, DebrisPull, CoolantSpray, SystemHack
- FloorDestabilizer, ImpactGauntlet, LocalizedEMP
- SeismicStamp, GravityAnchor, AirlockBreach

### System (5) - Various locations
- CombatRules, AttackData_Jab/Heavy (AI), StationIntegrity, DebugCommands

## Development Rules
- NO GAS - custom component system
- Data-driven via DataAssets (all magic numbers removed)
- Interface-based communication (ICombatInterface, IHealthInterface, IActionInterface)
- No parry or camera lock systems
- Use interfaces over direct component access

## Gameplay Tag Structure
- **Action.*** = Player abilities (15 total, each handles its own logic)
- **State.*** = Combat states (attacking, blocking, vulnerable, etc)
- **Anim.*** = Animation triggers (hit reactions, death, etc)
- **Risk.*** = Station integrity risk levels
- **Interactable.*** = Environmental interactions

## CRITICAL ISSUE - Fix Next Session
**🔴 CRASH BUG**: Game crashes when using abilities (LMB/RMB/etc)
- Occurs when abilities are triggered through input system
- Likely related to CombatComponent::StartAttack expecting AttackDataAsset
- Need to verify DataAsset configuration and CombatComponent's AttackDataMap

## Next Tasks
- **PRIORITY**: Fix ability usage crash
  - Check CombatComponent's AttackDataMap configuration
  - Verify ActionDataAssets are properly configured
  - Ensure ProcessHit has valid attack data
- Update all 15 ActionDataAssets with new timing fields in editor
- Test refactored function map routing and interfaces
- Migrate final CombatComponent attack/block logic to UniversalAction
- Implement P17-18 enemy archetypes
- P21-22 Wife's Arm passive system
- Verify all new DataAsset fields work (ActionDuration, MontagePlayRate, etc.)

## Debug Commands
```
Atlas.DamageIntegrity [amount]
Atlas.SetIntegrityPercent [percent]
Atlas.ShowIntegrityStatus
Atlas.ResetIntegrity
Atlas.ToggleFocusMode
Atlas.ShowFocusDebug
```

## Documentation
- `ARCHITECTURE_IMPROVEMENTS.md` - Latest refactoring details and testing checklist
- `changelog.md` - Development history

## New DataAsset Fields (Must Configure)
- `ActionDuration`: How long action takes
- `MontagePlayRate`: Animation speed multiplier
- `AttackWindupTime`: Time before damage
- `AttackRecoveryTime`: Time after damage
- `bCanBeInterrupted`: Can action be cancelled
- `bAutoReleaseOnComplete`: Auto-end when timer expires
- `bIsToggleAction`: Toggle on/off (e.g., Focus Mode)
- `InputBufferWindow`: Action queueing window