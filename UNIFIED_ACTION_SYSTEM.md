# Unified Action System - COMPLETED

## Overview
A fully implemented data-driven system where ALL abilities can be assigned to any of 5 input slots. No distinction between "core" and "special" actions - complete player freedom.

## Architecture

### Core Components

#### 1. UniversalAction (Single Implementation)
- One class handles ALL action types through data configuration
- Behavior determined entirely by ActionDataAsset
- No need for separate action classes

#### 2. ActionDataAsset (Configuration)
- Context-sensitive fields that show/hide based on ActionType
- Contains all parameters: damage, cooldown, range, IntegrityCost, etc.
- IntegrityCost is fully configurable - YOU decide the balance

#### 3. ActionManagerComponent
```cpp
class UActionManagerComponent : public UActorComponent
{
    // 5 Action Slots
    TMap<EActionSlot, UBaseAction*> ActionSlots;
    
    // Slot definitions
    enum EActionSlot {
        Slot_LMB,    // Left Mouse Button
        Slot_RMB,    // Right Mouse Button
        Slot_E,      // E Key
        Slot_R,      // R Key
        Slot_Space   // Spacebar
    }
    
    // Methods
    - ExecuteAction(EActionSlot Slot)
    - AssignActionToSlot(UBaseAction* Action, EActionSlot Slot)
    - SwapActions(EActionSlot Slot1, EActionSlot Slot2)
    - GetAvailableActions()
}
```

## Current Implementation (COMPLETED)

### What's Done
✅ UniversalAction class handling all action types
✅ ActionManagerComponent with 5 configurable slots
✅ ActionDataAsset with context-sensitive fields
✅ All 15 abilities implemented and working
✅ Console commands for runtime configuration
✅ Removed all legacy action classes
✅ Cleaned up PlayerCharacter input handling

## Example Configurations

### Example Loadout
```
Slot1 (LMB): Any ability
Slot2 (RMB): Any ability
Slot3 (E): Any ability
Slot4 (R): Any ability
Slot5 (Space): Any ability
```

Players can assign ANY of the 15 abilities to ANY slot. Examples:
- 5 different attacks for aggressive play
- Mix of defense and utility for tactical play
- All movement abilities for mobility focus

## Benefits
1. **Player Choice**: Complete control over control scheme
2. **Experimentation**: Easy to try different combinations
3. **Accessibility**: Players can adapt controls to preference
4. **Future Proof**: Easy to add new actions
5. **Modular**: Each action is self-contained

## Available Abilities (15 Total)

All abilities start with IntegrityCost = 0. Adjust in DataAssets for balance:

- BasicAttack, HeavyAttack, Block
- Dash, FocusMode
- KineticPulse, DebrisPull, CoolantSpray, SystemHack
- FloorDestabilizer, ImpactGauntlet, LocalizedEMP
- SeismicStamp, GravityAnchor, AirlockBreach

## Integrity Cost System
Each ability's IntegrityCost is fully configurable in its DataAsset:
- Set to 0 for no station damage
- Increase value for risk/reward gameplay
- Create different game modes by adjusting costs
- Complete control over balance through data

## Testing Plan
1. Create debug commands to swap actions
2. Console commands for action assignment
3. Runtime action switching for testing
4. Performance monitoring for action execution

## Future Enhancements
- Combo system across different actions
- Action upgrades/evolution
- Conditional actions based on state
- Action synergies (bonus for specific combinations)
- Quick-swap action sets for different scenarios