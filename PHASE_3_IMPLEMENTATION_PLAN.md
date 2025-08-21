# Phase 3: Content Creation & UI Implementation - Ultra-Detailed Plan

## Executive Summary
Phase 3 transforms the completed framework into a playable game through content creation, UI implementation, and system integration. This document provides step-by-step instructions for every aspect of implementation.

---

## Part 1: Reward DataAsset Creation

### Setup Instructions for Each Reward

#### Step 1: Create Base Reward DataAsset
1. **Right-click** in Content Browser → Miscellaneous → Data Asset
2. **Select** `RewardDataAsset` as parent class
3. **Name** using convention: `DA_Reward_[Category]_[Name]`
4. **Configure** base properties:
   ```
   RewardTag: Reward.[Category].[Name]
   RewardName: Localized display name
   Description: Clear, concise effect description
   Icon: 256x256 texture (create in Photoshop/GIMP)
   ```

### 1.1 Defense Category Implementation

#### DA_Reward_Defense_ImprovedBlock
```cpp
Configuration:
- RewardTag: Reward.Defense.ImprovedBlock
- Category: Defense
- SlotCost: 1
- MaxStackLevel: 3
- StackMultipliers: [0.6, 0.7, 0.8]
- LinkedAction: DA_Action_Block_Enhanced
- bReplacesExistingAbility: true
- ReplacesActionTag: Action.Block

Implementation:
1. Duplicate existing DA_Action_Block
2. Modify DamageReduction values per stack
3. Add visual effect for enhanced block
4. Update block animation notify to check for enhancement
```

#### DA_Reward_Defense_ParryMaster
```cpp
Configuration:
- RewardTag: Reward.Defense.ParryMaster
- Category: Defense
- SlotCost: 1
- MaxStackLevel: 2
- StackMultipliers: [1.5, 2.0]
- PassiveParameters:
  - ParryWindowExtension: 0.1
  - LateParryWindowExtension: 0.05

Blueprint Integration:
1. In BP_PlayerCharacter BeginPlay:
   - Get SlotManagerComponent
   - Bind to OnRewardEquipped
2. On Reward Equipped:
   - If RewardTag matches ParryMaster
   - Modify ActionManager parry windows
3. Create custom ParryNotifyState that checks for extension
```

#### DA_Reward_Defense_CounterStrike
```cpp
Configuration:
- RewardTag: Reward.Defense.CounterStrike
- Category: Defense
- SlotCost: 2
- MaxStackLevel: 1
- LinkedAction: DA_Action_Counter
- PassiveAbilityTag: Ability.Counter.Auto

Implementation Details:
1. Create DA_Action_Counter:
   - Damage: 10
   - AnimationMontage: AM_CounterAttack
   - bCanBeInterrupted: false
2. Modify ParryNotifyState:
   - On successful parry
   - Check for CounterStrike reward
   - Queue counter action automatically
3. Add 0.2s invulnerability during counter
```

#### DA_Reward_Defense_IronSkin
```cpp
Configuration:
- RewardTag: Reward.Defense.IronSkin
- Category: PassiveStats
- SlotCost: 1
- MaxStackLevel: 3
- StatModifiers:
  - DamageReduction: 0.1
- StackMultipliers: [1.0, 1.5, 2.0]

Implementation:
1. In HealthComponent::TakeDamage:
   - Query SlotManager for DamageReduction stat
   - Apply before vulnerability calculation
2. Add metallic overlay material when equipped
3. Play metal impact sound on hit
```

#### DA_Reward_Defense_LastStand
```cpp
Configuration:
- RewardTag: Reward.Defense.LastStand
- Category: PassiveAbility
- SlotCost: 2
- MaxStackLevel: 1
- PassiveAbilityTag: Ability.LastStand
- PassiveParameters:
  - HealthThreshold: 0.25
  - BlockBreakImmunity: 1.0

Implementation:
1. Create ULastStandComponent:
   - Monitor health percentage
   - When below 25%, set flag
2. Modify Block action:
   - Check LastStand flag
   - Ignore poise damage when active
3. Add red aura VFX when active
4. Play heartbeat sound effect
```

### 1.2 Offense Category Implementation

#### DA_Reward_Offense_SharpBlade
```cpp
Configuration:
- RewardTag: Reward.Offense.SharpBlade
- Category: PassiveStats
- SlotCost: 1
- MaxStackLevel: 3
- StatModifiers:
  - DamageMultiplier: 0.2
- StackMultipliers: [1.0, 1.75, 2.5]

Implementation:
1. Modify ActionManagerComponent::ExecuteAttack:
   float DamageMultiplier = 1.0f + SlotManager->CalculateTotalStatModifier("DamageMultiplier");
   FinalDamage *= DamageMultiplier;
2. Add sword gleam VFX to weapon
3. Play sharper hit sounds
```

