# Atlas Development Changelog

## Session: 2025-08-16 - Phase P0 - Project Setup & Upgrade (UE 5.5)

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
  - Parry (RMB)
  - Block (Space Hold)
  - Focus (Q Hold)
  - Heavy (E)
- Created AtlasPlayerController and updated AtlasGameMode
- Added initial Gameplay Tags taxonomy for combat system

### Tests Performed
- Project compiles successfully with all modules
- Character movement and camera controls ready for testing in PIE
- Input action debug logs verify input binding

### Tuning Values
- Character capsule: 42 radius, 96 height
- Camera boom: 400 units
- Movement speeds: Walk 500, Enemy 400
- Jump velocity: 700
- Air control: 0.35

### Known Issues / Next
- Input Mapping Context assets need to be created in Editor
- Character blueprints need to be created from C++ classes
- Ready for PIE testing after Editor setup
- Next phase: P1 - Core Gameplay Tags + Data Assets + Damage Pipeline

## Session: 2025-08-16 - Phase P1 - Core Gameplay Tags + Data Assets + Damage Pipeline

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
    - Parry system with timing windows
    - Vulnerability system with charge consumption
    - Poise/Stagger mechanics with regeneration
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
- **Combat Values**:
  - Jab: 5 damage, 100 knockback, 20 stagger damage
  - Heavy: 500 knockback + ragdoll
  - Block: 40% damage reduction
  - Vulnerability: 8x damage multiplier, 1s duration
  - Poise: 100 max, 15/s regen after 1.5s delay
  - Parry window: 0.3s (configurable)
  - Focus range: 1000 units

### Tests Required
- Create attack Data Assets in editor (Jab, Heavy)
- Create combat rules Data Asset with default values
- Assign Data Assets to player/enemy Combat Components
- Test damage pipeline with debug output
- Verify block/parry/vulnerability mechanics

### Known Issues / Next
- Data Assets need to be created in Editor
- Attack animations need to be hooked up
- Hitbox detection system not yet implemented
- Next phase: P2 - Animation Notifies + Hitbox System

## Session: 2025-08-17 - Phase P2 - Animation Notifies + Hitbox System

### Changes
- **Animation Notify System**
  - Created UAttackNotify for single-frame attack events
  - Created UAttackNotifyState for continuous attack windows with hit detection
  - Created UParryWindowNotifyState for parry timing windows
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
  - Parry window now controlled by animation notifies (attacker-defined)
  - Hit detection flows through existing damage pipeline
  - Socket-based positioning for accurate limb-based attacks

### Implementation Details
- **AttackNotify**: Instant hit detection at animation frame
- **AttackNotifyState**: Continuous detection with configurable intervals
- **ParryWindowNotifyState**: Opens/closes parry vulnerability window
- **HitboxComponent**: Reusable collision component for weapons/limbs
- Attack flow: Animation → Notify → Hitbox → CombatComponent → DamageCalculator
- Debug visualization color coding: Red (attack), Orange (continuous), Cyan (parry)

### Files Created
- Source/Atlas/Animation/AttackNotify.h/cpp
- Source/Atlas/Animation/AttackNotifyState.h/cpp
- Source/Atlas/Animation/ParryWindowNotifyState.h/cpp
- Source/Atlas/Components/HitboxComponent.h/cpp

### Tests Required
- Add animation notifies to attack montages in Editor
- Configure socket names for hand/foot positions
- Test hitbox collision detection with debug drawing
- Verify parry window timing with animations
- Test multi-hit prevention per attack

### Build Issues Fixed
- Fixed InternalConstructor crash by moving component initialization to BeginPlay
- Changed TSet to TArray with UPROPERTY for proper garbage collection
- Removed AnimGraphRuntime dependency (not needed for simple notifies)
- Simplified all constructors to prevent initialization crashes

### Setup Completed
- ✅ Attack montages created with animation notifies
- ✅ Socket configuration added (hand_r, hand_l)
- ✅ Debug visualization working (red/orange/cyan spheres)
- ✅ Hit detection flowing through damage pipeline
- ✅ Parry windows properly timed with animations

### Working Features
- Animation-driven hit detection at correct frames
- Socket-based hitbox positioning for accurate hits
- Per-actor hit tracking prevents multiple hits
- Debug visualization for all combat states
- Full integration with P1 damage pipeline

