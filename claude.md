# Atlas - Combat System Development

## Project Overview
**Engine**: UE 5.5 | **Type**: Single-player roguelite combat | **Focus**: C++ with data-driven design  
**Genre**: 1v1 close-combat on decaying ring-shaped space station

## Game Concept
**High Concept**: Strategic 1v1 duels where players manage both their health and station integrity. Combat focuses on creating vulnerabilities through environmental manipulation before landing critical hits.

**Core Pillars**:
1. **Vulnerability System**: Enemies only take significant damage when vulnerable
2. **Integrity Dilemma**: Powerful abilities damage the station (dual-fail state)
3. **Strategic Combat**: Environmental awareness over button mashing
4. **Narrative Integration**: Protagonist carries deceased wife's cybernetic arm

## Core Systems

### Combat Pipeline
`UCombatComponent → UDamageCalculator → UHealthComponent`
- Animation-driven hitboxes with Data Assets
- Block: 40% damage reduction (RMB hold)
- Vulnerability: 8x damage multiplier, 1s duration
- Poise/Stagger: In HealthComponent, 100 max, breaks at 0, 2s recovery

### Key Mechanics
- **Dash** (Space): 4-directional, 400 units, i-frames, 2s cooldown
- **Focus Mode** (Q): 2000 unit range, screen-space targeting
- **Block** (RMB): 40% damage reduction when held
- **Heavy Attack** (E): 15 damage, 500 knockback + ragdoll
- **Interactables** (neutral, affect all characters):
  - Vent: Physics projectile, 50 poise damage, despawns on hit
  - Valve: AoE effect (Vulnerability or Stagger), 10s cooldown

### Combat Values
- **Basic Attack (Jab)**: 5 damage, 100 knockback, 20 poise damage
- **Heavy Attack**: 15 damage, 500 knockback + ragdoll
- **Block**: 60% damage taken (40% reduction)
- **Vulnerability**: 8x damage multiplier, 1s duration, charge-based
- **Poise**: 100 max, 15/s regen after 1.5s delay
- **Wall Impact**: 2s stagger, bounce effect
- **Floor Impact**: 1s ragdoll, recovery animation

### Knockback System
- **Direction**: Calculated from attacker to target position
- **Configurable Parameters**:
  - KnockbackUpwardForce: 0.0-1.0 (vertical lift)
  - RagdollKnockbackMultiplier: 1.0-5.0 (heavy attack scaling)
  - StandardKnockbackMultiplier: Regular knockback strength
- **Impact Detection**: Collision-based during flight
  - Wall threshold: Surface normal Z < 0.3
  - Floor threshold: Surface normal Z > 0.7
  - Min force for tracking: 150 units

## Planned Features

### P11 - Vulnerability Tiers (Implemented)
- **Stunned (Yellow)**: 1 critical hit, 2x damage, 2s duration
- **Crippled (Orange)**: 3 critical hits, 4x damage, 3s duration
- **Exposed (Red)**: 5 critical hits, 8x damage, 4s duration

### P12-13 - Station Integrity System
- **80% Integrity**: Lights flicker, steam leaks
- **60% Integrity**: Gravity fluctuations
- **40% Integrity**: Structural failures
- **20% Integrity**: Hull breaches, defensive systems activate

### P14-16 - Player Abilities
**Low Risk** (No Integrity Cost):
- Kinetic Pulse: Short-range force push
- Debris Pull: Magnetic object manipulation
- Coolant Spray: Creates slip hazard
- Basic System Hack: Remote interaction

**Medium Risk** (Minor Integrity Cost):
- Floor Destabilizer: Buckles floor sections
- Impact Gauntlet: Charged punch with extended knockback
- Localized EMP: Disables tech in radius

**High Risk** (Major Integrity Cost):
- Seismic Stamp: AoE ground smash
- Gravity Anchor: Rips and launches heavy objects
- Airlock Breach: Creates decompression hazard

### P17-18 - Enemy Archetypes
- **Duelist**: Balanced, adaptive combat
- **Brute**: Heavy, resistant, high-damage
- **Controller**: Trapper, environmental focus

### P21-22 - Wife's Arm System (Passive)
AI-controlled passive abilities:
- **Firewall**: Auto-blocks first vulnerability attempt
- **Predictive Algorithm**: Hazard trajectory warnings
- **Emergency Sealant**: Prevents integrity failure (1x per run)
- **Overclock**: Speed boost after critical hits

## Key Components

### Core Combat
- `HealthComponent`: Health + poise management, damage/stagger events
- `CombatComponent`: Attack/block states, combat state management
- `DamageCalculator`: Centralized damage calculation and knockback
- `DashComponent`: Movement ability with i-frames
- `FocusModeComponent`: Target detection and interaction
- `WallImpactComponent`: Collision-based knockback impact detection
- `VulnerabilityComponent`: Damage multiplier state with charges

### Animation System
- `AttackNotifyState`: Animation-driven attack windows
- `HitboxComponent`: Socket-based collision detection
- Animation-driven combat timing

### AI System
- `EnemyAIController`: Perception and behavior tree execution
- Behavior Tree tasks for combat decisions
- Blackboard for state management

### Data Assets
- `AttackDataAsset`: Attack definitions (damage, knockback, tags)
- `CombatRulesDataAsset`: Global combat values and rules
- Tag-based rule validation

## Technical Architecture
- **No GAS**: Custom component-based system
- **Data-Driven**: All values in Data Assets
- **Tag-Based Rules**: Gameplay tags drive combat logic
- **Event-Driven**: Delegates for all state changes
- **Component Interfaces**: IInteractable, ICombatant

## Development Rules
- NO GAS - use custom components
- Data-driven via Data Assets and Gameplay Tags
- Centralized damage pipeline
- Component-based with interfaces
- No parry system (removed for simplicity)
- No camera lock system (full manual control)

## Important Notes
- Poise system in HealthComponent (not CombatComponent)
- Interactables are neutral (affect all characters)
- Focus range: 2000 units
- No camera lock system (full manual control)
- No parry system (simplified combat)
- Wall impacts use collision detection (not prediction)
- Knockback direction based on attacker-target vector

## Testing & Debug

### Console Commands
- `Atlas.ToggleFocusMode` - Toggle focus mode
- `Atlas.ShowFocusDebug` - Visual debugging
- `Atlas.SpawnTestInteractable` - Spawn test objects
- Various combat state inspection commands

### Debug Visualization
- Yellow sphere: Knockback tracking
- Red sphere: Wall impact
- Green sphere: Floor impact
- Magenta arrow: Knockback direction
- Text overlays for state info

## Completed Systems
- **P0-P2**: Core setup, tags, damage pipeline, animation notifies
- **P3-P6**: Block, vulnerability, heavy attack, poise/stagger system
- **P7**: Focus mode, interactables (Vent/Valve)
- **P8**: Wall impact detection (collision-based)
- **P9**: Dash system with state restrictions
- **P10**: Advanced AI (BTTasks for Soul Attack, Catch Special, Combos)
- **P11**: Tiered Vulnerability System (Stunned/Crippled/Exposed)

## Current Task: P12 - Station Integrity System
**Todo**: 
- Add station health bar (separate from player health)
- Implement integrity damage from high-risk abilities
- Create instability effects at different integrity levels

## Setup Required
- See `ENGINE_SETUP_GUIDE.md` for configuration steps
- See `COMPONENT_ARCHITECTURE.md` for system dependencies
- See `changelog.md` for development history