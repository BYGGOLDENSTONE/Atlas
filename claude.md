# Atlas - Combat System Development

## Project Overview
**Engine**: UE 5.5 | **Type**: Single-player combat | **Focus**: C++ with data-driven design

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
- **Interactables** (neutral):
  - Vent: Physics projectile, 50 poise damage, despawns on hit
  - Valve: AoE effect (Vulnerability or Stagger), 10s cooldown

### Combat Values
- Jab: 5 damage, 100 knockback, 20 poise damage
- Heavy: 15 damage, 500 knockback + ragdoll
- Block: 60% damage taken | Vulnerability: 8x multiplier
- Poise: 100 max, 15/s regen after 1.5s delay
- Wall Impact: 2s stagger | Floor Impact: 1s ragdoll

## Development Rules
- NO GAS - use custom components
- Data-driven via Data Assets and Gameplay Tags
- Centralized damage pipeline
- Component-based with interfaces

## Completed Systems
- **P0-P2**: Core setup, tags, damage pipeline, animation notifies
- **P3-P6**: Block, vulnerability, heavy attack, poise/stagger system
- **P7**: Focus mode, interactables (Vent/Valve)
- **P8**: Wall impact detection (collision-based)
- **P9**: Dash system with state restrictions

## Current Task: P10 - Advanced AI
**Todo**: AI combos, Soul Attack (unblockable), Catch Special (gap closer), enemy dash

## Key Components
- `HealthComponent`: Health + poise management, damage/stagger events
- `CombatComponent`: Attack/block states, damage calculation
- `DashComponent`: Movement ability with i-frames
- `FocusModeComponent`: Target detection and interaction
- `WallImpactComponent`: Knockback collision detection
- `VulnerabilityComponent`: Damage multiplier state

## Important Notes
- Poise system moved from Combat to Health component (2025-01-19)
- Interactables are neutral - affect all characters
- Focus range increased to 2000 units
- No camera lock system (removed)
- No parry system (removed)