### Known Issues / Next
- Ready for P3 - AI Behavior Trees + Enemy Combat
- Enemy AI needs combat decision making
- Interactables system to be implemented later

## Session: 2025-08-16 - Phase P1 Fixes & Polish

### Changes
- **Fixed null pointer crashes in CombatComponent**
  - Added null checks for CombatRules throughout
  - Added default values when CombatRules not set
  - Fixed crash on TickComponent and other functions
  
- **Implemented native C++ gameplay tags for UE 5.5**
  - Created FAtlasGameplayTags singleton
  - Tags now initialize on module startup
  - More reliable than INI configuration
  
- **Enhanced debug logging**
  - Added detailed logging to all combat functions
  - Attack logs show damage and knockback values
  - Block/Parry/Focus all log state changes
  
- **Fixed input hold mechanics**
  - Documented Hold trigger configuration for Input Actions
  - Block (Space) and Focus (Q) now properly require holding
  - Added comprehensive input setup guides

### Documentation Added
- `BLUEPRINT_SETUP_GUIDE.md` - Complete BP configuration guide
- `GAMEPLAY_TAGS_SETUP.md` - Gameplay tags implementation guide
- `INPUT_HOLD_FIX.md` - Input hold configuration guide

### Tests Performed
- All combat inputs tested and working:
  - LMB: Jab attack (5 damage)
  - RMB: Parry window (0.3s)
  - Space (Hold): Block (40% reduction)
  - Q (Hold): Focus mode
  - E: Heavy attack (15 damage)
- No crashes during gameplay
- Debug output confirms all systems functioning

### Known Issues / Next
- Attacks don't actually hit yet (no animation notifies)
- Damage pipeline ready but needs hitbox detection
- Next phase: P2 - Animation Notifies + Hitbox System

## Session: 2025-08-18 - Phase P3 - AI Behavior Trees + Enemy Combat

### Changes
- **AI Controller System**
  - Created EnemyAIController with perception system
  - Implemented sight and damage sense configuration
  - Auto-possession of enemy pawns
  
- **Behavior Tree Tasks**
  - BTTask_SimpleAttack: Basic melee attack execution
  - BTTask_MeleeAttack: Advanced combo attacks
  - BTTask_DefendBlock: Defensive stance activation
  - BTTask_SoulAttack: Unblockable special (10s cooldown)
  - BTTask_CatchSpecial: Anti-kiting gap closer
  
- **AI Combat Integration**
  - Connected AI tasks to CombatComponent
  - Implemented attack cooldowns and state management
  - Added combo execution logic
  - Integrated with existing damage pipeline

### Implementation Details
- AI perception range: 2000 units sight
- Attack decision making via Behavior Tree
- Cooldown tracking for special abilities
- State-based combat transitions

### Tests Required
- Create Behavior Tree asset in editor
- Set up Blackboard with required keys
- Configure perception settings
- Test AI combat engagement

### Known Issues / Next
- Ready for P4 - Advanced Parry & Vulnerability
- AI needs parry reaction behavior
- Interactables system pending

## Session: 2025-08-18 - Phase P4 - Advanced Parry & Vulnerability System

### Changes
- **VulnerabilityComponent**
  - Manages vulnerability state with charge system
  - 8x damage multiplier during vulnerability
  - 1 second default duration
  - Charge-based consumption (default 1 charge)
  - Optional i-frames support for invincibility
  
- **Enhanced Parry System**
  - Frame-perfect counter windows via animation notifies
  - Successful parry applies vulnerability to attacker
  - HandleSuccessfulParry() method for clean execution
  - I-frame grants on successful parry (optional)
  
- **ParryFeedbackComponent**
  - VFX for parry success, vulnerability, and i-frames
  - Camera shake and slow-motion effects
  - Persistent vulnerability visual indicators
  - Configurable feedback intensity
  
- **Debug Testing System**
  - Console commands: TestParryWindow, TestApplyVulnerability, ShowParryInfo
  - Real-time state inspection
  - Damage simulation tools
  - TestVulnerabilityDamage command

### Implementation Details
- Parry window controlled by attacker's animation
- Vulnerability stacks with existing damage multipliers
- I-frames duration configurable per-character
- Visual feedback scales with damage dealt