#### DA_Reward_Offense_HeavyImpact
```cpp
Configuration:
- RewardTag: Reward.Offense.HeavyImpact
- Category: Offense
- SlotCost: 2
- MaxStackLevel: 2
- LinkedAction: DA_Action_HeavyAttack_AOE
- bReplacesExistingAbility: true
- ReplacesActionTag: Action.Attack.Heavy

AOE Implementation:
1. Create radial damage component
2. On heavy attack impact:
   - Spawn damage sphere (300 unit radius)
   - Apply 50% damage to all enemies
   - Add camera shake
3. Spawn ground crack decal
4. Play shockwave particle effect
```

#### DA_Reward_Offense_BleedingStrikes
```cpp
Configuration:
- RewardTag: Reward.Offense.BleedingStrikes
- Category: PassiveAbility
- SlotCost: 1
- MaxStackLevel: 3
- PassiveAbilityTag: Ability.DOT.Bleed
- PassiveParameters:
  - BleedDamage: 2
  - BleedDuration: 5
  - BleedTickRate: 0.5

DOT System Implementation:
1. Create UDamageOverTimeComponent:
   - Track active DOTs
   - Handle tick damage
   - Stack management
2. On any attack hit:
   - Apply bleed effect
   - Stack if already bleeding
3. Blood drip particle effect
4. Red damage numbers for DOT
```

#### DA_Reward_Offense_Executioner
```cpp
Configuration:
- RewardTag: Reward.Offense.Executioner
- Category: PassiveAbility
- SlotCost: 2
- MaxStackLevel: 1
- PassiveAbilityTag: Ability.Execution
- PassiveParameters:
  - StunnedDamageMultiplier: 2.0

Implementation:
1. Check target vulnerability state
2. If Stunned/Staggered:
   - Apply 2x damage multiplier
   - Play execution animation
   - Add decapitation VFX
3. Screen flash on execution
4. Brutal kill sound effect
```

#### DA_Reward_Offense_RapidStrikes
```cpp
Configuration:
- RewardTag: Reward.Offense.RapidStrikes
- Category: PassiveStats
- SlotCost: 1
- MaxStackLevel: 3
- StatModifiers:
  - AttackSpeed: 0.3
- StackMultipliers: [1.0, 1.5, 2.0]

Animation Speed Implementation:
1. Get AnimInstance in BeginPlay
2. On stat change:
   AnimInstance->GlobalAnimRateScale = 1.0f + AttackSpeedBonus;
3. Reduce action cooldowns proportionally
4. Add motion blur to attacks
```

### 1.3 Passive Stats Implementation

#### DA_Reward_Passive_Vitality
```cpp
Configuration:
- RewardTag: Reward.Passive.Vitality
- Category: PassiveStats
- SlotCost: 1
- MaxStackLevel: 3
- StatModifiers:
  - MaxHealth: 50
- StackMultipliers: [1.0, 1.5, 2.0]

Health Scaling:
1. On equip:
   float HealthBonus = SlotManager->CalculateTotalStatModifier("MaxHealth");
   HealthComponent->SetMaxHealth(BaseHealth + HealthBonus, true);
2. Update health bar UI max value
3. Heal to new max if upgrading
```

#### DA_Reward_Passive_Swiftness
```cpp
Configuration:
- RewardTag: Reward.Passive.Swiftness
- Category: PassiveStats
- SlotCost: 1
- MaxStackLevel: 2
- StatModifiers:
  - MoveSpeed: 0.2
- StackMultipliers: [1.0, 1.5]

Movement Implementation:
1. In Character tick:
   float SpeedMultiplier = 1.0f + SlotManager->CalculateTotalStatModifier("MoveSpeed");
   CharacterMovement->MaxWalkSpeed = BaseSpeed * SpeedMultiplier;
2. Add speed lines VFX when moving
3. Increase footstep frequency
```

#### DA_Reward_Passive_Heavyweight
```cpp
Configuration:
- RewardTag: Reward.Passive.Heavyweight
- Category: PassiveStats
- SlotCost: 1
- MaxStackLevel: 2
- StatModifiers:
  - KnockbackForce: 0.5
  - KnockbackResistance: 0.25
- StackMultipliers: [1.0, 1.5]

Physics Implementation:
1. Modify knockback calculation:
   OutgoingKnockback *= (1.0f + KnockbackForce);
   IncomingKnockback *= (1.0f - KnockbackResistance);
2. Add screen shake on heavy impacts
3. Heavier footstep sounds
```

#### DA_Reward_Passive_Regeneration
```cpp
Configuration:
- RewardTag: Reward.Passive.Regeneration
- Category: PassiveStats
- SlotCost: 2
- MaxStackLevel: 2
- StatModifiers:
  - HealthRegenRate: 2
  - CombatRegenDelay: 3
- StackMultipliers: [1.0, 1.5]

Regen System:
1. Create health regen timer
2. Reset on damage taken
3. After delay, heal per second
4. Green healing particles
5. Soft healing sound loop
```

