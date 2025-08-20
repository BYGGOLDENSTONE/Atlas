# Atlas Development Changelog

## Session: 2025-01-20 - P12 Station Integrity System

### Changes
- **Phase P12 - Station Integrity System Completed**
  - Core risk/reward mechanic: powerful abilities damage the station
  - Both player AND enemies can destroy the station (game over at 0%)
  - Three integrity thresholds:
    - **100%**: Station stable, all systems normal
    - **50%**: Critical warning threshold (future environmental effects)
    - **0%**: Station destroyed, game over, run resets
  - High-risk abilities apply integrity damage when used
  - No blocking of abilities - player must manage the risk

### Technical Implementation
- **New Components**:
  - `StationIntegrityComponent`: Tracks station health with events
  - `AtlasGameState`: Holds integrity component for global access
  - `AtlasGameInstance`: Handles game over states with different reasons
  - `StationIntegrityDataAsset`: Configurable integrity costs per ability

- **Integration**:
  - CombatComponent checks and applies integrity costs on ability use
  - GameMode configured to use custom GameState
  - Added gameplay tags for risk levels (High/Medium/Low)
  - Console commands for testing and debugging

- **Console Commands Added**:
  - `Atlas.DamageIntegrity [amount]` - Apply integrity damage
  - `Atlas.SetIntegrityPercent [percent]` - Set specific percentage
  - `Atlas.ShowIntegrityStatus` - Display current status
  - `Atlas.TestHighRiskAbility [cost]` - Test ability with cost
  - `Atlas.ResetIntegrity` - Reset to 100%

### Files Created
- Source/Atlas/Components/StationIntegrityComponent.h/cpp
- Source/Atlas/Core/AtlasGameState.h/cpp
- Source/Atlas/Core/AtlasGameInstance.h/cpp
- Source/Atlas/Data/StationIntegrityDataAsset.h/cpp
- Source/Atlas/Debug/StationIntegrityDebugCommands.h/cpp

### Files Modified
- AtlasGameMode.cpp - Set custom GameState class
- CombatComponent.cpp - Apply integrity costs on abilities
- AtlasGameplayTags.h/cpp - Added risk level tags
- Atlas.cpp - Registered debug commands
- CLAUDE.md - Updated documentation

### Next Steps
- P13: Add environmental effects at 50% threshold
- Create UI for station integrity display
- Implement visual warnings for critical state

## Hotfix: 2025-01-20 - Station Integrity Debug Commands

### Fixed
- Console commands now properly detect game world context
- Commands only work during Play In Editor (PIE) or gameplay
- Fixed "No world was found" error when using commands
- All debug commands now use proper world context iteration

### Verified Working Commands
- `Atlas.ShowIntegrityStatus` - Displays current integrity state
- `Atlas.DamageIntegrity [amount]` - Damages station
- `Atlas.SetIntegrityPercent [percent]` - Sets specific percentage
- `Atlas.TestHighRiskAbility [cost]` - Tests ability with integrity cost
- `Atlas.ResetIntegrity` - Resets to 100%

### Technical Details
- Updated StationIntegrityDebugCommands.cpp to iterate through world contexts
- Properly detects PIE and Game world types
- Clear error messages when not in play mode

## Session: 2025-01-20 - P10 & P11 Completion

### Changes
- **Phase P10 - Advanced AI Completed**
  - BTTask_SoulAttack: Unblockable special with 50 damage, 10s cooldown
  - BTTask_CatchSpecial: Gap closer for anti-kiting (800 unit range)
  - BTTask_MeleeAttack: Full combo system with 3-hit chains
  - BTTask_DefendBlock: Defensive AI behavior
  - EnemyAIController: Perception and behavior tree integration
  - Note: Enemy dash deferred to future update
  - Basic AI sufficient for testing combat mechanics

- **Phase P11 - Tiered Vulnerability System Completed**
  - Converted binary vulnerability to 3-tier system:
    - **Stunned (Yellow)**: 1 critical hit, 2x damage multiplier, 2s duration
    - **Crippled (Orange)**: 3 critical hits, 4x damage multiplier, 3s duration
    - **Exposed (Red)**: 5 critical hits, 8x damage multiplier, 4s duration
  - Added visual debug indicators per tier (colored spheres and text)
  - Updated DamageCalculator to consume hits properly
  - Modified ValveInteractable to apply Stunned tier
  - Created comprehensive debug commands for testing
  - Maintained backwards compatibility with legacy functions

