# Atlas - Combat System Development

## Project Overview

* **Engine**: Unreal Engine 5.5
* **Type**: Single-player combat game with AI enemies
* **Focus**: C++ implementation with data-driven design

## Core Systems Architecture

### Combat Framework

* **Damage Pipeline**: UCombatComponent → UDamageCalculator → UHealthComponent
* **Attack System**: Animation-driven hitboxes with Data Assets
* **Defense Mechanics**:

  * Block: 40% damage reduction (RMB hold)
  * Vulnerability: 8x damage multiplier with charge system

* **Stagger/Poise**: Managed by HealthComponent, prevents infinite trading, action lock on poise break
* **Mobility**:

  * Dash: 4-directional dash (Space), invincibility frames
  * State restrictions: Cannot dash while attacking/blocking/staggered/airborne

### Key Mechanics

* **Focus Mode (Q)**: Screen-space targeting for interactables (2000 unit range)
* **Interactables**: Neutral objects that affect all characters
  * **Vent**: Physics projectile, applies stagger, despawns after hit
  * **Valve**: Two types - Vulnerability or Stagger AoE effect

### AI System

* Behavior Tree/Blackboard driven
* Combat behaviors: Basic attacks, combos, block reactions
* Special attacks: Soul Attack (unblockable, 10s CD), Catch Special (anti-kiting)
* Environmental awareness for interactable usage

## Important Values

* Jab: 5 damage, 100 knockback
* Heavy: 500 knockback + ragdoll
* Vulnerability: 1s duration, 1 charge default, 8x multiplier
* Block: 60% damage taken (40% reduction) - RMB hold
* Poise: 100 max, 20 jab damage, 15/s regen after 1.5s (managed by HealthComponent)
* Focus Range: 2000 units
* Dash: 400 units distance, 1s duration, 2s cooldown (configurable)

## Development Rules

* NO Gameplay Ability System (GAS)
* Data-driven via Data Assets and Gameplay Tags
* All damage flows through centralized pipeline
* Tag-based combat rules (Blockable, etc.)
* Component-based architecture with interfaces

## Completed Phases Status

### ✅ P0: Project Setup \& Upgrade (UE 5.5) - COMPLETED

### ✅ P1: Core Gameplay Tags + Data Assets + Damage Pipeline - COMPLETED

### ✅ P2: Animation Notifies + Hitbox System - COMPLETED

### ✅ P3: Block System (40% damage reduction) - COMPLETED

### ✅ P4: Vulnerability System - COMPLETED

### ✅ P5: Heavy Attack (partial) - IN PROGRESS

* Heavy attack input and basic functionality implemented
* Ragdoll knockback system working
* **Missing**: Wall impact detection and bonus damage (moved to P8)

### ✅ P6: Stagger System + Poise - COMPLETED (Refactored 2025-01-19)

* Full poise system moved to HealthComponent (100 max, 20 jab damage, 15/s regen)
* Stagger on poise break with recovery timer
* Action lock during stagger state
* Hit reaction animations on poise damage

### ✅ P7: Focus Mode \& Interactables - COMPLETED

* **Focus Mode System**:

  * Q key hold to activate (2000 unit detection range)
  * Screen-space targeting prioritizes closest to center
  * Release Q to interact with focused target
  * Debug visualization with colored spheres and lines

* **FocusModeComponent**: Complete targeting system

  * Scans for interactables in range
  * Calculates screen-space priorities for targeting

* **Interactable System**:

  * IInteractable interface for all interactable objects
  * InteractableBase abstract class with cooldown management
  * Visual feedback for focused/cooldown states

* **Vent Interactable**: Physics-based projectile (Refactored 2025-01-19)

  * Launches itself when triggered (predetermined direction)
  * Configurable physics (mass, speed, bounce)
  * Applies stagger on hit (50 poise damage) to any character
  * Despawns after hitting a character
  * Plays hit reaction animation on impact
  * One-time use only

* **Valve Interactable**: Area effect (Refactored 2025-01-19)

  * 500 unit AoE radius with visual effects
  * Two types: Vulnerability (purple) or Stagger (yellow)
  * Affects all characters in range (neutral behavior)
  * Single instant effect (no damage over time)
  * 10-second cooldown

### ✅ P8: Wall Impact - COMPLETED (Refactored 2025-08-19)

* **Wall Impact System**: Collision-based detection during knockback

  * Reactive collision detection (not predictive)
  * Tracks enemy during flight with attached collision sphere
  * Distinguishes wall impacts (2s stagger) vs floor impacts (1s ragdoll)
  * Works from ANY direction (sideways, diagonal, forward, backward)
  * No damage on wall impact (stagger-only effect)
  * Configurable knockback forces (upward, multipliers)
  * Enhanced debug visualization (following spheres, directional arrows)

### ✅ P9: Dash System - COMPLETED

* **DashComponent**: Full 4-directional dash system

  * Space key to dash (migrated block to RMB)
  * W/A/S/D determines dash direction (cardinal only)
  * Dash forward when no movement input
  * 400 unit distance, 1s duration, 2s cooldown (configurable via Data Asset)
  * Invincibility frames during dash
  * Collision detection stops dash early
  * State restrictions: Cannot dash while attacking/blocking/staggered/airborne

### 🔄 P10: Basic Enemy AI - PARTIALLY COMPLETED

* Basic AI controller with perception
* Simple attack tasks implemented
* **Missing**: Advanced features (combos, Soul Attack, Catch Special)

## Current Phase: P10 - Advanced AI Features

Next to implement:

* AI combo attacks system
* Soul Attack (unblockable, 10s cooldown)
* Catch Special (anti-kiting gap closer)
* Add dash ability to enemy AI

## Remaining Phases:

* **P10**: Complete Advanced AI Features (including enemy dash)
* **P11**: Polish \& Debug (hit-pause, camera shake, debug HUD)