#### DA_Reward_Passive_Fortitude
```cpp
Configuration:
- RewardTag: Reward.Passive.Fortitude
- Category: PassiveStats
- SlotCost: 1
- MaxStackLevel: 3
- StatModifiers:
  - PoiseMax: 50
  - PoiseRegenRate: 0.5
- StackMultipliers: [1.0, 1.5, 2.0]

Poise Enhancement:
1. Scale max poise value
2. Increase regen rate
3. Add stability VFX aura
4. Reduce stagger animation intensity
```

### 1.4 Passive Abilities Implementation

#### DA_Reward_Ability_SecondWind
```cpp
Configuration:
- RewardTag: Reward.Ability.SecondWind
- Category: PassiveAbility
- SlotCost: 3
- MaxStackLevel: 1
- PassiveAbilityTag: Ability.Revive.Once
- PassiveParameters:
  - ReviveHealth: 0.5
  - InvulnerabilityDuration: 2.0

Death Prevention System:
1. On HealthComponent death:
   - Check for SecondWind
   - If available, cancel death
   - Set health to 50%
   - Remove reward from slot
   - Grant temporary invulnerability
2. Phoenix resurrection VFX
3. Dramatic revival sound
4. Screen flash white
```

#### DA_Reward_Ability_Vampirism
```cpp
Configuration:
- RewardTag: Reward.Ability.Vampirism
- Category: PassiveAbility
- SlotCost: 2
- MaxStackLevel: 2
- PassiveAbilityTag: Ability.Lifesteal
- PassiveParameters:
  - LifestealPercent: 0.1
- StackMultipliers: [1.0, 1.5]

Lifesteal Implementation:
1. On damage dealt:
   float Healing = DamageDealt * LifestealPercent;
   HealthComponent->Heal(Healing, this);
2. Red absorption particles
3. Blood drain sound effect
4. Cannot exceed max health
```

#### DA_Reward_Ability_Berserker
```cpp
Configuration:
- RewardTag: Reward.Ability.Berserker
- Category: PassiveAbility
- SlotCost: 2
- MaxStackLevel: 1
- PassiveAbilityTag: Ability.Berserker
- PassiveParameters:
  - HealthThreshold: 0.3
  - DamageBonus: 0.5

Berserker Mode:
1. Monitor health percentage
2. When below 30%:
   - Apply damage multiplier
   - Add red rage overlay
   - Increase attack speed
   - Play heartbeat sound
3. Screen edges turn red
4. Aggressive combat music
```

#### DA_Reward_Ability_Momentum
```cpp
Configuration:
- RewardTag: Reward.Ability.Momentum
- Category: PassiveAbility
- SlotCost: 2
- MaxStackLevel: 1
- PassiveAbilityTag: Ability.Momentum
- PassiveParameters:
  - DamageIncreasePerHit: 0.05
  - MaxStacks: 10
  - StackDuration: 5.0

Combo System:
1. Track consecutive hits
2. Each hit adds stack
3. Display combo counter UI
4. Reset on miss or timeout
5. Fire effects at high stacks
6. Combo meter sound progression
```

#### DA_Reward_Ability_StationShield
```cpp
Configuration:
- RewardTag: Reward.Ability.StationShield
- Category: PassiveAbility
- SlotCost: 2
- MaxStackLevel: 1
- PassiveAbilityTag: Ability.StationShield
- PassiveParameters:
  - DamageToStation: 0.5

Damage Redirect:
1. On player damage:
   float StationDamage = Damage * 0.5;
   StationIntegrity->TakeDamage(StationDamage);
   PlayerDamage *= 0.5;
2. Energy shield VFX
3. Warning when station low
4. Electrical discharge effect
```

### 1.5 Interactables Implementation

#### DA_Reward_Interact_ExplosiveValves
```cpp
Configuration:
- RewardTag: Reward.Interact.ExplosiveValves
- Category: Interactable
- SlotCost: 1
- MaxStackLevel: 2
- InteractableType: Interact.Valve.Explosive
- InteractionRange: 800
- InteractableCooldown: 10
- StackMultipliers: [1.0, 1.5]

Valve System:
1. Spawn valve actors in room
2. On interact (E key):
   - Play valve turning animation
   - 1 second delay
   - Explosion effect (300 unit radius)
   - 100 damage (* stack multiplier)
3. Steam burst VFX
4. Metal creaking sound
5. Screen shake on explosion
```

#### DA_Reward_Interact_GravityWells
```cpp
Configuration:
- RewardTag: Reward.Interact.GravityWells
- Category: Interactable
- SlotCost: 2
- MaxStackLevel: 1
- InteractableType: Interact.Gravity.Well
- InteractionRange: 1000
- InteractableCooldown: 15

Gravity Manipulation:
1. Place gravity well marker
2. Create physics field:
   - 500 unit radius
   - Pull force: 1000
   - Duration: 5 seconds
3. Distortion visual effect
4. Low frequency hum sound
5. Slow enemies by 50%
```