### Technical Details
- **VulnerabilityComponent Refactor**:
  - Added EVulnerabilityTier enum
  - FVulnerabilityTierConfig struct for tier settings
  - OnCriticalHitLanded() replaces ConsumeCharge()
  - Dynamic visual indicators with pulsing effects
  - Per-tier color coding (Yellow/Orange/Red)

- **Debug Commands Added**:
  - Atlas.ApplyStunned - Apply yellow tier
  - Atlas.ApplyCrippled - Apply orange tier
  - Atlas.ApplyExposed - Apply red tier
  - Atlas.SimulateCritHit - Test hit consumption
  - Atlas.ShowVulnInfo - Display tier details
  - Atlas.ToggleVulnDebug - Toggle visual indicators

### Files Created
- Source/Atlas/Debug/VulnerabilityDebugCommands.h/cpp

### Files Modified
- VulnerabilityComponent.h/cpp - Complete tier system implementation
- DamageCalculator.cpp - Updated to use OnCriticalHitLanded()
- ValveInteractable.cpp - Now applies Stunned tier
- Atlas.cpp - Registered debug commands

---

## Session: 2025-01-20 - Documentation Consolidation
### Changes
- **Documentation Restructured**
  - Consolidated all game features into CLAUDE.md
  - Moved historical implementation data to changelog.md
  - Removed redundant documentation files
  - Created clear separation between current features and development history

---

## Session: 2025-01-19 - Interactables & Poise System Refactor

### Major Changes
- **Poise System Moved to HealthComponent**
  - Moved all poise management from CombatComponent to HealthComponent
  - Centralized health and stagger mechanics in one component
  - Added PlayHitReaction() for animation feedback
  - Poise regen (15/s after 1.5s delay) now handled by HealthComponent tick
  
- **Interactables Refactored to be Neutral**
  - All interactables now affect any character (player or enemy)
  - Removed ally/enemy targeting distinction
  - Vent: Despawns after hitting any character, plays hit reaction
  - Valve: Two distinct types (Vulnerability or Stagger)
  
- **Valve Simplification**
  - Removed damage dealing and effect types (Fire/Electric/Poison/Physical)
  - Simplified to two variants: Vulnerability (purple) or Stagger (yellow)
  - Removed damage-over-time mechanics
  - Single instant effect on trigger
  
- **Focus Mode Range Increased**
  - Extended from 1000 to 2000 units
  - Better interaction range for environmental objects
  
### Technical Details
- HealthComponent now manages both health and poise with proper events
- DashComponent and WallImpactComponent updated to use HealthComponent for stagger checks
- Fixed compilation errors with PlayerCharacter vs AtlasCharacter naming
- All combat components properly reference HealthComponent for poise

### Files Modified
- HealthComponent.h/cpp - Added poise system from CombatComponent
- CombatComponent.h/cpp - Removed poise system, now uses HealthComponent
- VentInteractable.cpp - Added despawn and hit reactions
- ValveInteractable.h/cpp - Simplified to two types, removed damage
- FocusModeComponent.h/cpp - Range increased to 2000
- DashComponent.cpp - Updated to check stagger via HealthComponent
- WallImpactComponent.cpp - Updated to apply poise damage via HealthComponent

---

## Session: 2025-01-19 - Wall Impact System Refactor & Debug Improvements

### Major Changes
- **Wall Impact System Complete Refactor**
  - Changed from predictive pre-checking to reactive collision-based detection
  - Now tracks enemy during knockback flight with collision sphere
  - Uses character's capsule component for more reliable collision detection
  - Distinguishes between wall impacts (vertical) and floor impacts (horizontal)
  
- **Configurable Knockback System**
  - Added KnockbackUpwardForce (0.0-1.0) for vertical lift control
  - Added RagdollKnockbackMultiplier (1.0-5.0) for heavy attack scaling
  - Added StandardKnockbackMultiplier for regular knockback tuning
  - Knockback works from ANY direction (sideways, diagonal, forward, backward)
  
- **Impact Detection Improvements**
  - Wall Impact: 2-second stagger, bounce effect, wall break VFX placeholder
  - Floor Impact: 1-second ragdoll, recovery animation placeholder
  - Surface detection: Wall (Normal.Z < 0.3), Floor (Normal.Z > 0.7)
  - Lowered detection threshold from 300 to 150 knockback force
  
