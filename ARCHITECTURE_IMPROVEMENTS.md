# Atlas Architecture - Final State

## Current Architecture (2025-01-21)

### Component Hierarchy
```
GameCharacterBase (implements all interfaces)
    ├── ActionManagerComponent (unified combat/action system)
    ├── HealthComponent (health/poise management)
    ├── VulnerabilityComponent (vulnerability tiers)
    ├── FocusModeComponent (focus mode targeting)
    └── StationIntegrityComponent (station health)
```

### Core Interfaces (4 Total)
1. **ICombatInterface** - All combat interactions (state, attack, block, vulnerability, poise, damage)
2. **IHealthInterface** - Health queries and modification
3. **IActionInterface** - Action system abstraction
4. **IInteractable** - Environmental interactions

## Key Improvements

### 1. Unified Component System ✅
- **Single Source of Truth**: ActionManagerComponent handles ALL combat and action logic
- **No Legacy Components**: CombatComponent and DamageCalculator completely removed
- **Clean Dependencies**: Components communicate only through interfaces

### 2. Function Map Routing ✅
- **O(1) Action Execution**: Replaced if/else chains with function pointer map
- **Easy Extensibility**: Add new actions without modifying routing logic
- **Clear Separation**: Each action type has dedicated execution method

### 3. Data-Driven Configuration ✅
All configuration in ActionDataAsset:
- `ActionDuration` - How long action takes
- `MontagePlayRate` - Animation speed multiplier
- `AttackWindupTime` - Time before damage
- `AttackRecoveryTime` - Time after damage
- `bCanBeInterrupted` - Can action be cancelled
- `bAutoReleaseOnComplete` - Auto-end when timer expires
- `bIsToggleAction` - Toggle on/off abilities
- `InputBufferWindow` - Action queueing window

### 4. Animation-Driven Combat ✅
- **AttackNotifyState** - Hit detection windows
- **CombatStateNotify** - State transitions
- **ComboWindowNotifyState** - Input buffering
- No hardcoded timers - animations control everything

## Testing Checklist
- [ ] All 15 abilities work with new system
- [ ] Action interruption respects `bCanBeInterrupted`
- [ ] Toggle actions (Focus Mode) work correctly
- [ ] Montage playback rates apply properly
- [ ] Station integrity costs are deducted
- [ ] Cooldowns work as configured
- [ ] Combat state tags update correctly
- [ ] Damage multipliers apply (8x vulnerable, 0.6x blocked)

## Performance Notes
- Interface calls have minimal overhead in C++
- Function pointer map is more efficient than if/else chains
- Component references cached during action execution
- Data-driven approach reduces recompilation needs

## Migration Complete
- ✅ All legacy components removed
- ✅ All references updated to ActionManagerComponent
- ✅ Interface consolidation complete
- ✅ Data-driven system fully implemented