#### DA_Reward_Interact_TurretHack
```cpp
Configuration:
- RewardTag: Reward.Interact.TurretHack
- Category: Interactable
- SlotCost: 2
- MaxStackLevel: 1
- InteractableType: Interact.Turret.Hack
- InteractionRange: 500
- InteractableCooldown: 20

Turret Control:
1. Find turret actors
2. On hack (2 second channel):
   - Change turret faction
   - Target enemies
   - Fire rate: 2/second
   - Damage: 10 per shot
3. Holographic hack effect
4. Digital corruption sound
5. Turret active for 15 seconds
```

#### DA_Reward_Interact_EmergencyVent
```cpp
Configuration:
- RewardTag: Reward.Interact.EmergencyVent
- Category: Interactable
- SlotCost: 1
- MaxStackLevel: 2
- InteractableType: Interact.Vent.Emergency
- InteractionRange: 600
- InteractableCooldown: 8

Vent System:
1. Mark vent locations
2. On activation:
   - Warning light flash
   - 1 second wind-up
   - Launch force: 2000
   - Direction: Away from vent
3. Steam jet VFX
4. Pressurized air sound
5. Vulnerability on wall impact
```

#### DA_Reward_Interact_PowerSurge
```cpp
Configuration:
- RewardTag: Reward.Interact.PowerSurge
- Category: Interactable
- SlotCost: 1
- MaxStackLevel: 3
- InteractableType: Interact.Panel.Overload
- InteractionRange: 400
- InteractableCooldown: 6
- StackMultipliers: [1.0, 1.5, 2.0]

Electrical Overload:
1. Target control panels
2. On overload:
   - Electric arc effect
   - AOE stun (2 seconds base)
   - Minor damage (20)
   - Chain to nearby enemies
3. Sparks particle system
4. Electrical buzzing sound
5. Screen static effect
```

---

## Part 2: Room DataAsset Configuration

### Room Creation Process

#### Step 1: Base Room Setup
```cpp
1. Create DataAsset from URoomDataAsset
2. Name: DA_Room_[Letter]_[Theme]
3. Configure base properties:
   - RoomID: Room_[Letter]
   - RoomName: Localized name
   - Description: Atmospheric description
   - RoomIcon: 512x512 texture
```

### 2.1 Room A - Engineering Bay
```cpp
Configuration:
- RoomID: Room_A_Engineering
- RoomName: "Engineering Bay"
- Description: "The heart of the station's mechanical systems"
- RoomTheme: Defense
- Difficulty: Medium
- EnvironmentalHazard: ElectricalSurges
- AppearOnLevels: [1, 2, 3]
- RoomSelectionWeight: 1.0

Enemy Configuration:
- UniqueEnemy: BP_Enemy_HeavyTechnician
- EnemyName: "Chief Engineer Hayes"
- EnemyDescription: "Once maintained the station, now maintains chaos"
- EnemyBasePower: 3
- EnemyAIPreset: AI.Defensive
- EnemyAbilities: [Action.Block, Action.Shield.Deploy]

Reward Pool (10 items):
- All 5 Defense rewards (weight: 2.0)
- 2 Passive Stats (Vitality, Fortitude)
- 2 Offense (SharpBlade, HeavyImpact)
- 1 Interactable (ExplosiveValves)

Environmental Setup:
- RoomLevel: /Game/Maps/Rooms/L_Engineering
- Spawn shield generators at corners
- Electrical hazard zones (10 damage/sec)
- Sparking panels for atmosphere
```

### 2.2 Room B - Medical Ward
```cpp
Configuration:
- RoomID: Room_B_Medical
- RoomName: "Medical Ward"
- Description: "Where healing became harm"
- RoomTheme: PassiveStats
- Difficulty: Easy
- EnvironmentalHazard: ToxicLeak
- AppearOnLevels: [1, 2]
- RoomSelectionWeight: 1.2

Enemy Configuration:
- UniqueEnemy: BP_Enemy_CorruptedMedic
- EnemyName: "Dr. Voss"
- EnemyDescription: "The cure is worse than the disease"
- EnemyBasePower: 2
- EnemyAIPreset: AI.Balanced
- EnemyAbilities: [Action.Heal.Self, Action.Poison.Apply]

Reward Pool:
- All 5 Passive Stats (weight: 2.0)
- 2 Defense (IronSkin, LastStand)
- 2 Passive Abilities (Regeneration, Vampirism)
- 1 Offense (BleedingStrikes)

Environmental Setup:
- RoomLevel: /Game/Maps/Rooms/L_Medical
- Healing stations (disabled during combat)
- Green toxic pools (5 damage/sec)
- Medical equipment obstacles
```

