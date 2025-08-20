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
- `UniversalAction`: Handles all action types via data
- `ActionManagerComponent`: Manages 5 action slots
- `ActionDataAsset`: Context-sensitive configuration
- `HealthComponent`: Health + poise management
- `CombatComponent`: Legacy combat (being phased out)
- `DamageCalculator`: Centralized damage math
- `StationIntegrityComponent`: Station health tracking

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
- Data-driven via DataAssets
- Tag-based rules (Action.Ability.*)
- Component interfaces (IInteractable, ICombatant)
- No parry or camera lock systems

## Next Tasks
- Create all 15 ActionDataAssets in editor
- Test ability assignments and balance
- Phase out remaining CombatComponent logic
- Implement P17-18 enemy archetypes
- P21-22 Wife's Arm passive system

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
- `UNIVERSAL_ACTION_GUIDE.md` - Action system usage
- `REQUIRED_DATA_ASSETS.md` - DataAsset requirements
- `COMPONENT_ARCHITECTURE.md` - System dependencies
- `changelog.md` - Development history