- **Debug Visualization Enhancements**
  - Yellow sphere follows enemy during knockback (properly attached)
  - Magenta arrow shows knockback direction and force
  - Red sphere/effects for wall impacts
  - Green circle/sphere for floor impacts
  - Real-time position and direction text overlays
  
- **Code Cleanup**
  - Removed verbose initialization logs (input setup, dash init, etc.)
  - Removed attack window begin/end logs
  - Removed combat state change logs
  - Kept only essential error logging
  - Cleaner console output focused on gameplay events

### Technical Implementation
- **System Flow**:
  1. Heavy attack triggers with 500 knockback force
  2. Full knockback applied to enemy (ragdoll + launch)
  3. Collision sphere spawns on enemy (100 unit radius)
  4. During flight, sphere detects first collision
  5. System checks surface normal:
     - WALL HIT: 2-second stagger + bounce + wall break VFX
     - FLOOR HIT: 1-second ragdoll + get-up animation
  6. Tracking stops after impact or 2-second timeout

### Files Modified
- WallImpactComponent.h/cpp - Complete refactor for collision-based detection
- DamageCalculator.h/cpp - Added configurable knockback parameters
- PlayerCharacter.cpp - Removed verbose logging
- DashComponent.cpp - Removed debug spam
- CombatComponent.cpp - Cleaned up combat logs
- AttackNotifyState.cpp - Removed attack window logs

---

## Session: 2025-01-18 - Parry System Complete Removal

### Changes
- **Removed Parry System Entirely**
  - Deleted all parry-related files (ParryWindowNotifyState, ParryTestCommands, ParryFeedbackComponent)
  - Removed parry methods from CombatComponent (TryParry, HandleSuccessfulParry, SetAttackerParryWindow, etc.)
  - Removed parry gameplay tags from AtlasGameplayTags
  - Removed parry from CombatRulesDataAsset
  - Removed parry input from PlayerCharacter (ParryRMB)
  - Removed parry option from AI defense behavior
  - Updated all documentation to reflect parry removal

- **Simplified Combat System**
  - Block system remains intact (40% damage reduction)
  - Vulnerability system continues to work (8x damage multiplier)
  - Soul Attack now only unblockable (not unparryable)
  - Cleaner, more focused combat mechanics

### Rationale
- Parry system added unnecessary complexity
- Focus on core mechanics: attack, block, vulnerability, and stagger
- Simplifies player learning curve and AI behavior

### Files Deleted
- Source/Atlas/Animation/ParryWindowNotifyState.h/cpp
- Source/Atlas/Debug/ParryTestCommands.h/cpp
- Source/Atlas/Components/ParryFeedbackComponent.h/cpp

---

## Session: 2025-01-18 - Camera Lock System Removal & Code Cleanup

### Changes
- **Removed Camera Lock System**
  - Deleted SoftLockComponent.h and SoftLockComponent.cpp files
  - Removed SoftLockComponent from PlayerCharacter
  - Cleaned up all references in debug commands
  - Player now has full manual camera control
  
- **Code Cleanup**
  - Fixed ANY_PACKAGE deprecation warnings in EpicUnrealMCPBlueprintCommands.cpp
  - Fixed BufferSize naming conflict in MCPServerRunnable.cpp
  - Updated FocusModeDebugCommands to remove soft lock references
  
- **Documentation Updates**
  - Updated CLAUDE.md to reflect removal of camera lock system
  - Removed references to Smart Soft Lock from key mechanics
  - Added note about camera lock removal in P8 section

### Rationale
- Camera lock system was causing unwanted rotation behavior
- Hard lock implementation was too restrictive for gameplay
- Player feedback indicated preference for full manual control

---

## Session: 2025-01-17 - Phase P9 - Dash System Implementation

### Changes
- **DashComponent Created**
  - 4-directional dash based on WASD input
  - 400 unit dash distance (reduced from 600 for balance)
  - 2 second cooldown between dashes
  - I-frames during dash (0.3s invulnerability)
  - State restrictions: Cannot dash while staggered, attacking, or already dashing
  
- **Input Integration**
  - Space key triggers dash (moved block to RMB)
  - Direction determined by current movement input
  - No dash if no movement input active
  
### Implementation Details
- Dash uses LaunchCharacter for smooth movement
- I-frames handled via temporary invulnerability flag
- Cooldown prevents dash spam
- Visual feedback via debug logs (VFX placeholder)