### 2.3 Room C - Weapons Lab
```cpp
Configuration:
- RoomID: Room_C_Weapons
- RoomName: "Weapons Laboratory"
- Description: "Where destruction was perfected"
- RoomTheme: Offense
- Difficulty: Hard
- EnvironmentalHazard: None
- AppearOnLevels: [2, 3, 4]
- RoomSelectionWeight: 0.8

Enemy Configuration:
- UniqueEnemy: BP_Enemy_SecurityChief
- EnemyName: "Commander Rex"
- EnemyDescription: "Armed and extremely dangerous"
- EnemyBasePower: 4
- EnemyAIPreset: AI.Aggressive
- EnemyAbilities: [Action.Attack.Heavy, Action.Combo.Triple]

Reward Pool:
- All 5 Offense rewards (weight: 2.0)
- 2 Defense (CounterStrike, ParryMaster)
- 2 Passive Abilities (Berserker, Momentum)
- 1 Interactable (TurretHack)

Environmental Setup:
- RoomLevel: /Game/Maps/Rooms/L_Weapons
- Weapon racks (can be destroyed)
- Testing range layout
- Prototype weapons on display
```

### 2.4 Room D - Command Center
```cpp
Configuration:
- RoomID: Room_D_Command
- RoomName: "Command Center"
- Description: "Where order turned to chaos"
- RoomTheme: PassiveAbility
- Difficulty: Hard
- EnvironmentalHazard: SystemMalfunction
- AppearOnLevels: [3, 4, 5]
- RoomSelectionWeight: 0.9

Enemy Configuration:
- UniqueEnemy: BP_Enemy_StationCommander
- EnemyName: "Admiral Kronos"
- EnemyDescription: "Captain goes down with the ship"
- EnemyBasePower: 5
- EnemyAIPreset: AI.Tactical
- EnemyAbilities: [Action.Command.Rally, Action.Orbital.Strike]

Reward Pool:
- All 5 Passive Abilities (weight: 2.0)
- 2 Offense (Executioner, RapidStrikes)
- 2 Defense (IronSkin, LastStand)
- 1 Interactable (PowerSurge)

Environmental Setup:
- RoomLevel: /Game/Maps/Rooms/L_Command
- Holographic displays
- Random system malfunctions (abilities disabled)
- Central command console
```

### 2.5 Room E - Maintenance Shaft
```cpp
Configuration:
- RoomID: Room_E_Maintenance
- RoomName: "Maintenance Shaft"
- Description: "The station's forgotten underbelly"
- RoomTheme: Interactable
- Difficulty: Medium
- EnvironmentalHazard: LowGravity
- AppearOnLevels: [1, 2, 3, 4, 5]
- RoomSelectionWeight: 1.1

Enemy Configuration:
- UniqueEnemy: BP_Enemy_MaintenanceDrone
- EnemyName: "Unit M-471"
- EnemyDescription: "Repurposed for destruction"
- EnemyBasePower: 3
- EnemyAIPreset: AI.Tactical
- EnemyAbilities: [Action.Dash, Action.Environment.Use]

Reward Pool:
- All 5 Interactables (weight: 2.0)
- 2 Passive Stats (Swiftness, Heavyweight)
- 2 Defense (ParryMaster, CounterStrike)
- 1 Offense (HeavyImpact)

Environmental Setup:
- RoomLevel: /Game/Maps/Rooms/L_Maintenance
- Low gravity (50% normal)
- Exposed machinery hazards
- Narrow corridors
- Vent systems everywhere
```

---

## Part 3: UI Widget Implementation Details

### 3.1 WBP_RewardSelection

#### Widget Structure
```
Root: Canvas Panel
├── Background (Image)
│   └── Material: M_UI_RewardBG (blur effect)
├── Title Panel (Horizontal Box)
│   ├── Icon (Image) - Trophy icon
│   └── Title (Text) - "Choose Your Reward"
├── Cards Container (Horizontal Box)
│   ├── Reward Card 1 (Border)
│   │   └── WBP_RewardCard
│   └── Reward Card 2 (Border)
│       └── WBP_RewardCard
├── Timer Bar (Progress Bar)
│   └── Fills over SelectionTimeout
└── Skip Button (Button)
    └── Visible if bAllowSkip
```

#### Blueprint Logic
```cpp
Event Construct:
1. Get RewardSelectionComponent reference
2. Bind to OnSelectionPresented event
3. Start timer countdown animation

Function UpdateRewardDisplay:
1. Get CurrentChoices from component
2. For each reward card:
   - Set icon texture
   - Set name text
   - Set description
   - Set slot cost
   - Color code by category
   - Show stack indicator if enhancing

On Card Clicked:
1. Play selection animation
2. Call component->OnRewardChosen(SelectedReward)
3. Transition to slot management if needed
```

#### Animations
- **Intro**: Cards slide in from sides (0.5s)
- **Hover**: Card scales to 1.05x, glow effect
- **Selection**: Card flashes, others fade out
- **Timer**: Progress bar depletes smoothly

### 3.2 WBP_SlotManager

