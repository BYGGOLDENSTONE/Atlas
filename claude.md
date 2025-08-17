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
  - Parry: Frame-perfect counter with vulnerability charges
  - Vulnerability: 8x damage multiplier with charge system
- **Stagger/Poise**: Prevents infinite trading, action lock on poise break

### Key Mechanics
- **Focus Mode (Q)**: Screen-space targeting for interactables (1000 unit range)
- **Smart Soft Lock**: Auto-engages in combat, force-unlock with rapid camera movement
- **Interactables**: Vent (projectile stagger), Valve (AoE with archetype-specific effects)

### AI System
- Behavior Tree/Blackboard driven
- Combat behaviors: Basic attacks, combos, parry/block reactions
- Special attacks: Soul Attack (unblockable/unparryable, 10s CD), Catch Special (anti-kiting)
- Environmental awareness for interactable usage

## Important Values
- Jab: 5 damage, 100 knockback
- Heavy: 500 knockback + ragdoll
- Parry Window: Attacker's notify-driven
- Vulnerability: 1s duration, 1 charge default, 8x multiplier
- Block: 60% damage taken (40% reduction)
- Poise: 100 max, 20 jab damage, 15/s regen after 1.5s
- Focus Range: 1000 units

## Development Rules
- NO Gameplay Ability System (GAS)
- Data-driven via Data Assets and Gameplay Tags
- All damage flows through centralized pipeline
- Tag-based combat rules (Blockable, Parryable, etc.)
- Component-based architecture with interfaces

## Current Phase: P4 - COMPLETED ✓
Successfully implemented advanced parry mechanics with vulnerability system, charge-based damage multipliers, and optional i-frames.

### P4 Features Implemented:
- **VulnerabilityComponent**: Manages vulnerability state with charge system
  - 8x damage multiplier during vulnerability
  - 1 second default duration
  - Charge-based consumption (default 1 charge)
  - Optional i-frames support for invincibility
- **Enhanced Parry System**:
  - Frame-perfect counter windows via animation notifies
  - Successful parry applies vulnerability to attacker
  - HandleSuccessfulParry() method for clean execution
  - I-frame grants on successful parry (optional)
- **ParryFeedbackComponent**: Visual/audio feedback system
  - VFX for parry success, vulnerability, and i-frames
  - Camera shake and slow-motion effects
  - Persistent vulnerability visual indicators
- **Debug Testing System**: Console commands for testing
  - TestParryWindow, TestApplyVulnerability, ShowParryInfo
  - Real-time state inspection and damage simulation

## Next Phase: P5 - Polish & Balancing
Ready to refine combat feel, add juice, and balance gameplay systems.