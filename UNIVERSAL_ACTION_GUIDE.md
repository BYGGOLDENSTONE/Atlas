# Universal Action System Guide

## Overview
The Universal Action System is a unified, data-driven approach for handling ALL player actions in Atlas. Instead of separate classes for each action type, we use a single `UniversalAction` class configured through `ActionDataAsset`.

## Architecture

### Core Components

#### 1. UniversalAction (`Source/Atlas/Actions/UniversalAction.h`)
- Single action class that handles all action types
- Behavior determined by ActionDataAsset configuration
- Supports: Movement, Defense, Melee/Ranged Attacks, Area Effects, Utilities, Special abilities

#### 2. ActionDataAsset (`Source/Atlas/DataAssets/ActionDataAsset.h`)
- Context-sensitive data asset with fields that show/hide based on ActionType
- Contains all configuration for an action (damage, cooldown, range, etc.)
- Lives in `Content/Data/Actions/` folder

#### 3. ActionManagerComponent (`Source/Atlas/Components/ActionManagerComponent.h`)
- Manages 5 customizable action slots (Slot1-5)
- Maps to input buttons: LMB, RMB, E, R, Space
- Handles action execution, cooldowns, and state management

#### 4. BaseAction (`Source/Atlas/Actions/BaseAction.h`)
- Abstract base class for all actions
- Provides common interface: CanActivate, OnActivate, OnTick, OnRelease, OnInterrupted

## Creating New Actions

### Step 1: Create Action Data Asset
1. In Content Browser, navigate to `Content/Data/Actions/`
2. Right-click → Miscellaneous → Data Asset
3. Choose `ActionDataAsset` as the class
4. Name it (e.g., `DA_KineticPulse`)

### Step 2: Configure Data Asset
```
1. Identity Section:
   - ActionTag: Action.Ability.YourActionName
   - ActionType: Choose appropriate type (Movement, Defense, MeleeAttack, etc.)
   - ActionClass: UniversalAction
   - ActionName: Display name
   - Description: What the action does

2. Universal Section:
   - Cooldown: Time between uses
   - IntegrityCost: Station damage cost (0 for low risk)

3. Type-Specific Sections (auto-show based on ActionType):
   - Movement: DashDistance, DashDuration, InvincibilityDuration
   - Defense: DamageReduction, RequiresHoldInput
   - MeleeAttack: Damage, KnockbackForce, PoiseDamage
   - RangedAttack: ProjectileSpeed, ProjectileDamage, Range
   - AreaEffect: Radius, Damage, Duration
   - Utility: InteractionRange, ChargeTime
   - Special: AbilityClass, CustomParameters
```

### Step 3: Register Action Tag
Add to `Source/Atlas/Core/AtlasGameplayTags.cpp`:
```cpp
UE_DEFINE_GAMEPLAY_TAG(Action_Ability_YourActionName, "Action.Ability.YourActionName");
```

## Console Commands

### Action Management
```bash
# Assign action to slot
Atlas_AssignAction Slot1 Action.Ability.BasicAttack
Atlas_AssignAction Slot2 Action.Ability.Block
Atlas_AssignAction Slot3 Action.Ability.HeavyAttack
Atlas_AssignAction Slot4 Action.Ability.KineticPulse
Atlas_AssignAction Slot5 Action.Ability.Dash

# Clear a slot
Atlas_ClearSlot Slot3

# Swap two slots
Atlas_SwapSlots Slot1 Slot2

# List all available actions
Atlas_ListActions

# Show current slot assignments
Atlas_ShowSlots

# Reset to default configuration
Atlas_ResetSlots
```

## Available Actions

### Combat Actions
- `Action.Ability.BasicAttack` - Basic melee attack (5 damage)
- `Action.Ability.HeavyAttack` - Heavy melee attack (15 damage, ragdoll)
- `Action.Ability.Block` - Defensive block (40% damage reduction)

### Movement Actions
- `Action.Ability.Dash` - Quick directional dash with i-frames

### Low Risk Abilities (No Integrity Cost)
- `Action.Ability.KineticPulse` - Short-range force push
- `Action.Ability.DebrisPull` - Magnetic object manipulation
- `Action.Ability.CoolantSpray` - Creates slip hazard area
- `Action.Ability.SystemHack` - Remote interaction with objects

## Example Configurations

### Default Combat Loadout
```
Slot1 (LMB): BasicAttack
Slot2 (RMB): Block
Slot3 (E): HeavyAttack
Slot4 (R): KineticPulse
Slot5 (Space): Dash
```

### Ability-Focused Loadout
```
Slot1 (LMB): KineticPulse
Slot2 (RMB): DebrisPull
Slot3 (E): CoolantSpray
Slot4 (R): SystemHack
Slot5 (Space): Dash
```

### Defensive Loadout
```
Slot1 (LMB): Block
Slot2 (RMB): BasicAttack
Slot3 (E): Dash
Slot4 (R): KineticPulse
Slot5 (Space): CoolantSpray
```

## Implementation Details

### How UniversalAction Works
1. **Configuration**: Each UniversalAction instance is configured by an ActionDataAsset
2. **Tag-Based Behavior**: The action's GameplayTag determines its behavior
3. **Type-Specific Logic**: Based on ActionType, different code paths execute:
   - Movement → ExecuteDash()
   - Defense → ExecuteBlock()
   - MeleeAttack → ExecuteMeleeAttack()
   - Special → ExecuteGenericAbility()

### Action State Flow
```
Ready → CanActivate() → OnActivate() → Executing → OnTick() → OnRelease()/OnInterrupted() → Cooldown → Ready
```

### Adding Custom Behavior
To add unique behavior for a specific action:

1. Check the action tag in UniversalAction::OnActivate()
2. Add custom logic for that tag
3. Example:
```cpp
if (ActionData->ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Action.Ability.YourAction")))
{
    // Custom behavior here
}
```

## Migration from Old System

### Old Component-Based System
- **REMOVED**: Separate classes (DashAction, MeleeAttackAction, BlockAction)
- **REMOVED**: Direct input bindings in PlayerCharacter
- **REMOVED**: AbilityAction wrapper class

### New Unified System
- **Single Class**: UniversalAction handles everything
- **Data-Driven**: All configuration in ActionDataAsset
- **Flexible**: Any action can be assigned to any slot
- **Extensible**: Easy to add new actions without new code

## Troubleshooting

### Action Not Working
1. Check if ActionDataAsset is properly configured
2. Verify ActionClass is set to UniversalAction
3. Ensure ActionTag is registered in AtlasGameplayTags
4. Check console for error messages

### Can't Find Action in List
1. Action must have valid ActionDataAsset in Content/Data/Actions/
2. ActionTag must be under Action.Ability.* namespace
3. Use Atlas_ListActions to see all available actions

### Input Not Responding
1. Verify slot has action assigned (Atlas_ShowSlots)
2. Check if action is on cooldown
3. Ensure character state allows action (not stunned, etc.)

## Best Practices

1. **Naming Convention**: All action tags should be `Action.Ability.YourActionName`
2. **Data Asset Location**: Keep all ActionDataAssets in `Content/Data/Actions/`
3. **Documentation**: Document new actions in CLAUDE.md
4. **Testing**: Use console commands to test action configurations
5. **Balancing**: Adjust values in data assets, not code
6. **Integrity Costs**: 
   - Low Risk: 0 cost
   - Medium Risk: 5-10 cost
   - High Risk: 15-25 cost

## Future Enhancements
- Action upgrade system
- Combo system between actions
- Visual action customization UI
- Action synergies and bonuses
- Quick-swap action sets