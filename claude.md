# Atlas - Combat System Development

## Project Overview
- **Engine**: Unreal Engine 5.5
- **Type**: Single-player combat game with AI enemies
- **Focus**: C++ implementation with data-driven design

## Core Systems Architecture

### Combat Framework
- **Damage Pipeline**: UCombatComponent → UDamageCalculator → UHealthComponent
- **Attack System**: Animation-driven hitboxes with Data Assets
- **Defense Mechanics**: 
  - Block: 40% damage reduction (Space hold)
  - Vulnerability: 8x damage multiplier with charge system
- **Stagger/Poise**: Prevents infinite trading, action lock on poise break

### Key Mechanics
- **Focus Mode (Q)**: Screen-space targeting for interactables (1000 unit range)
- **Interactables**: Vent (projectile stagger), Valve (AoE with archetype-specific effects)

### AI System
- Behavior Tree/Blackboard driven
- Combat behaviors: Basic attacks, combos, block reactions
- Special attacks: Soul Attack (unblockable, 10s CD), Catch Special (anti-kiting)
- Environmental awareness for interactable usage

## Important Values
- Jab: 5 damage, 100 knockback
- Heavy: 500 knockback + ragdoll
- Vulnerability: 1s duration, 1 charge default, 8x multiplier
- Block: 60% damage taken (40% reduction)
- Poise: 100 max, 20 jab damage, 15/s regen after 1.5s
- Focus Range: 1000 units

## Development Rules
- NO Gameplay Ability System (GAS)
- Data-driven via Data Assets and Gameplay Tags
- All damage flows through centralized pipeline
- Tag-based combat rules (Blockable, etc.)
- Component-based architecture with interfaces

## Completed Phases Status

### ✅ P0: Project Setup & Upgrade (UE 5.5) - COMPLETED
### ✅ P1: Core Gameplay Tags + Data Assets + Damage Pipeline - COMPLETED  
### ✅ P2: Animation Notifies + Hitbox System - COMPLETED
### ✅ P3: Block System (40% damage reduction) - COMPLETED
### ✅ P4: Vulnerability System - COMPLETED
### ✅ P5: Heavy Attack (partial) - IN PROGRESS
- Heavy attack input and basic functionality implemented
- Ragdoll knockback system working
- **Missing**: Wall impact detection and bonus damage (moved to P8)

### ✅ P6: Stagger System + Poise - COMPLETED
- Full poise system (100 max, 20 jab damage, 15/s regen)
- Stagger on poise break with recovery timer
- Action lock during stagger state

### ✅ P7: Focus Mode & Interactables - COMPLETED
- **Focus Mode System**:
  - Q key hold to activate (1000 unit detection range)
  - Screen-space targeting prioritizes closest to center
  - Release Q to interact with focused target
  - Debug visualization with colored spheres and lines
  
- **FocusModeComponent**: Complete targeting system
  - Scans for interactables in range
  - Calculates screen-space priorities for targeting
  
- **Interactable System**:
  - IInteractable interface for all interactable objects
  - InteractableBase abstract class with cooldown management
  - Visual feedback for focused/cooldown states
  
- **Vent Interactable**: Physics-based projectile
  - Launches itself when triggered (predetermined direction)
  - Configurable physics (mass, speed, bounce)
  - Applies stagger on hit (50 poise damage)
  - One-time use only
  
- **Valve Interactable**: Area effect visualization
  - 500 unit AoE radius with visual effects
  - Four types: Fire (red), Electric (cyan), Poison (green), Physical (white)
  - Optional vulnerability/stagger application
  - 10-second cooldown

### ✅ P8: Wall Impact - COMPLETED
- **Wall Impact System**: WallImpactComponent for heavy attacks
  - Detects walls within 200 units behind target
  - 1.5x damage multiplier on wall impact
  - Extended 2-second stagger
  - Bounce-back physics effect

### ❌ REMOVED: Camera Lock System
- Soft/Hard lock system completely removed from project
- Player has full manual camera control
- No automatic enemy tracking

### 🔄 P9: Basic Enemy AI - PARTIALLY COMPLETED
- Basic AI controller with perception
- Simple attack tasks implemented
- **Missing**: Advanced features (combos, Soul Attack, Catch Special)

## Current Phase: P9 - Advanced AI Features
Next to implement:
- AI combo attacks system
- Soul Attack (unblockable, 10s cooldown)
- Catch Special (anti-kiting gap closer)

## Remaining Phases:
- **P9**: Complete Advanced AI Features
- **P10**: Polish & Debug (hit-pause, camera shake, debug HUD)