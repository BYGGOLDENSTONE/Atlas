# Unified Action System Design

## Overview
A flexible system where ALL player actions (attacks, abilities, movement) can be assigned to any of 5 input slots, allowing complete combat customization.

## Architecture

### Core Components

#### 1. BaseAction (Base Class)
```cpp
class UBaseAction : public UActorComponent
{
    // Common properties for all actions
    - ActionName
    - Cooldown
    - ExecutionDuration
    - IntegrityCost (0 for low risk)
    - RequiredTags/BlockedTags
    - State (Ready/Executing/Cooldown)
    
    // Virtual methods
    - CanExecute()
    - Execute()
    - Interrupt()
    - GetActionType()
}
```

#### 2. Action Types (Inherit from BaseAction)
- **CombatAction**: Basic Attack, Heavy Attack, Combos
- **DefensiveAction**: Block, Parry (future)
- **MovementAction**: Dash, Dodge (future)
- **AbilityAction**: Kinetic Pulse, Debris Pull, Coolant Spray, System Hack

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

## Implementation Plan

### Phase 1: Create Base System
1. Create `UBaseAction` class
2. Create `UActionManagerComponent`
3. Define action slot enum and input mapping

### Phase 2: Migrate Existing Systems
1. **Convert CombatComponent actions**:
   - BasicAttackAction (from AttackLMB)
   - HeavyAttackAction (from HeavyAttack)
   - BlockAction (from Block)

2. **Convert DashComponent**:
   - DashAction (from Dash ability)

3. **Convert Abilities**:
   - Rename AbilityBase to AbilityAction
   - Update all 4 abilities to inherit from BaseAction

### Phase 3: Input System
1. Remove hard-coded input bindings from PlayerCharacter
2. Route all inputs through ActionManagerComponent
3. Create input action mapping for 5 slots

### Phase 4: Action Inventory
1. Create action inventory system
2. UI for action selection (basic debug UI)
3. Save/load action configurations

## Example Configurations

### Default Loadout
- **LMB**: BasicAttackAction
- **RMB**: BlockAction
- **E**: HeavyAttackAction
- **R**: KineticPulseAction
- **Space**: DashAction

### Ability-Focused Loadout
- **LMB**: KineticPulseAction
- **RMB**: DebrisPullAction
- **E**: CoolantSprayAction
- **R**: SystemHackAction
- **Space**: DashAction

### Defensive Loadout
- **LMB**: BlockAction
- **RMB**: BasicAttackAction
- **E**: DashAction
- **R**: KineticPulseAction
- **Space**: CoolantSprayAction

## Benefits
1. **Player Choice**: Complete control over control scheme
2. **Experimentation**: Easy to try different combinations
3. **Accessibility**: Players can adapt controls to preference
4. **Future Proof**: Easy to add new actions
5. **Modular**: Each action is self-contained

## Migration Notes

### From Current System
- **CombatComponent**: Extract attack/block logic into actions
- **DashComponent**: Convert to DashAction
- **AbilityBase**: Rename and adjust to inherit from BaseAction
- **PlayerCharacter**: Remove direct input handling

### Data Assets
- Create `UActionDataAsset` base class
- Each action type has specific data asset
- Centralized configuration

## Risk Tiers Integration
Actions will have IntegrityCost:
- **Low Risk** (0 cost): Basic attacks, dash, low-tier abilities
- **Medium Risk** (5-10 cost): Enhanced attacks, mid-tier abilities  
- **High Risk** (15-25 cost): Ultimate abilities, devastating attacks

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