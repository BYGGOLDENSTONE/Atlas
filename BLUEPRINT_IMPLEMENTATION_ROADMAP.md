# Blueprint Implementation Roadmap
**Engine**: UE 5.5 | **Date**: 2025-01-22

## Overview
This roadmap details the step-by-step blueprint implementation needed to bring Atlas's C++ framework to life in Unreal Engine. Each phase builds upon the previous, ensuring a systematic and testable implementation.

---

## Phase 1: Core Foundation Setup
**Goal**: Establish the fundamental game structure and verify C++ components work in-engine.
**Duration**: 1-2 days

### 1.1 Project Configuration
- [ ] **Project Settings**
  - Enable Enhanced Input System
  - Set default GameMode to BP_AtlasGameMode
  - Configure collision profiles (Player, Enemy, Interactable, Hazard)
  - Set up GameplayTag categories in Project Settings

### 1.2 GameMode Blueprint
- [ ] **BP_AtlasGameMode** (Parent: AAtlasGameMode)
  - Verify console commands work (`Atlas.ShowIntegrityStatus`)
  - Set default pawn class to BP_PlayerCharacter
  - Set HUD class to BP_AtlasHUD
  - Configure spawn points for room transitions

### 1.3 GameInstance Blueprint
- [ ] **BP_AtlasGameInstance** (Parent: UGameInstance)
  - Initialize SaveManagerSubsystem
  - Store persistent run data
  - Handle level transitions between rooms

### 1.4 Player Character Blueprint
- [ ] **BP_PlayerCharacter** (Parent: AGameCharacterBase)
  - **Mesh Setup**:
    - Add Skeletal Mesh Component (use Mannequin for prototype)
    - Configure capsule collision (Height: 180, Radius: 42)
    - Set up camera boom and follow camera
  - **Component Verification**:
    - Confirm ActionManagerComponent present
    - Confirm HealthComponent present
    - Confirm VulnerabilityComponent present
    - Confirm SlotManagerComponent present
    - Confirm StationIntegrityComponent present
  - **Basic Movement**:
    - Set walk speed: 600
    - Set jump velocity: 420
    - Enable air control: 0.2

### 1.5 Input Configuration
- [ ] **IMC_PlayerContext** (Input Mapping Context)
  - Movement: WASD / Left Stick
  - Camera: Mouse / Right Stick
  - Jump: Spacebar / A Button
  - Basic Attack: LMB / X Button
  - Heavy Attack: RMB / Y Button
  - Block: Q / LB
  - Dash: Shift / RB
  - Soul Attack: E / RT
  - Focus Mode: F / LT
  - Interact: E / Square

- [ ] **IA_PlayerInput** (Input Actions)
  - Create all input actions with proper value types
  - Link to PlayerController blueprint

### 1.6 Basic HUD
- [ ] **BP_AtlasHUD** (Parent: AHUD)
  - Display player health (0-100)
  - Display station integrity (0-100)
  - Show equipped slots count
  - Current room indicator (1-5)

