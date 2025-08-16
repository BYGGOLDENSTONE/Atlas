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