### Files Created
- Source/Atlas/Components/DashComponent.h/cpp

---

## Session: 2025-01-17 - Phase P8 - Smart Soft Lock + Wall Impact System

### Changes
- **Combat State Detection**
  - IsInCombat() method checks active combat states
  - Tracks last combat action timestamp
  - 3-second combat memory for smoother transitions
  - Combat states: Attacking, Blocking, Parrying, Staggered
  
- **Enhanced Soft Lock Camera** (Later Removed)
  - Auto-engaged when entering combat near enemies (800 units)
  - Soft camera rotation tracked locked target
  - Player could override with strong input
  - Force-unlock with rapid camera movement (>180°/s)
  - Preserved player pitch control
  
- **Wall Impact System**
  - WallImpactComponent detects walls behind targets
  - Triggers on heavy knockback (>300 force)
  - 200 unit wall detection range
  - Wall impact effects:
    - 1.5x bonus damage multiplier
    - Extended stagger (2 seconds)
    - Bounce-back physics effect
    - Visual debug indicators
  
- **Integration Updates**
  - DamageCalculator checks for walls during knockback
  - Automatic WallImpactComponent creation if needed
  - Wall impacts reduce knockback force by 50%
  - Full integration with existing damage pipeline

### Implementation Details
- Combat auto-engage checked every tick when not in focus mode
- Soft lock interpolation speed: 2.0 (adjustable)
- Wall detection uses WorldStatic collision channel
- Wall verification: Surface must be <30° from vertical
- Extended stagger applies 100 poise damage initially

### Files Created
- Source/Atlas/Components/WallImpactComponent.h/cpp

---

## Session: 2025-01-17 - Phase P7 - Focus Mode & Interactables

### Changes
- **Focus Mode System**
  - Q key hold activation with 1000 unit range (later increased to 2000)
  - Screen-space targeting algorithm
  - Visual debugging with colored spheres
  - Smart Soft Lock integration (800 units)
  
- **FocusModeComponent**
  - Scans for IInteractable actors in range
  - Prioritizes targets closest to screen center
  - Force-unlock with rapid camera movement (>180°/s)
  - Smooth target switching with hysteresis
  
- **Interactable Framework**
  - IInteractable interface for all interactables
  - InteractableBase class with cooldown system
  - Visual state feedback (focused/cooldown)
  
- **Vent Interactable**
  - One-time use projectile
  - Launches in predetermined direction
  - Applies 50 poise damage on hit
  - Physics-based movement
  
- **Valve Interactable**
  - 500 unit AoE effect
  - Four types: Fire, Electric, Poison, Physical (later simplified)
  - 10 second cooldown
  - Optional vulnerability/stagger effects

### Debug Commands
- Atlas.ToggleFocusMode - Toggle focus mode
- Atlas.ShowFocusDebug - Visual debugging
- Atlas.SpawnTestInteractable - Spawn test objects
- Atlas.ShowFocusInfo - Display current state

### Files Created
- Source/Atlas/Components/FocusModeComponent.h/cpp
- Source/Atlas/Interfaces/Interactable.h
- Source/Atlas/Interactables/InteractableBase.h/cpp
- Source/Atlas/Interactables/VentInteractable.h/cpp
- Source/Atlas/Interactables/ValveInteractable.h/cpp

---

## Session: 2025-01-16 - Phase P6 - Stagger System + Light Poise

### Changes
- **Poise System**
  - MaxPoise: 100, CurrentPoise tracking
  - TakePoiseDamage() reduces poise
  - Poise regeneration: 15/s after 1.5s delay
  - Automatic stagger on poise break
  
- **Stagger Mechanics**
  - Combat.State.Staggered tag management
  - 2 second stagger recovery time
  - Action lock during stagger (no attack/block/parry)
  - Interrupts current actions on stagger
  
### Implementation Details
- Poise values: Jab deals 20 damage, Heavy deals 50
- RegenPoise() runs every 0.1s when active
- Stagger clears all combat actions
- Poise resets to max after stagger recovery

---

## Session: 2025-01-16 - Phase P5 - Heavy Attack Special

### Changes
- **Heavy Attack Basic Implementation**
  - E key binding for heavy attack
  - Attack.Type.Heavy tag support
  - Ragdoll system in DamageCalculator
  - 500 knockback force with ragdoll flag
  