#### Widget Structure
```
Root: Canvas Panel
├── Background Panel (Border)
├── Title (Text) - "Equipment Slots"
├── Slot Grid (Uniform Grid Panel)
│   ├── Slot 1 (WBP_SlotWidget)
│   ├── Slot 2 (WBP_SlotWidget)
│   ├── Slot 3 (WBP_SlotWidget)
│   ├── Slot 4 (WBP_SlotWidget)
│   ├── Slot 5 (WBP_SlotWidget)
│   └── Slot 6 (WBP_SlotWidget)
├── Info Panel (Vertical Box)
│   ├── Selected Reward Name
│   ├── Description
│   └── Stats Preview
└── Action Buttons (Horizontal Box)
    ├── Equip Button
    ├── Replace Button
    └── Cancel Button
```

#### Drag & Drop Implementation
```cpp
WBP_SlotWidget::OnMouseButtonDown:
1. Detect drag initiation
2. Create DragDropOperation
3. Set drag visual to reward icon

WBP_SlotWidget::OnDrop:
1. Get source and target slots
2. Call SlotManager->SwapRewards(SourceIndex, TargetIndex)
3. Play swap animation
4. Update both slot displays

Visual Feedback:
- Empty slots: Dashed border
- Occupied slots: Solid border with glow
- Multi-slot items: Connected visual
- Dragging: 50% opacity
- Valid drop: Green highlight
- Invalid drop: Red highlight
```

### 3.3 WBP_RunProgress

#### Widget Structure
```
Root: Overlay
├── Main Container (Vertical Box)
│   ├── Room Progress (Horizontal Box)
│   │   ├── Room 1 Icon (Completed/Current/Locked)
│   │   ├── Connector Line
│   │   ├── Room 2 Icon
│   │   ├── Connector Line
│   │   ├── Room 3 Icon
│   │   ├── Connector Line
│   │   ├── Room 4 Icon
│   │   ├── Connector Line
│   │   └── Room 5 Icon
│   ├── Health Bar (Progress Bar)
│   │   └── Text Overlay (100/100)
│   └── Station Integrity Bar (Progress Bar)
│       └── Text Overlay (100/100)
└── Current Room Info (Vertical Box)
    ├── Room Name (Text)
    ├── Enemy Name (Text)
    └── Hazard Warning (Text + Icon)
```

#### Update Logic
```cpp
Tick Event:
1. Update health from HealthComponent
2. Update integrity from StationIntegrity
3. Pulse current room icon

On Room Changed:
1. Update room icons (completed/current)
2. Animate transition between rooms
3. Update room info text
4. Flash hazard warning if applicable

Visual States:
- Completed Room: Green checkmark
- Current Room: Pulsing gold
- Future Room: Grayed out lock
- Failed Room: Red X
```

---

## Part 4: Enemy Blueprint Implementation

### 4.1 Base Enemy Setup (BP_EnemyBase)

#### Components Structure
```
BP_EnemyBase (Inherits from GameCharacterBase)
├── Mesh Component
├── ActionManagerComponent
├── HealthComponent
├── AIDifficultyComponent
├── AIController
├── Behavior Tree Component
├── Blackboard Component
├── AI Perception Component
│   ├── Sight Config (Range: 2000)
│   └── Damage Config (Always sense)
└── Custom Enemy Components
```

### 4.2 BP_Enemy_HeavyTechnician

#### Character Setup
```cpp
Mesh: SK_HeavyTechnician
Animation Blueprint: ABP_HeavyTech
Health: 150 (base, scaled by difficulty)
Movement Speed: 450 (slower)
Poise: 150 (high stability)
```

#### Behavior Tree
```
Root
├── Selector
│   ├── Sequence [Shield Available]
│   │   ├── Check Shield Cooldown
│   │   ├── Deploy Shield
│   │   └── Wait 3 seconds
│   ├── Sequence [Under Pressure]
│   │   ├── Health < 50%
│   │   ├── Activate Emergency Protocols
│   │   └── Aggressive Push
│   └── Sequence [Default Combat]
│       ├── Approach Player (800 units)
│       ├── Random Select
│       │   ├── Heavy Attack (30%)
│       │   ├── Block Stance (40%)
│       │   └── Basic Combo (30%)
│       └── Cooldown (0.5-1.5s)
```

#### Unique Abilities
1. **Shield Deploy**: Creates frontal barrier (100 HP)
2. **Emergency Repair**: Heals 30 HP over 3s
3. **Overcharge**: Next attack deals 2x damage

### 4.3 BP_Enemy_CorruptedMedic

#### Character Setup
```cpp
Mesh: SK_CorruptedMedic
Animation Blueprint: ABP_Medic
Health: 100 (base)
Movement Speed: 550
Poise: 75
```

#### Behavior Tree
```
Root
├── Selector
│   ├── Sequence [Self Heal]
│   │   ├── Health < 30%
│   │   ├── Retreat to safe distance
│   │   ├── Inject healing stim
│   │   └── Heal 40 HP
│   ├── Sequence [Apply Poison]
│   │   ├── Player in melee range
│   │   ├── Poison strike
│   │   └── Apply DOT (3 damage/sec for 5s)
│   └── Sequence [Ranged Combat]
│       ├── Maintain distance (1000-1500)
│       ├── Throw syringes
│       └── Dodge if player approaches
```