### Validation Checklist
- [ ] Player spawns and moves correctly
- [ ] Console commands work (`Atlas.ShowIntegrityStatus`)
- [ ] Health and integrity display on HUD
- [ ] Input responds (even if actions don't work yet)

---

## Phase 2: Level Architecture & Room System
**Goal**: Create the 5 unique room environments and transition system.
**Duration**: 2-3 days

### 2.1 Master Level Setup
- [ ] **L_Atlas_Master** (Persistent Level)
  - Central hub area for room loading
  - Room transition volumes
  - Lighting: Space station emergency lighting
  - Post-process volume for space atmosphere

### 2.2 Room Blueprints
- [ ] **BP_RoomBase** (Base room actor)
  - Room bounds definition
  - Enemy spawn point
  - Player spawn point
  - Reward spawn point
  - Hazard spawn points (3-5 per room)
  - Interactable spawn points (2-3 per room)
  - Exit trigger volume

### 2.3 Individual Room Maps (Streaming Levels)

#### L_Room_EngineeringBay
- [ ] **Theme**: Industrial, pipes, machinery
- [ ] **Layout**: 20x20m square with elevated platforms
- [ ] **Unique Features**:
  - Steam vents (hazard points)
  - Explosive barrels placement
  - Destructible pipes
  - Flickering lights
- [ ] **Cover**: Machinery, crates, workbenches

#### L_Room_CombatArena
- [ ] **Theme**: Gladiatorial pit
- [ ] **Layout**: 25x25m circular arena
- [ ] **Unique Features**:
  - Minimal cover (pillars only)
  - Wall spikes for impact damage
  - Crowd viewing areas (aesthetic)
  - Central lowered fighting pit
- [ ] **Cover**: 4 pillars strategically placed

#### L_Room_MedicalBay
- [ ] **Theme**: Sterile, clinical
- [ ] **Layout**: 15x20m with corridors
- [ ] **Unique Features**:
  - Medical beds (destructible)
  - Chemical hazard areas
  - Emergency medical stations (interactables)
  - Glass windows (breakable)
- [ ] **Cover**: Medical equipment, dividers

#### L_Room_CargoHold
- [ ] **Theme**: Storage, cluttered
- [ ] **Layout**: 30x20m with stacked containers
- [ ] **Unique Features**:
  - Shipping containers (climbable)
  - Crane system (potential hazard)
  - Cargo nets (physics objects)
  - Multiple elevation levels
- [ ] **Cover**: Abundant - crates, containers

#### L_Room_Bridge
- [ ] **Theme**: Command center
- [ ] **Layout**: 25x15m multi-level
- [ ] **Unique Features**:
  - Captain's chair (central platform)
  - Holographic displays
  - Window to space (breakable = instant death)
  - Two-tier layout with stairs
- [ ] **Cover**: Console stations, railings

### 2.4 Room Transition System
- [ ] **BP_RoomManager** (Manages room flow)
  - Track current room (1-5)
  - Handle level streaming
  - Randomize room order
  - Trigger reward selection after combat
  - Progress to next room

### Validation Checklist
- [ ] All 5 rooms load correctly
- [ ] Player spawns at correct position
- [ ] Room transitions work
- [ ] Each room has unique visual identity
- [ ] Performance acceptable (60+ FPS)

---

## Phase 3: Combat & Animation System
**Goal**: Implement full combat system with proper animation timing.
**Duration**: 3-4 days

### 3.1 Animation Blueprint Setup
- [ ] **ABP_PlayerCharacter**
  - **State Machine**:
    - Idle/Walk/Run blend space
    - Jump states (Start, Loop, Land)
    - Combat stance toggle
    - Hit reaction states
    - Death state
  - **Montage Slots**:
    - UpperBody (attacks while moving)
    - FullBody (heavy attacks, staggers)

### 3.2 Combat Animation Montages
- [ ] **AM_BasicAttack** (0.5s duration)
  - Hit notify at 0.2s
  - Recovery at 0.4s
  - Can be interrupted after 0.3s

- [ ] **AM_HeavyAttack** (1.2s duration)
  - Wind-up: 0-0.4s
  - Hit notify at 0.6s
  - Knockback force applied
  - Cannot be interrupted

- [ ] **AM_Block** (Pose)
  - Instant activation
  - Hold pose while input held
  - 40% damage reduction

- [ ] **AM_Parry** (0.5s duration)
  - Perfect window: 0-0.2s (100% reduction)
  - Late window: 0.2-0.3s (50% reduction)
  - Vulnerability: 0.3-0.5s (2x damage taken)

- [ ] **AM_SoulAttack** (1.5s duration)
  - Charge-up VFX: 0-0.5s
  - Unblockable hit at 0.8s
  - 50 damage, bypasses integrity

- [ ] **AM_Dash** (0.4s duration)
  - I-frames: 0.1-0.3s
  - 400 unit displacement
  - 4-directional based on input

- [ ] **AM_Stagger** (2.0s duration)
  - Triggered at 0 poise
  - Complete vulnerability
  - Cannot act

- [ ] **AM_HitReaction_Light** (0.3s)
- [ ] **AM_HitReaction_Heavy** (0.6s)
- [ ] **AM_Death** (2.0s)

### 3.3 Animation Notifies
- [ ] **AN_DealDamage**
  - Trigger damage calculation
  - Check hit targets
  - Apply vulnerability modifiers

- [ ] **AN_ParryWindow**
  - Start/End perfect parry window
  - Start/End late parry window

- [ ] **AN_CanCombo**
  - Allow action chaining

- [ ] **AN_ApplyKnockback**
  - Apply physics impulse
  - Check wall collision

### 3.4 Combat VFX
- [ ] **P_BasicHit** (Spark on impact)
- [ ] **P_HeavyHit** (Larger impact with screen shake)
- [ ] **P_Blocked** (Shield shimmer effect)
- [ ] **P_Parry** (Flash and deflection effect)
- [ ] **P_SoulAttack** (Energy buildup and blast)
- [ ] **P_Vulnerability** (Red glow on vulnerable target)
- [ ] **P_Stagger** (Stars above head)

### 3.5 Combat Audio Setup
- [ ] **Sound Cues**:
  - SC_Attack_Swing (whoosh)
  - SC_Attack_Hit (impact)
  - SC_Block_Success (shield sound)
  - SC_Parry_Perfect (ring sound)
  - SC_Stagger (daze sound)
  - SC_SoulAttack_Charge (energy buildup)

### Validation Checklist
- [ ] All attacks deal correct damage (Basic: 5, Heavy: 15, Soul: 50)
- [ ] Block reduces damage by 40%
- [ ] Parry windows work correctly
- [ ] Vulnerability multipliers apply (2x, 4x, 8x)
- [ ] Poise system causes stagger at 0
- [ ] Animation notifies trigger at correct times
- [ ] Combat feels responsive

---

## Phase 4: Data Assets & Reward System
**Goal**: Create all reward and room data assets, implement reward selection flow.
**Duration**: 2-3 days

### 4.1 ActionDataAssets Creation
- [ ] **Basic Actions** (Already available to all)
  - DA_BasicAttack
  - DA_HeavyAttack
  - DA_Block
  - DA_Dash
  - DA_SoulAttack

### 4.2 RewardDataAssets (25 Total)

#### Defense Category (5)
- [ ] **DA_Reward_IronSkin**
  - 25% damage reduction
  - Tier 2: 40%, Tier 3: 50%
  
- [ ] **DA_Reward_ReactiveArmor**
  - Reflect 10% damage to attacker
  - Tier 2: 20%, Tier 3: 30%

- [ ] **DA_Reward_ShieldBoost**
  - Block reduces 60% damage (up from 40%)
  - Tier 2: 70%, Tier 3: 80%

- [ ] **DA_Reward_Evasion**
  - 15% dodge chance
  - Tier 2: 25%, Tier 3: 35%

- [ ] **DA_Reward_DamageReduction**
  - Flat -5 damage from all sources
  - Tier 2: -8, Tier 3: -12

#### Offense Category (5)
- [ ] **DA_Reward_Berserker**
  - +20% attack speed
  - Tier 2: +35%, Tier 3: +50%

- [ ] **DA_Reward_CriticalStrikes**
  - 20% chance for 2x damage
  - Tier 2: 30% chance, Tier 3: 40% chance

- [ ] **DA_Reward_DoubleStrike**
  - Attacks hit twice
  - Tier 2: +10% damage, Tier 3: +20% damage

- [ ] **DA_Reward_Bleed**
  - Attacks apply 3 damage/sec for 3s
  - Tier 2: 5 damage/sec, Tier 3: 7 damage/sec

- [ ] **DA_Reward_ArmorPiercing**
  - Ignore 30% of enemy defense
  - Tier 2: 50%, Tier 3: 70%

#### Passive Stats Category (5)
- [ ] **DA_Reward_HealthBoost**
  - +25 max health
  - Tier 2: +40, Tier 3: +60

- [ ] **DA_Reward_PoiseBoost**
  - +30 max poise
  - Tier 2: +50, Tier 3: +70

- [ ] **DA_Reward_StaminaBoost**
  - -20% ability cooldowns
  - Tier 2: -35%, Tier 3: -50%

- [ ] **DA_Reward_SpeedBoost**
  - +15% movement speed
  - Tier 2: +25%, Tier 3: +35%

- [ ] **DA_Reward_CooldownReduction**
  - -1s on all cooldowns
  - Tier 2: -1.5s, Tier 3: -2s

#### Passive Abilities Category (5)
- [ ] **DA_Reward_Lifesteal**
  - Heal 10% of damage dealt
  - Tier 2: 20%, Tier 3: 30%

- [ ] **DA_Reward_Thorns**
  - Deal 5 damage to melee attackers
  - Tier 2: 10 damage, Tier 3: 15 damage

- [ ] **DA_Reward_Regeneration**
  - Heal 2 HP/sec out of combat
  - Tier 2: 4 HP/sec, Tier 3: 6 HP/sec

- [ ] **DA_Reward_CounterStrike**
  - Successful parries deal 20 damage
  - Tier 2: 35 damage, Tier 3: 50 damage

- [ ] **DA_Reward_LastStand**
  - At 25% health, gain 2x damage
  - Tier 2: At 35%, Tier 3: At 45%

#### Interactables Category (5)
- [ ] **DA_Reward_ExplosiveBarrel**
  - Hack barrels to explode (30 damage AOE)
  - Tier 2: 45 damage, Tier 3: 60 damage

- [ ] **DA_Reward_GravityWell**
  - Create gravity anomaly pulling enemies
  - Tier 2: Stronger pull, Tier 3: Damages trapped enemies

- [ ] **DA_Reward_TurretHack**
  - Hack turrets to fire at enemies
  - Tier 2: Faster fire rate, Tier 3: Explosive rounds

- [ ] **DA_Reward_PowerSurge**
  - Overload systems for area stun
  - Tier 2: Longer stun, Tier 3: Damage + stun

- [ ] **DA_Reward_EmergencyVent**
  - Open vents to push enemies
  - Tier 2: Further push, Tier 3: Push + damage

### 4.3 RoomDataAssets (5 Total)
- [ ] **DA_Room_EngineeringBay**
  - Enemy: BP_Enemy_Mechanic
  - Reward Pool: Defense + Interactables

- [ ] **DA_Room_CombatArena**
  - Enemy: BP_Enemy_Gladiator
  - Reward Pool: Offense + Defense

- [ ] **DA_Room_MedicalBay**
  - Enemy: BP_Enemy_Medic
  - Reward Pool: Passive Stats + Passive Abilities

- [ ] **DA_Room_CargoHold**
  - Enemy: BP_Enemy_Brute
  - Reward Pool: Offense + Passive Stats

- [ ] **DA_Room_Bridge**
  - Enemy: BP_Enemy_Captain
  - Reward Pool: All categories

### 4.4 Reward Selection UI
- [ ] **WBP_RewardSelection** (Parent: URewardSelectionWidget)
  - Two reward cards side by side
  - Show name, description, current tier
  - Highlight on hover
  - Click to select
  - Fade in/out animations

- [ ] **WBP_RewardCard**
  - Icon display
  - Name text
  - Description text
  - Tier indicator (1-3 stars)
  - Rarity border color

### Validation Checklist
- [ ] All 25 rewards created and configured
- [ ] Reward selection UI appears after combat
- [ ] Rewards properly slot into SlotManager
- [ ] Enhancement works when selecting same reward
- [ ] Console command `Atlas.Phase3.ListRewards` shows all rewards

---

## Phase 5: Enemy AI Implementation
**Goal**: Create 5 unique enemy types with adaptive difficulty.
**Duration**: 3-4 days

### 5.1 Base Enemy Setup
- [ ] **BP_EnemyBase** (Parent: AEnemyCharacter)
  - AI Controller setup
  - Behavior Tree reference
  - Perception component (sight, hearing)
  - Same components as player (ActionManager, Health, etc.)

### 5.2 Enemy Blueprints

#### BP_Enemy_Mechanic
- [ ] **Visual**: Worker outfit, wrench weapon
- [ ] **Stats**: 
  - Health: 80
  - Preferred Range: Medium
- [ ] **AI Priority**:
  - Favor defensive abilities
  - Use interactables frequently
  - Retreat when low health
- [ ] **Unique Behavior**: Attempts to repair hazards

#### BP_Enemy_Gladiator
- [ ] **Visual**: Armored, sword and shield
- [ ] **Stats**:
  - Health: 120
  - Preferred Range: Melee
- [ ] **AI Priority**:
  - Aggressive melee combat
  - Frequent parry attempts
  - Chain combos
- [ ] **Unique Behavior**: Enters rage at 50% health

#### BP_Enemy_Medic
- [ ] **Visual**: Medical gear, syringe weapon
- [ ] **Stats**:
  - Health: 60
  - Preferred Range: Long
- [ ] **AI Priority**:
  - Keep distance
  - Use passive abilities
  - Heal self when possible
- [ ] **Unique Behavior**: Can heal to full once per fight

#### BP_Enemy_Brute
- [ ] **Visual**: Large, heavy armor, hammer
- [ ] **Stats**:
  - Health: 150
  - Preferred Range: Melee
- [ ] **AI Priority**:
  - Heavy attacks
  - Ignore defense
  - Maximum aggression
- [ ] **Unique Behavior**: Immune to stagger at >50% health

#### BP_Enemy_Captain
- [ ] **Visual**: Officer uniform, energy sword
- [ ] **Stats**:
  - Health: 100
  - Preferred Range: Adaptive
- [ ] **AI Priority**:
  - Balanced approach
  - Uses all ability types
  - Adapts to player pattern
- [ ] **Unique Behavior**: Copies player's most-used ability

### 5.3 AI Behavior Trees
- [ ] **BT_EnemyBase**
  - **Root**: Selector
  - **Services**: 
    - Update target location
    - Check ability cooldowns
    - Analyze player patterns
  - **Main Branches**:
    1. Combat (when in range)
    2. Approach (when far)
    3. Retreat (when low health)
    4. Use Interactable (when available)

- [ ] **Combat Subtree**:
  - Check if should parry (player attacking)
  - Check if should block (under pressure)
  - Check if can combo
  - Select ability based on priority
  - Execute ability

### 5.4 Adaptive Difficulty System
- [ ] **Enemy Power Scaling**:
  - Count player's equipped slots
  - Enemy abilities = player slots + 1
  - Randomly assign abilities from reward pool
  - Higher tier rewards at higher levels

- [ ] **Pattern Analysis** (AIDifficultyComponent):
  - Track player's last 10 actions
  - Identify most common pattern
  - Increase chance to counter
  - Reset on new run

### 5.5 Enemy VFX
- [ ] **P_Enemy_Spawn** (Teleport/materialize effect)
- [ ] **P_Enemy_Death** (Disintegration)
- [ ] **P_Enemy_Enrage** (Red aura for gladiator)
- [ ] **P_Enemy_Heal** (Green effect for medic)
- [ ] **P_Enemy_Block** (Shield effect)

### Validation Checklist
- [ ] Each enemy has unique appearance
- [ ] AI uses abilities appropriately
- [ ] Difficulty scales with player power
- [ ] Enemies feel distinct to fight
- [ ] Pattern adaptation works
- [ ] Console command `Atlas.Phase3.SpawnEnemy [Type]` works

---

## Phase 6: UI System & Polish
**Goal**: Complete UI implementation and run flow.
**Duration**: 2-3 days

### 6.1 Main Menu
- [ ] **WBP_MainMenu**
  - Start New Run button
  - Continue Run button (if save exists)
  - Settings button
  - Quit button
  - Background: Space station exterior

### 6.2 HUD System
- [ ] **WBP_GameHUD** (Complete HUD)
  - **Health Bar**:
    - Current/Max display
    - Color change at low health
    - Damage flash effect
  
  - **Station Integrity Bar**:
    - Current/Max display
    - Warning at <25%
    - Shake effect on damage
  
  - **Equipped Slots Display**:
    - 6 slot icons
    - Show equipped rewards
    - Cooldown timers
    - Highlight when available
  
  - **Room Progress**:
    - Rooms 1-5 indicator
    - Current room highlighted
    - Completed rooms marked

### 6.3 Slot Management UI
- [ ] **WBP_SlotManager** (Parent: USlotManagerWidget)
  - 6 slot grid layout
  - Drag and drop support
  - Right-click context menu
  - Slot swapping
  - Enhancement indicators (Tier 1-3)
  - Empty slot placeholder

### 6.4 Run Progress UI
- [ ] **WBP_RunProgress** (Parent: URunProgressWidget)
  - Room 1-5 path visual
  - Current position marker
  - Room preview on hover
  - Enemy portrait for current room
  - Potential rewards preview

### 6.5 Death/Victory Screens
- [ ] **WBP_RunComplete**
  - Victory: "Run Complete!"
  - Death: Show cause (health/integrity)
  - Stats: Rooms cleared, damage dealt, rewards collected
  - Continue button (save equipped rewards)
  - Main menu button

### 6.6 Environmental UI
- [ ] **WBP_InteractPrompt**
  - "Press E to Interact"
  - Show interaction type
  - Progress bar for timed interactions

- [ ] **WBP_DamageNumbers**
  - Floating combat text
  - Different colors for damage types
  - Critical hit emphasis
  - Vulnerability multiplier display

### 6.7 Settings Menu
- [ ] **WBP_Settings**
  - Graphics quality
  - Resolution
  - Audio sliders
  - Control remapping
  - Return to game

### 6.8 Visual Polish
- [ ] **Post-Processing Effects**:
  - Low health screen edge redness
  - Stagger blur effect
  - Soul attack charge distortion
  - Station damage screen shake

- [ ] **Environmental Effects**:
  - Emergency lights at low integrity
  - Sparks from damaged areas
  - Steam leaks
  - Debris physics

### 6.9 Audio Polish
- [ ] **Music**:
  - Menu theme
  - Combat music (intensity scaling)
  - Victory stinger
  - Death theme

- [ ] **Ambience**:
  - Station hum
  - Distant alarms
  - Creaking metal at low integrity

### Validation Checklist
- [ ] Complete run flow works (menu → game → victory/death → menu)
- [ ] All UI elements display correctly
- [ ] Rewards persist between runs
- [ ] Save/load system works
- [ ] Visual feedback for all actions
- [ ] Performance maintains 60+ FPS

---

## Testing Protocol

### After Each Phase
1. **Functional Testing**:
   - Test all new features
   - Verify console commands work
   - Check for crashes/errors

2. **Integration Testing**:
   - Ensure new phase works with previous phases
   - Test complete gameplay loop
   - Verify data persistence

3. **Performance Testing**:
   - Monitor FPS in each room
   - Check memory usage
   - Profile expensive operations

### Final Testing Checklist
- [ ] Complete 5 full runs without crashes
- [ ] All 25 rewards functional
- [ ] All 5 enemies have unique behavior
- [ ] Save system persists between sessions
- [ ] All console commands work
- [ ] Achieve 60+ FPS consistently
- [ ] UI scales properly at different resolutions

---

## Common Issues & Solutions

### Issue: Components not found in Blueprint
**Solution**: Ensure C++ classes are compiled, regenerate Blueprint

### Issue: Animations not playing
**Solution**: Check montage slot names match Animation Blueprint

### Issue: AI not using abilities
**Solution**: Verify AIDifficultyComponent is assigning abilities

### Issue: Rewards not persisting
**Solution**: Check SaveManagerSubsystem is initialized in GameInstance

### Issue: Room transitions failing
**Solution**: Verify level streaming setup and spawn points

### Issue: Console commands not working
**Solution**: Ensure GameMode blueprint inherits from AAtlasGameMode

---

## Resource Requirements

### Assets Needed
- **Character Models**: Player, 5 enemy types
- **Animations**: Full combat set for each character
- **Environment**: 5 unique room meshes and props
- **VFX**: 20+ particle effects
- **Audio**: 30+ sound effects, 5+ music tracks
- **UI**: Icons for 25 rewards

### Technical Requirements
- **Unreal Engine**: 5.5
- **RAM**: 16GB minimum
- **GPU**: GTX 1060 or better
- **Storage**: 50GB free space

---

## Quick Reference Commands
```
# Testing Phase 1
Atlas.ShowIntegrityStatus
Atlas.DamageIntegrity 10

# Testing Phase 3
Atlas.Phase3.ListRewards
Atlas.Phase3.GiveReward Berserker
Atlas.Phase3.ShowSlots

# Testing Phase 4
Atlas.Phase3.SpawnEnemy Gladiator
Atlas.Phase3.StartRun

# Testing Phase 5
Atlas.Phase4.TestAll
Atlas.Phase4.SpawnHazard Electrical

# Testing Phase 6
Atlas.Phase3.QuickTest UI
```

---

## Next Steps After Blueprint Implementation
1. **Phase 5 (Code)**: Performance optimization
2. **Phase 6 (Code)**: Audio system implementation
3. **Phase 7 (Code)**: Meta progression systems
4. **Phase 8 (Code)**: Platform integration
5. **Phase 9**: Final polish and release

This completes the Blueprint implementation requirements for Atlas's Phase 1-4 C++ systems.