### Implementation Details
- Heavy attack uses existing attack pipeline
- bCausesRagdoll flag in AttackDataAsset
- ApplyKnockback() supports ragdoll mode
- Wall impact detection deferred to P8

---

## Session: 2025-01-16 - Phase P4 - Advanced Parry & Vulnerability System

### Changes
- **VulnerabilityComponent**
  - Manages vulnerability state with charge system
  - 8x damage multiplier during vulnerability
  - 1 second default duration
  - Charge-based consumption (default 1 charge)
  - Optional i-frames support for invincibility
  
- **Enhanced Parry System** (Later Removed)
  - Frame-perfect counter windows via animation notifies
  - Successful parry applies vulnerability to attacker
  - HandleSuccessfulParry() method for clean execution
  - I-frame grants on successful parry (optional)
  
- **ParryFeedbackComponent** (Later Removed)
  - VFX for parry success, vulnerability, and i-frames
  - Camera shake and slow-motion effects
  - Persistent vulnerability visual indicators
  - Configurable feedback intensity

### Files Created
- Source/Atlas/Components/VulnerabilityComponent.h/cpp

---

## Session: 2025-01-16 - Phase P3 - Block System Implementation

### Changes
- **Block Mechanic Implementation**
  - StartBlock()/EndBlock() methods in CombatComponent
  - 40% damage reduction when blocking (60% damage taken)
  - Block state prevents other actions
  - Space key hold activation via Enhanced Input (later moved to RMB)
  
### Implementation Details
- Block state managed via Combat.State.Blocking tag
- Damage reduction applied in DamageCalculator
- Cannot block while staggered or attacking
- Unblockable attacks bypass reduction (tag-based)

---

## Session: 2025-01-16 - Phase P2 - Animation Notifies + Hitbox System

### Changes
- **Animation Notify System**
  - Created UAttackNotify for single-frame attack events
  - Created UAttackNotifyState for continuous attack windows with hit detection
  - Created UParryWindowNotifyState for parry timing windows (later removed)
  - Notify classes support socket-based hitbox positioning
  - Debug visualization for hitbox locations during attacks
  
- **Hitbox Detection System**
  - Created UHitboxComponent extending USphereComponent
  - Per-actor hit tracking to prevent multiple hits from same attack
  - Configurable hitbox radius and forward offset
  - Debug drawing support for visualizing active hitboxes
  - Automatic deactivation timer support
  
- **Combat Integration**
  - Updated UCombatComponent with ProcessHit overload for character hits
  - Added SetAttackerParryWindow for animation-driven parry timing
  - Hit detection flows through existing damage pipeline
  - Socket-based positioning for accurate limb-based attacks

### Implementation Details
- AttackNotify: Instant hit detection at animation frame
- AttackNotifyState: Continuous detection with configurable intervals
- HitboxComponent: Reusable collision component for weapons/limbs
- Attack flow: Animation → Notify → Hitbox → CombatComponent → DamageCalculator
- Debug visualization color coding: Red (attack), Orange (continuous), Cyan (parry)

### Files Created
- Source/Atlas/Animation/AttackNotify.h/cpp
- Source/Atlas/Animation/AttackNotifyState.h/cpp
- Source/Atlas/Components/HitboxComponent.h/cpp

---

## Session: 2025-01-16 - Phase P1 - Core Gameplay Tags + Data Assets + Damage Pipeline

### Changes
- **Gameplay Tags System**
  - Created GameplayTags.csv with comprehensive combat tags taxonomy
  - Configured tag categories: Attack.Type, Attack.Property, Combat.State, Damage.Type, Character.Type, Ability, Interactable
  - Updated DefaultGameplayTags.ini to reference CSV table
  
- **Data Asset Architecture**
  - Created AttackDataAsset class for attack definitions (damage, knockback, stagger, animation)
  - Created CombatRulesDataAsset class for combat rules (block reduction, parry window, vulnerability)
  - Implemented tag-based combat rule validation (CanBlock, CanParry)
  
- **Combat System Components**
  - **UCombatComponent**: Full combat state management
    - Attack execution with Data Asset integration
    - Block system with damage reduction
    - Parry system with timing windows (later removed)
    - Vulnerability system with charge consumption
    - Poise/Stagger mechanics with regeneration (later moved)
  - **UDamageCalculator**: Centralized damage pipeline
    - Final damage calculation with multipliers
    - Block/vulnerability state processing
    - Knockback and ragdoll application
    - Damage event broadcasting
  - **UHealthComponent**: Health and death management
    - Damage reception with instigator tracking
    - Death/revival system
    - Health change event system
    
