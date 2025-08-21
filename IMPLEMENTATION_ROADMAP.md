# Atlas Implementation Roadmap

## Overview
This roadmap outlines the phased implementation plan for Atlas, a 1v1 roguelite dueling game. Each phase builds upon the existing unified ActionManagerComponent architecture and follows data-driven design principles.

## Current Status ✅
- **Unified Component System**: ActionManagerComponent handles all combat/actions
- **4 Core Interfaces**: ICombatInterface, IHealthInterface, IActionInterface, IInteractable
- **Animation-Driven Combat**: Working attack timing via notifies
- **Basic Combat**: Attack, Block, Heavy Attack, Dash functional
- **Vulnerability System**: 3-tier system implemented
- **Station Integrity**: Risk/reward mechanic functional
- **Focus Mode**: Environmental targeting system working

## Phase 1: Core Combat Polish & Parry System
**Goal**: Complete core combat mechanics per GDD requirements

### Tasks:
1. **Re-implement Parry System** (GDD explicitly requires this)
   - Create ParryAction class extending BaseAction
   - Add ParryDataAsset with timing windows
   - Implement parry-to-vulnerability conversion
   - Add animation notifies for parry windows
   - Test parry vs all attack types

2. **Soul Attack Implementation**
   - Create SoulAttackDataAsset (unblockable, no station damage)
   - Add unique visual effects
   - Balance damage values (50 base per GDD)

3. **Combat Value Tuning**
   - Verify 8x vulnerability multiplier
   - Confirm 40% block reduction
   - Test poise values (100 max, proper regen)
   - Validate dash i-frames (0.3s window)

## Phase 2: Reward System Architecture
**Goal**: Implement the slot-based reward system

### Tasks:
1. **Reward Base Classes**
   - Create RewardDataAsset base class
   - DefenseRewardDataAsset (Block, Parry, Dodge)
   - OffenseRewardDataAsset (Attacks)
   - PassiveStatRewardDataAsset (Speed, Health, etc.)
   - PassiveAbilityRewardDataAsset (Second Life, etc.)
   - InteractableRewardDataAsset (Hacking abilities)

2. **Slot Management System**
   - Create SlotManagerComponent
   - Implement slot UI framework
   - Add reward equip/replace logic
   - Support multi-slot rewards
   - Implement reward enhancement stacking

3. **Reward Persistence**
   - Save equipped rewards at run end
   - Load rewards at run start
   - Handle reward enhancement levels
   - Create save game system

## Phase 3: Run Structure & Level Flow
**Goal**: Implement the 5-room run structure

### Tasks:
1. **Room System**
   - Create RoomDataAsset (enemy type, reward pool, theme)
   - Implement 5 unique rooms (A-E)
   - Room randomization system
   - Level progression manager

2. **Reward Pool System**
   - Create themed reward pools per room
   - Implement reward selection UI (2 choices)
   - Scale rewards by level (1-5)
   - Implement reward rarity tiers

3. **Run Management**
   - Create RunManagerComponent
   - Track current level (1-5)
   - Handle run completion/failure
   - Implement run statistics

## Phase 4: Dynamic Enemy AI System
**Goal**: Implement reactive enemy scaling

### Tasks:
1. **Shared Ability Pool**
   - Enemies use same ActionDataAssets as player
   - Create EnemyAbilitySelector component
   - Implement ability assignment from pools

2. **Reactive Scaling**
   - Count player filled slots
   - Assign enemy abilities (player slots + 1)
   - Balance enemy stat scaling

3. **Themed AI Behaviors**
   - Create 5 unique AI behavior trees
   - Room A Enemy: Defense priority
   - Room B Enemy: Offense priority
   - Room C Enemy: Passive/tactical
   - Room D Enemy: Aggressive/risky
   - Room E Enemy: Interactable-focused

## Phase 5: Enhanced Interactables
**Goal**: Expand environmental interaction system

### Tasks:
1. **New Interactable Types**
   - Exploding Valve (AoE stagger)
   - Launch Vent (vulnerability on collision)
   - Gravity Well (pull effect)
   - Shield Generator (temporary defense)
   - Weapon Rack (temporary damage boost)

2. **Hacking System**
   - Create HackingComponent
   - Implement hack timing/cost
   - Add visual hacking indicators
   - Balance risk/reward

3. **Neutral Targeting**
   - Ensure all interactables affect any character
   - Add friendly fire warnings
   - Implement strategic positioning

## Phase 6: Passive Systems
**Goal**: Implement all passive rewards

### Tasks:
1. **Stat Passives**
   - Movement speed modifiers
   - Attack speed scaling
   - Knockback force multipliers
   - Max health increases
   - Station health boosts

2. **Ability Passives**
   - Second Life system (revival + slot consumption)
   - Double jump
   - Health regeneration
   - Damage reflection
   - Critical hit chance

3. **Multi-Slot Passives**
   - Implement 2-slot requirement system
   - Ultra-powerful abilities
   - Visual slot linking in UI

## Phase 7: Visual Feedback & Polish
**Goal**: Complete visual communication systems

### Tasks:
1. **Combat Feedback**
   - Hit reactions and stagger animations
   - Vulnerability visual states (Yellow/Orange/Red)
   - Block and parry effects
   - Damage numbers

2. **UI Systems**
   - Health bars
   - Station integrity meter
   - Slot management interface
   - Reward selection screens
   - Run progress tracker

3. **Environmental Effects**
   - Station decay visuals at 50% integrity
   - Wall impact effects
   - Interactable activation VFX
   - Room transition sequences

## Phase 8: Meta Progression
**Goal**: Long-term player progression

### Tasks:
1. **Unlock System**
   - Track total runs completed
   - Unlock new starting rewards
   - Achievement system
   - Statistics tracking

2. **Difficulty Scaling**
   - Implement difficulty tiers
   - Enemy stat scaling
   - Reward quality adjustments
   - New Game+ mechanics

## Phase 9: Audio & Game Feel
**Goal**: Complete audio implementation

### Tasks:
1. **Combat Audio**
   - Impact sounds by attack type
   - Vulnerability activation stings
   - Station damage warnings
   - Ambient station decay

2. **Music System**
   - Dynamic combat music
   - Room-specific themes
   - Intensity scaling

## Phase 10: Testing & Balance
**Goal**: Final polish and balance

### Tasks:
1. **Playtesting**
   - Full run completion testing
   - Difficulty curve validation
   - Reward balance verification
   - Bug fixing

2. **Performance**
   - Optimization pass
   - Memory management
   - Loading time reduction

## Technical Guidelines

### All implementations must follow:
1. **Component Architecture**: Use existing ActionManagerComponent system
2. **Data-Driven Design**: All values in DataAssets
3. **Interface Communication**: Use the 4 core interfaces
4. **Animation-Driven**: Combat timing via notifies
5. **No GAS**: Continue using custom component system
6. **Tag-Based Rules**: Use GameplayTags for all conditions

### Code Standards:
- Single responsibility components
- Interface-based communication
- No circular dependencies
- Comprehensive debug commands
- Clear separation of concerns

## Priority Notes
- **Phase 1-3**: Core gameplay loop (CRITICAL)
- **Phase 4-5**: Enemy variety and depth
- **Phase 6-7**: Polish and feedback
- **Phase 8-10**: Long-term retention

## Next Immediate Steps
1. Begin Phase 1: Re-implement parry system
2. Create Soul Attack ability
3. Tune existing combat values to match GDD
4. Start prototyping reward data structure