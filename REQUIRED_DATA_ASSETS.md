# Required Data Assets for Atlas

## ActionDataAssets (For Universal Action System)
All ActionDataAssets should be created in `Content/Data/Actions/` folder

### All Available Abilities (15 Assets)
Every ability can be assigned to any slot (Slot1-5) or removed completely. IntegrityCost is fully configurable per ability - you decide the risk/reward balance.

| Asset Name | Action Tag | Type | Suggested Settings |
|------------|------------|------|-------------------|
| **DA_BasicAttack** | Action.Ability.BasicAttack | MeleeAttack | Damage: 5, Knockback: 100, PoiseDamage: 20, Cooldown: 0, IntegrityCost: 0 |
| **DA_HeavyAttack** | Action.Ability.HeavyAttack | MeleeAttack | Damage: 15, Knockback: 500, PoiseDamage: 40, CausesRagdoll: true, Cooldown: 1.5, IntegrityCost: 0 |
| **DA_Block** | Action.Ability.Block | Defense | DamageReduction: 0.4, RequiresHoldInput: true, Cooldown: 0, IntegrityCost: 0 |
| **DA_Dash** | Action.Ability.Dash | Movement | DashDistance: 400, DashDuration: 0.3, GrantsInvincibility: true, Cooldown: 2.0, IntegrityCost: 0 |
| **DA_FocusMode** | Action.Focus | Utility | InteractionRange: 2000, RequiresHoldInput: true, Cooldown: 0, IntegrityCost: 0 |
| **DA_KineticPulse** | Action.Ability.KineticPulse | RangedAttack | ProjectileSpeed: 1500, Knockback: 300, Range: 500, Cooldown: 2.0, IntegrityCost: 0 |
| **DA_DebrisPull** | Action.Ability.DebrisPull | Utility | InteractionRange: 1000, ChargeTime: 0.5, Cooldown: 3.0, IntegrityCost: 0 |
| **DA_CoolantSpray** | Action.Ability.CoolantSpray | AreaEffect | Radius: 300, Duration: 10, Cooldown: 5.0, IntegrityCost: 0 |
| **DA_SystemHack** | Action.Ability.SystemHack | Utility | InteractionRange: 2000, ChargeTime: 1.0, Cooldown: 4.0, IntegrityCost: 0 |
| **DA_FloorDestabilizer** | Action.Ability.FloorDestabilizer | AreaEffect | Radius: 400, EffectDamage: 10, Duration: 3, Cooldown: 6.0, IntegrityCost: 0 |
| **DA_ImpactGauntlet** | Action.Ability.ImpactGauntlet | MeleeAttack | Damage: 25, Knockback: 800, ChargeTime: 1.0, Cooldown: 4.0, IntegrityCost: 0 |
| **DA_LocalizedEMP** | Action.Ability.LocalizedEMP | AreaEffect | Radius: 500, Duration: 5, Cooldown: 8.0, IntegrityCost: 0 |
| **DA_SeismicStamp** | Action.Ability.SeismicStamp | AreaEffect | Radius: 600, EffectDamage: 30, Knockback: 400, Cooldown: 10.0, IntegrityCost: 0 |
| **DA_GravityAnchor** | Action.Ability.GravityAnchor | Special | CustomParameters, Cooldown: 12.0, IntegrityCost: 0 |
| **DA_AirlockBreach** | Action.Ability.AirlockBreach | AreaEffect | Radius: 800, Duration: 8, EffectDamage: 20, Cooldown: 15.0, IntegrityCost: 0 |

**Note**: All IntegrityCost values start at 0. You control the balance by adjusting IntegrityCost in each DataAsset. Examples:
- Keep BasicAttack at 0 for a reliable fallback
- Set powerful abilities like SeismicStamp to 15-25 for high risk/reward
- Or make EVERY ability cost integrity for hardcore gameplay
- The system is completely flexible!

## Other Required DataAssets (System Configuration)

### Combat System (3 Assets)
Located in `Content/Data/Combat/`

