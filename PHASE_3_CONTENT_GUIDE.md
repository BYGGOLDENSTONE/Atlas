# Phase 3 Content Creation Guide

## Overview
This guide explains how to create content for Phase 3 of Atlas using the implemented C++ systems. All DataAssets and Blueprints should be created in the Unreal Editor using these base classes.

## Table of Contents
1. [Creating Reward DataAssets](#creating-reward-dataassets)
2. [Creating Room DataAssets](#creating-room-dataassets)
3. [Creating UI Widgets](#creating-ui-widgets)
4. [Creating Enemy Blueprints](#creating-enemy-blueprints)
5. [Testing Your Content](#testing-your-content)
6. [Best Practices](#best-practices)

---

## Creating Reward DataAssets

### Step 1: Create the DataAsset
1. **Right-click** in Content Browser → `Miscellaneous` → `Data Asset`
2. **Select** `RewardDataAsset` as parent class
3. **Name** using convention: `DA_Reward_[Category]_[Name]`
   - Example: `DA_Reward_Defense_ImprovedBlock`

### Step 2: Configure Base Properties
```
Identity:
- Reward Tag: Reward.[Category].[Name]
- Reward Name: Display name (localized)
- Description: Clear effect description
- Icon: 256x256 texture

Configuration:
- Category: Select from dropdown
- Slot Cost: 1-3 slots
- Max Stack Level: 1-5
- Stack Multipliers: Array of effectiveness per level
```

### Category-Specific Configuration

#### Defense Rewards
```
For ability replacements:
- Linked Action: Reference to ActionDataAsset
- Replaces Existing Ability: true
- Replaces Action Tag: Action.Block (or other)

For passive defense:
- Stat Modifiers: DamageReduction → value
```

#### Offense Rewards
```
For damage boosts:
- Stat Modifiers:
  - DamageMultiplier → 0.2 (20% increase)
  - AttackSpeed → 0.3 (30% faster)

For special attacks:
- Linked Action: Reference to enhanced attack
```

#### Passive Stats
```
Stat Modifiers:
- MaxHealth → 50 (adds 50 HP)
- MoveSpeed → 0.2 (20% faster)
- KnockbackForce → 0.5 (50% stronger)
- PoiseMax → 50 (adds 50 poise)
```

#### Passive Abilities
```
- Passive Ability Tag: Ability.[Type].[Name]
- Passive Parameters: (varies by ability)
  - ReviveHealth → 0.5
  - LifestealPercent → 0.1
  - HealthThreshold → 0.3
```

#### Interactables
```
- Interactable Type: Interact.[Type].[Name]
- Interaction Range: 400-1000 units
- Interactable Cooldown: 5-20 seconds
```

### Example: Creating "Improved Block" Reward

1. Create DataAsset: `DA_Reward_Defense_ImprovedBlock`
2. Configure:
   ```
   Reward Tag: Reward.Defense.ImprovedBlock
   Reward Name: "Improved Block"
   Description: "Reduces 60/70/80% damage when blocking"
   Category: Defense
   Slot Cost: 1
   Max Stack Level: 3
   Stack Multipliers: [0.6, 0.7, 0.8]
   Replaces Existing: true
   Replaces Tag: Action.Block
   ```

---

## Creating Room DataAssets

### Step 1: Create the DataAsset
1. **Right-click** in Content Browser → `Miscellaneous` → `Data Asset`
2. **Select** `RoomDataAsset` as parent class
3. **Name** using convention: `DA_Room_[Letter]_[Theme]`
   - Example: `DA_Room_A_Engineering`

### Step 2: Configure Room Properties
```
Identity:
- Room ID: Room_[Letter]_[Theme]
- Room Name: "Engineering Bay"
- Description: Atmospheric description
- Room Icon: 512x512 texture

Configuration:
- Room Theme: Defense/Offense/etc.
- Difficulty: Easy/Medium/Hard/Boss
- Environmental Hazard: ElectricalSurges/etc.
- Appear On Levels: [1,2,3] (array)
- Room Selection Weight: 1.0
```

### Step 3: Configure Enemy
```
Enemy:
- Unique Enemy: BP_Enemy_HeavyTechnician (Blueprint ref)
- Enemy Name: "Chief Engineer Hayes"
- Enemy Description: Lore text
- Enemy Base Power: 1-5
- Enemy AI Preset: AI.Defensive
- Enemy Abilities: [Action.Block, Action.Shield]
```

### Step 4: Configure Reward Pool
Add rewards with weights:
```
Reward Pool:
- DA_Reward_Defense_ImprovedBlock (weight: 2.0)
- DA_Reward_Defense_ParryMaster (weight: 2.0)
- DA_Reward_Passive_Vitality (weight: 1.0)
```

### Step 5: Set Environmental Properties
```
Environment:
- Room Level: /Game/Maps/Rooms/L_Engineering
- Integrity Drain Rate: 0.0
- Time Limit: 0 (no limit)
```

---

## Creating UI Widgets

### Creating WBP_RewardSelection

1. **Create Widget Blueprint**
   - Parent: `RewardSelectionWidget`
   - Name: `WBP_RewardSelection`

2. **Required UI Elements** (must match C++ bindings):
   ```
   - BackgroundOverlay (Image)
   - TitleText (TextBlock)
   - CardsContainer (HorizontalBox)
   - TimerBar (ProgressBar)
   - TimeRemainingText (TextBlock)
   - SkipButton (Button)
   - SkipButtonText (TextBlock)
   ```

3. **Configure Properties**:
   ```
   Selection Timeout: 15.0
   Allow Skip: false
   Reward Card Class: WBP_RewardCard
   ```

4. **Implement Blueprint Events**:
   - `PlayIntroAnimation`: Cards slide in
   - `PlayOutroAnimation`: Fade out
   - `PlaySelectionMadeAnimation`: Selected card glows

### Creating WBP_SlotManager

1. **Create Widget Blueprint**
   - Parent: `SlotManagerWidget`
   - Name: `WBP_SlotManager`

2. **Required UI Elements**:
   ```
   - TitleText (TextBlock)
   - SlotGrid (UniformGridPanel)
   - InfoPanel (VerticalBox)
   - SelectedRewardName (TextBlock)
   - SelectedRewardDescription (TextBlock)
   - StatsPreview (VerticalBox)
   - ActionButtons (HorizontalBox)
   - EquipButton (Button)
   - ReplaceButton (Button)
   - ClearButton (Button)
   - CancelButton (Button)
   ```

3. **Configure Properties**:
   ```
   Total Slots: 6
   Grid Columns: 3
   Slot Widget Class: WBP_SlotWidget
   ```

### Creating WBP_RunProgress

1. **Create Widget Blueprint**
   - Parent: `RunProgressWidget`
   - Name: `WBP_RunProgress`

2. **Required UI Elements**:
   ```
   Room Progress:
   - RoomProgressContainer (HorizontalBox)
   - CurrentLevelText (TextBlock)
   - CurrentRoomName (TextBlock)
   
   Health:
   - HealthBar (ProgressBar)
   - HealthText (TextBlock)
   
   Integrity:
   - IntegrityBar (ProgressBar)
   - IntegrityText (TextBlock)
   ```

---

## Creating Enemy Blueprints

### Base Enemy Setup

1. **Create Blueprint**
   - Parent: `GameCharacterBase`
   - Name: `BP_Enemy_[Type]`

2. **Add Components**:
   ```
   - ActionManagerComponent
   - HealthComponent
   - AIDifficultyComponent
   - AIController
   - BehaviorTreeComponent
   ```

### Example: BP_Enemy_HeavyTechnician

1. **Character Settings**:
   ```
   Health: 150
   Movement Speed: 450
   Poise: 150
   ```

2. **AI Configuration**:
   ```
   AI Preset: AI.Defensive
   Behavior Tree: BT_HeavyTechnician
   Abilities: [Block, ShieldDeploy]
   ```

3. **Unique Abilities**:
   - Shield Deploy: 100 HP frontal barrier
   - Emergency Repair: 30 HP heal over 3s
   - Overcharge: Next attack 2x damage

### Enemy Behavior Trees

Create in Content Browser:
1. **Right-click** → `Artificial Intelligence` → `Behavior Tree`
2. **Name**: `BT_[EnemyType]`

Example Structure:
```
Root
├── Selector
│   ├── Sequence [Special Ability Available]
│   │   ├── Check Cooldown
│   │   └── Execute Special
│   ├── Sequence [Low Health]
│   │   ├── Retreat
│   │   └── Defensive Actions
│   └── Sequence [Default Combat]
│       ├── Approach Player
│       ├── Attack Pattern
│       └── Cooldown Wait
```

---

## Testing Your Content

### Console Commands for Testing

#### Test Rewards
```
Atlas.Phase3.ListRewards              // See all rewards
Atlas.Phase3.GiveReward SharpBlade    // Give specific reward
Atlas.Phase3.ShowSlots                // Check equipped rewards
Atlas.Phase3.PresentChoice Vitality IronSkin  // Test selection UI
```

#### Test Rooms
```
Atlas.Phase3.ListRooms                // See all rooms
Atlas.Phase3.StartRun                 // Begin new run
Atlas.Phase3.CompleteRoom             // Complete current room
Atlas.Phase3.ShowRunProgress          // Check progress
```

#### Test UI
```
Atlas.Phase3.ShowRewardUI             // Display reward selection
Atlas.Phase3.ShowSlotUI               // Display slot manager
```

#### Validation
```
Atlas.Phase3.RunTests                 // Run all validation tests
Atlas.Phase3.QuickTest rewards        // Quick test specific system
```

---

## Best Practices

### DataAsset Organization
```
Content/
├── DataAssets/
│   ├── Rewards/
│   │   ├── Defense/
│   │   ├── Offense/
│   │   ├── PassiveStats/
│   │   ├── PassiveAbilities/
│   │   └── Interactables/
│   └── Rooms/
│       ├── DA_Room_A_Engineering
│       ├── DA_Room_B_Medical
│       └── ...
```

### Naming Conventions
- **Rewards**: `DA_Reward_[Category]_[Name]`
- **Rooms**: `DA_Room_[Letter]_[Theme]`
- **UI Widgets**: `WBP_[WidgetName]`
- **Enemy Blueprints**: `BP_Enemy_[Type]`
- **Behavior Trees**: `BT_[EnemyType]`

### Balancing Guidelines

#### Slot Costs
- 1 Slot: Minor upgrades, basic stat boosts
- 2 Slots: Major abilities, significant effects
- 3 Slots: Game-changing abilities (Second Wind)

#### Stack Levels
- 1 Stack: Powerful one-time effects
- 2 Stacks: Moderate upgrades
- 3 Stacks: Incremental improvements

#### Enemy Power Scaling
```
Level 1: Power 1-2 (Easy enemies)
Level 2: Power 2-3 (Medium enemies)
Level 3: Power 3-4 (Challenging)
Level 4: Power 4-5 (Hard)
Level 5: Power 5-6 (Boss level)
```

### Testing Checklist

Before considering content complete:

- [ ] All 25 rewards created and functional
- [ ] All 5 rooms configured with enemies
- [ ] UI widgets display correctly
- [ ] Enemy AI behaves as intended
- [ ] Rewards persist between runs
- [ ] Room progression works correctly
- [ ] Console commands function
- [ ] Performance stays above 60 FPS
- [ ] No critical errors in log

---

## Troubleshooting

### Common Issues

**Reward not appearing in selection:**
- Check reward pool configuration in room
- Verify minimum level requirements
- Ensure reward DataAsset is valid

**Enemy not spawning:**
- Verify Blueprint reference in room DataAsset
- Check enemy Blueprint has all required components
- Ensure spawn points are configured

**UI not updating:**
- Verify widget bindings match C++ names exactly
- Check component references are set
- Ensure events are implemented

**Slots not working:**
- Verify SlotManagerComponent is on player
- Check reward slot costs vs available slots
- Ensure save/load system initialized

---

## Next Steps

1. Create all 25 reward DataAssets using factory definitions
2. Configure 5 room DataAssets with proper reward pools
3. Build UI widgets following specifications
4. Create enemy Blueprints and behavior trees
5. Test all systems using console commands
6. Balance and iterate based on playtesting

Remember: The C++ framework is complete. Your job is to create the content that brings it to life!