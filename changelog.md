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