| Asset Name | Class | Purpose |
|------------|-------|---------|
| **DA_CombatRules** | CombatRulesDataAsset | Global combat values (multipliers, timings, rules) |
| **DA_AttackData_Jab** | AttackDataAsset | Legacy jab attack config (for AI) |
| **DA_AttackData_Heavy** | AttackDataAsset | Legacy heavy attack config (for AI) |

### Station Integrity (1 Asset)
Located in `Content/Data/Systems/`

| Asset Name | Class | Purpose |
|------------|-------|---------|
| **DA_StationIntegrity** | StationIntegrityDataAsset | Integrity thresholds, warnings, damage values |

### Debug Commands (1 Asset)
Located in `Content/Data/Debug/`

| Asset Name | Class | Purpose |
|------------|-------|---------|
| **DA_DebugCommands** | DebugCommandsDataAsset | Console command definitions |

## Total Required DataAssets: 20

### Breakdown:
- **15 ActionDataAssets** (all abilities for Universal Action System)
- **2 Legacy Attack DataAssets** (for AI behavior trees only)
- **1 Combat Rules DataAsset** (system configuration)
- **1 Station Integrity DataAsset**
- **1 Debug Commands DataAsset**

## Creation Checklist

### For Each ActionDataAsset:
1. Right-click in Content Browser → Miscellaneous → Data Asset
2. Select `ActionDataAsset` as class
3. Name it according to the table above
4. Configure these fields:
   - **Identity**: 
     - ActionTag (e.g., Action.Ability.BasicAttack)
     - ActionType (Movement/Defense/MeleeAttack/RangedAttack/AreaEffect/Utility/Special)
     - ActionClass: **UniversalAction** (ALWAYS use this)
     - Name & Description
   - **Universal**: 
     - Cooldown (seconds between uses)
     - IntegrityCost (YOU decide - 0 for no risk, any value for risk/reward)
   - **Type-Specific**: 
     - Fields auto-show/hide based on ActionType selection
     - Only fill in relevant fields for your action type
   - **Requirements** (optional): 
     - RequiredTags (player must have these tags)
     - BlockedDuringTags (can't use while these tags are active)
   - **Animation** (optional): 
     - ActionMontage (animation to play)

## Example Slot Configurations

**Balanced Loadout:**
```
Slot1 (LMB): BasicAttack
Slot2 (RMB): Block  
Slot3 (E): HeavyAttack
Slot4 (R): KineticPulse
Slot5 (Space): Dash
```

**Ability-Focused:**
```
Slot1 (LMB): KineticPulse
Slot2 (RMB): DebrisPull
Slot3 (E): CoolantSpray
Slot4 (R): SystemHack
Slot5 (Space): Dash
```

**Power Build:**
```
Slot1 (LMB): SeismicStamp
Slot2 (RMB): GravityAnchor
Slot3 (E): AirlockBreach
Slot4 (R): ImpactGauntlet
Slot5 (Space): Dash
```

Remember: ANY ability can go in ANY slot!

## Balancing Philosophy

The IntegrityCost field in each ActionDataAsset gives you complete control over game balance:

**Example Balance Profiles:**

**Classic Mode**: Basic abilities free, powerful abilities cost integrity
- BasicAttack, Block, Dash: 0 cost
- KineticPulse, DebrisPull: 2-3 cost  
- ImpactGauntlet, LocalizedEMP: 5-10 cost
- SeismicStamp, AirlockBreach: 15-25 cost

**Risk/Reward Mode**: Everything has a cost, scaled by power
- BasicAttack: 1 cost
- Block: 2 cost
- Dash: 3 cost
- Scale up from there based on ability power

**Arcade Mode**: No integrity costs, pure action
- All abilities: 0 cost
- Balance through cooldowns only

**Survival Mode**: High costs force careful ability use
- Even basic abilities cost 5+
- Powerful abilities cost 30+
- Every action matters

The beauty is YOU decide - just adjust the IntegrityCost value in each DataAsset!

## Validation Commands

After creating all DataAssets, use these console commands to verify:

```bash
# List all available actions (should show 15)
Atlas_ListActions

# Test slot assignments
Atlas_ShowSlots

# Test action assignment
Atlas_AssignAction Slot1 Action.Ability.BasicAttack
```