#### Unique Abilities
1. **Healing Stim**: Instant 40 HP heal (30s cooldown)
2. **Poison Cloud**: AOE poison zone
3. **Adrenaline Rush**: +50% speed for 3s

### 4.4 BP_Enemy_SecurityChief

#### Character Setup
```cpp
Mesh: SK_SecurityChief
Animation Blueprint: ABP_Security
Health: 120 (base)
Movement Speed: 600
Poise: 100
```

#### Behavior Tree
```
Root
├── Selector
│   ├── Sequence [Combo Assault]
│   │   ├── Player stunned/vulnerable
│   │   ├── Execute combo chain
│   │   └── Heavy finisher
│   ├── Sequence [Suppressing Fire]
│   │   ├── Player at range
│   │   ├── Draw sidearm
│   │   ├── Fire 3 shots
│   │   └── Holster weapon
│   └── Sequence [Aggressive Melee]
│       ├── Close distance rapidly
│       ├── Mix attacks and feints
│       └── Punish player blocks
```

#### Unique Abilities
1. **Triple Strike**: 3-hit combo with stagger
2. **Tactical Roll**: Dash with i-frames
3. **Flashbang**: Blind player for 2s

### 4.5 BP_Enemy_StationCommander

#### Character Setup
```cpp
Mesh: SK_Commander
Animation Blueprint: ABP_Commander
Health: 200 (base)
Movement Speed: 500
Poise: 125
```

#### Multi-Phase Boss Fight
```
Phase 1 (100-66% HP):
- Standard combat patterns
- Occasional special abilities

Phase 2 (66-33% HP):
- Summons turret support
- Increased aggression
- Orbital strike markers appear

Phase 3 (33-0% HP):
- Berserker mode activated
- All abilities enhanced
- Station integrity drains faster
```

#### Behavior Tree
```
Root
├── Selector
│   ├── Sequence [Phase Transition]
│   │   ├── Check phase threshold
│   │   ├── Invulnerability period
│   │   ├── Dramatic animation
│   │   └── Activate phase abilities
│   ├── Sequence [Orbital Strike]
│   │   ├── Mark target zones
│   │   ├── Warning period (2s)
│   │   ├── Strike damage (80)
│   │   └── Cooldown (20s)
│   └── Sequence [Command Presence]
│       ├── Rally cry (buff self)
│       ├── Calculated attacks
│       └── Defensive repositioning
```

### 4.6 BP_Enemy_MaintenanceDrone

#### Character Setup
```cpp
Mesh: SK_MaintenanceDrone
Animation Blueprint: ABP_Drone
Health: 80 (base)
Movement Speed: 700 (fastest)
Poise: 50 (lowest)
```

#### Behavior Tree
```
Root
├── Selector
│   ├── Sequence [Environmental Usage]
│   │   ├── Scan for interactables
│   │   ├── Move to nearest
│   │   ├── Activate hazard
│   │   └── Retreat to safety
│   ├── Sequence [Hit and Run]
│   │   ├── Dash in
│   │   ├── Quick attack
│   │   ├── Dash out
│   │   └── Reposition
│   └── Sequence [Evasive Maneuvers]
│       ├── Detect incoming attack
│       ├── Dodge roll
│       └── Counter opportunity
```

#### Unique Abilities
1. **System Hack**: Disables player ability (3s)
2. **Repair Drone**: Summons healing drone
3. **Overclock**: All actions 2x speed (5s)

---

## Part 5: Integration & Polish

### 5.1 GameMode Integration

#### AAtlasGameMode Implementation
```cpp
class AAtlasGameMode : public AGameModeBase
{
protected:
    UPROPERTY()
    URunManagerComponent* RunManager;
    
    UPROPERTY()
    URewardSelectionComponent* RewardSelection;
    
    UPROPERTY()
    USaveManagerSubsystem* SaveManager;

public:
    virtual void BeginPlay() override
    {
        // Initialize systems
        RunManager = NewObject<URunManagerComponent>(this);
        RewardSelection = NewObject<URewardSelectionComponent>(this);
        
        // Get save manager
        SaveManager = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
        
        // Bind events
        RunManager->OnRoomCompleted.AddDynamic(this, &AAtlasGameMode::OnRoomCompleted);
        RunManager->OnRunCompleted.AddDynamic(this, &AAtlasGameMode::OnRunCompleted);
        RewardSelection->OnSelectionComplete.AddDynamic(this, &AAtlasGameMode::OnRewardSelected);
    }
    
    void StartNewRun()
    {
        // Load persistent rewards
        if (APlayerCharacter* Player = GetPlayer())
        {
            USlotManagerComponent* SlotManager = Player->GetSlotManager();
            SaveManager->LoadRewards(SlotManager);
        }
        
        // Initialize run
        RunManager->StartNewRun();
    }
    
    void OnEnemyDefeated()
    {
        // Get room rewards
        URoomDataAsset* CurrentRoom = RunManager->GetCurrentRoom();
        TArray<URewardDataAsset*> Rewards = CurrentRoom->SelectRandomRewards(2, RunManager->GetCurrentLevel());
        
        // Present selection
        RewardSelection->PresentRewardChoice(Rewards);
    }
    
    void OnRewardSelected(ERewardSelectionResult Result)
    {
        if (Result == ERewardSelectionResult::Selected)
        {
            // Save progress
            SaveManager->SaveGame();
            
            // Complete room
            RunManager->CompleteCurrentRoom();
            
            // Transition or complete run
            if (RunManager->IsRunComplete())
            {
                CompleteRun(true);
            }
            else
            {
                RunManager->TransitionToNextRoom();
            }
        }
    }
};
```

