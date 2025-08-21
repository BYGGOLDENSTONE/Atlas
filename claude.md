# Atlas - Combat System

**Engine**: UE 5.5 | **Genre**: 1v1 roguelite duels on decaying space station

## Core Mechanics
- **Dual Fail State**: Manage both health and station integrity
- **Vulnerability System**: Enemies take 8x damage when vulnerable
- **Risk/Reward**: Powerful abilities damage the station

## Action System (Data-Driven)
- **5 Slots**: LMB, RMB, E, R, Space (any ability → any slot)
- **15 Abilities**: All equal, fully customizable via DataAssets
- **Single Component**: ActionManagerComponent handles everything

## Combat Values
| System | Values |
|--------|---------|
| Basic Attack | 5 damage, 100 knockback, 20 poise |
| Heavy Attack | 15 damage, 500 knockback + ragdoll |
| Block | 40% damage reduction |
| Vulnerability | 8x damage multiplier |
| Poise | 100 max, 2s stagger at 0 |
| Dash | 400 units, i-frames, 2s cooldown |

## Architecture
```
GameCharacterBase
├── ActionManagerComponent (all combat/actions)
├── HealthComponent (health/poise)
├── VulnerabilityComponent (tiers/i-frames)
├── FocusModeComponent (targeting)
└── StationIntegrityComponent (station health)
```

### Interfaces (4)
- `ICombatInterface` - Combat states and actions
- `IHealthInterface` - Health management
- `IActionInterface` - Action system
- `IInteractable` - Environment

## Animation-Driven Combat
Attacks use animation notifies for timing:
1. Frame 0: `CombatStateNotify` (set state)
2. Attack frames: `AttackNotifyState` (hit detection)
3. Combo window: `ComboWindowNotifyState` (buffering)
4. Last frame: `CombatStateNotify` (clear state)

**Important**: Set `bCanBeInterrupted = false` in attack DataAssets

## Console Commands
```
Atlas_AssignAction [Slot] [ActionTag]
Atlas_ClearSlot [Slot]
Atlas_SwapSlots [Slot1] [Slot2]
Atlas_ListActions
Atlas_ShowSlots
```

## Debug Commands
```
Atlas.DamageIntegrity [amount]
Atlas.SetIntegrityPercent [percent]
Atlas.ShowIntegrityStatus
Atlas.ToggleFocusMode
```

## DataAsset Configuration
Required fields for each action:
- `ActionType` - MeleeAttack, RangedAttack, etc.
- `Damage/Knockback/Poise` - Combat values
- `AnimationMontage` - Attack animation
- `ActionDuration` - Total time
- `bCanBeInterrupted` - Set false for attacks
- `IntegrityCost` - Station damage

## Next Priority
1. Configure all 15 DataAssets in editor
2. Set proper damage/timing values
3. Assign animation montages
4. Test each ability

## Rules
- NO GAS - custom components only
- Data-driven via DataAssets
- Interface-based communication
- No parry or camera lock

## Documentation
- `ARCHITECTURE_IMPROVEMENTS.md` - Technical details
- `changelog.md` - Development history