- **Character Integration**
  - Updated GameCharacterBase with Combat and Health components
  - Connected PlayerCharacter input actions to combat functions
  - Wired complete damage pipeline: Combat → Calculator → Health

### Implementation Details
- Combat Values:
  - Jab: 5 damage, 100 knockback, 20 stagger damage
  - Heavy: 500 knockback + ragdoll
  - Block: 40% damage reduction
  - Vulnerability: 8x damage multiplier, 1s duration
  - Poise: 100 max, 15/s regen after 1.5s delay
  - Focus range: 1000 units (later 2000)

### Files Created
- Source/Atlas/Data/AttackDataAsset.h/cpp
- Source/Atlas/Data/CombatRulesDataAsset.h/cpp
- Source/Atlas/Components/CombatComponent.h/cpp
- Source/Atlas/Components/HealthComponent.h/cpp
- Source/Atlas/Utilities/DamageCalculator.h/cpp

---

## Session: 2025-01-16 - Phase P0 - Project Setup & Upgrade (UE 5.5)

### Changes
- Created UE 5.5 C++ project with required modules
- Enabled plugins: Enhanced Input, Gameplay Tags, AIModule, NavigationSystem, GameplayTasks, Niagara, UMG
- Established folder structure: Characters, Core directories
- Created base character hierarchy:
  - AGameCharacterBase (abstract base class with camera setup)
  - APlayerCharacter (player-specific implementation)
  - AEnemyCharacter (enemy-specific implementation)
- Implemented Enhanced Input system with combat actions:
  - Move (WASD)
  - Look (Mouse)
  - Attack (LMB)
  - Parry (RMB) - later removed
  - Block (Space Hold) - later moved to RMB
  - Focus (Q Hold)
  - Heavy (E)
- Created AtlasPlayerController and updated AtlasGameMode
- Added initial Gameplay Tags taxonomy for combat system

### Files Created
- Source/Atlas/Characters/GameCharacterBase.h/cpp
- Source/Atlas/Characters/PlayerCharacter.h/cpp
- Source/Atlas/Characters/EnemyCharacter.h/cpp
- Source/Atlas/Core/AtlasPlayerController.h/cpp
- Source/Atlas/Core/AtlasGameMode.h/cpp

### Initial Setup Values
- Character capsule: 42 radius, 96 height
- Camera boom: 400 units
- Movement speeds: Walk 500, Enemy 400
- Jump velocity: 700
- Air control: 0.35

---

## Development Roadmap (Historical Reference)

### Original Phase Plan (P0-P10)
The project was initially structured in 10 phases:
- P0: Project setup and UE 5.5 upgrade
- P1: Core gameplay tags, data assets, and damage pipeline
- P2: Animation-driven hitbox system
- P3: Block mechanic implementation
- P4: Parry and vulnerability system (parry later removed)
- P5: Heavy attack with ragdoll
- P6: Stagger and poise system
- P7: Focus mode and interactables
- P8: Smart soft lock camera (later removed) and wall impact
- P9: Dash system
- P10: Advanced AI (current)

### Technical Decisions
- Chose custom component system over Gameplay Ability System (GAS)
- Implemented data-driven design with Data Assets
- Used Gameplay Tags for all combat rules
- Removed parry system to simplify combat
- Removed camera lock for better player control
- Moved poise system from CombatComponent to HealthComponent for better organization

---

## Key Learning Points

### What Worked Well
- Component-based architecture allowed for modular development
- Data Assets made tuning values easy without recompiling
- Gameplay Tags provided flexible rule system
- Animation notifies gave precise combat timing
- Collision-based wall impact more reliable than prediction

### Challenges Overcome
- Initial parry system was too complex - removal improved gameplay
- Camera lock felt restrictive - manual control better for players
- Predictive wall detection was unreliable - collision-based approach solved this
- Poise in CombatComponent created circular dependencies - moving to HealthComponent cleaner

### Current State
The combat system is functionally complete with all core mechanics working. The focus is now on AI behavior implementation (P10) to create engaging enemy encounters. The system is stable, performant, and ready for content creation.