### 5.2 Audio Implementation

#### Sound Cue Setup
```
SC_Combat_Music
├── Random Node
│   ├── Wave: Music_Engineering_Combat
│   ├── Wave: Music_Medical_Combat
│   ├── Wave: Music_Weapons_Combat
│   ├── Wave: Music_Command_Combat
│   └── Wave: Music_Maintenance_Combat

SC_Reward_Selection
├── Wave: UI_Reward_Appear
├── Delay (0.3s)
└── Wave: UI_Selection_Fanfare

SC_Room_Transition
├── Wave: Transition_Fade_Out
├── Delay (1.0s)
└── Wave: Transition_Fade_In
```

### 5.3 Visual Effects

#### Material Setup
```hlsl
// M_Reward_Glow
float Time = GetTime();
float Pulse = sin(Time * 2.0) * 0.5 + 0.5;
EmissiveColor = BaseColor * (1.0 + Pulse * 2.0);
```

#### Particle Systems
```
P_Reward_Equipped
- Burst: 50 particles
- Lifetime: 1.5s
- Velocity: Radial outward
- Color: Based on category
- Size over life: Grow then shrink
```

### 5.4 Performance Optimization

#### LOD Settings
```cpp
Mesh LODs:
- LOD0: Full detail (0-1000 units)
- LOD1: 50% triangles (1000-2000)
- LOD2: 25% triangles (2000-4000)
- LOD3: 10% triangles (4000+)

Animation LODs:
- Update rate based on distance
- Disable cloth sim beyond 2000 units
- Reduce bone count at distance
```

#### Memory Management
```cpp
// Pool frequently spawned objects
TArray<AProjectile*> ProjectilePool;
TArray<UParticleSystemComponent*> ParticlePool;

// Preload critical assets
void PreloadAssets()
{
    for (URoomDataAsset* Room : AllRooms)
    {
        Room->RoomLevel.LoadSynchronous();
        Room->UniqueEnemy.LoadSynchronous();
    }
}
```

---

## Part 6: Testing Procedures

### 6.1 Automated Testing

#### Unit Tests
```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRewardSystemTest, "Atlas.Rewards.Stacking", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FRewardSystemTest::RunTest(const FString& Parameters)
{
    // Test reward stacking
    USlotManagerComponent* SlotManager = NewObject<USlotManagerComponent>();
    URewardDataAsset* TestReward = NewObject<URewardDataAsset>();
    
    TestTrue("Equip first instance", SlotManager->EquipReward(TestReward, 0));
    TestTrue("Enhance second instance", SlotManager->EnhanceReward(TestReward->RewardTag));
    TestEqual("Stack level is 2", SlotManager->GetRewardStackLevel(0), 2);
    
    return true;
}
```

### 6.2 Manual Testing Checklist

#### Reward System
- [ ] Each reward equips correctly
- [ ] Stacking increases effects
- [ ] Slot swapping works
- [ ] Persistence between runs
- [ ] UI updates properly

#### Room Progression
- [ ] 5 rooms complete in order
- [ ] Each enemy spawns correctly
- [ ] Hazards apply properly
- [ ] Transitions are smooth
- [ ] Save/load mid-run works

#### Combat Balance
- [ ] Damage values correct
- [ ] Enemy scaling appropriate
- [ ] Abilities trigger properly
- [ ] Death states work
- [ ] Station integrity drains correctly

### 6.3 Performance Metrics

Target Performance:
- 60+ FPS at 1080p (GTX 1060)
- < 100ms input latency
- < 2GB RAM usage
- < 3 second level load times

---

## Success Criteria

Phase 3 is complete when:
1. ✅ All 25 rewards function correctly
2. ✅ 5 unique rooms with enemies
3. ✅ Complete UI flow implemented
4. ✅ Save/load system reliable
5. ✅ 60+ FPS maintained
6. ✅ No critical bugs
7. ✅ Playtest feedback positive
8. ✅ 30-minute run completion possible