### Files Created
- Source/Atlas/Components/VulnerabilityComponent.h/cpp
- Source/Atlas/Components/ParryFeedbackComponent.h/cpp
- Source/Atlas/Debug/ParryTestCommands.h/cpp

### Documentation
- P4_ENGINE_SETUP_GUIDE.md created with complete setup instructions
- Detailed configuration steps for all components
- Troubleshooting guide included

### Tests Performed
- Parry window timing verified
- Vulnerability damage multiplier confirmed at 8x
- I-frames prevent damage during window
- Console commands functional

### Known Issues / Next
- Ready for P5 - Heavy Attack Special implementation

## Session: 2025-08-18 - Phase P3 - Block System (40% damage reduction)

### Changes
- **Block Mechanic Implementation**
  - StartBlock()/EndBlock() methods in CombatComponent
  - 40% damage reduction when blocking (60% damage taken)
  - Block state prevents other actions
  - Space key hold activation via Enhanced Input
  
### Implementation Details
- Block state managed via Combat.State.Blocking tag
- Damage reduction applied in DamageCalculator
- Cannot block while staggered or attacking
- Unblockable attacks bypass reduction (tag-based)

### Tests Performed
- Verified 40% damage reduction when blocking
- Confirmed block state prevents attacks
- Tested unblockable tag bypass

### Known Issues / Next
- Ready for P5 - Heavy Attack implementation

## Session: 2025-08-18 - Phase P5 - Heavy Attack Special (partial)

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
- **Note**: Wall impact detection moved to P8

### Tests Performed
- Heavy attack input triggers correctly
- Ragdoll applied on hit with proper knockback

### Known Issues / Next
- Wall impact detection deferred to P8
- Ready for P6 - Stagger/Poise System

## Session: 2025-08-18 - Phase P6 - Stagger System + Light Poise

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

### Tests Performed
- Verified poise break after repeated hits
- Stagger state blocks all actions
- Poise regeneration timing confirmed

### Known Issues / Next
- Ready for P7 - Focus Mode & Interactables

## Session: 2025-08-18 - Phase P7 - Focus Mode & Interactables - COMPLETED

### Changes
- **Focus Mode System**
  - Q key hold activation with 1000 unit range
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
  - Four types: Fire, Electric, Poison, Physical
  - 10 second cooldown
  - Optional vulnerability/stagger effects

### Debug Commands
- Atlas.ToggleFocusMode - Toggle focus mode
- Atlas.ShowFocusDebug - Visual debugging
- Atlas.SpawnTestInteractable - Spawn test objects
- Atlas.ShowFocusInfo - Display current state

### Tests Performed
- Focus mode detection and targeting verified
- Interactable triggering works correctly
- Smart Soft Lock engages in combat range
- Force-unlock with camera movement functional

### Known Issues / Next
- Ready for P8 - Smart Soft Lock Camera + Wall Impact System

## Session: 2025-08-18 - Phase P8 - Smart Soft Lock + Wall Impact System - COMPLETED

### Changes
- **Combat State Detection**
  - IsInCombat() method checks active combat states
  - Tracks last combat action timestamp
  - 3-second combat memory for smoother transitions
  - Combat states: Attacking, Blocking, Parrying, Staggered
  
- **Enhanced Soft Lock Camera**
  - Auto-engages when entering combat near enemies (800 units)
  - Soft camera rotation tracks locked target
  - Player can override with strong input
  - Force-unlock with rapid camera movement (>180°/s)
  - Preserves player pitch control
  
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
- Combat auto-engage checks every tick when not in focus mode
- Soft lock interpolation speed: 2.0 (adjustable)
- Wall detection uses WorldStatic collision channel
- Wall verification: Surface must be <30° from vertical
- Extended stagger applies 100 poise damage initially

### Files Created
- Source/Atlas/Components/WallImpactComponent.h/cpp

### Tests Required
- Verify combat auto-engage triggers correctly
- Test soft lock camera tracking smoothness
- Confirm force-unlock sensitivity
- Test wall impact detection accuracy
- Verify bonus damage and extended stagger

### Known Issues
- Soft lock camera has some asymmetry in rotation handling (functional but needs polish)
- AI "out of range" warning removed but AI attack timing needs tuning

### Next Steps
- P9 - Complete Advanced AI Features (combos, Soul Attack, Catch Special)
- P10 - Polish & Debug (hit-pause, camera shake, debug HUD)
- Soft lock system refinement for smoother player experience