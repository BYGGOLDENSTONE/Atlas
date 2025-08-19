# Atlas Development Roadmap

## Current Status
**Completed**: P0-P9 (Core combat, damage pipeline, block/vulnerability, poise/stagger, focus mode, interactables, wall impact, dash)  
**Active**: P10 - Advanced AI

---

## Phase 1: Complete Core Combat Foundation

### P10 - Advanced AI System ⚡ *[CURRENT]*
- Enemy combo sequences
- Soul Attack (unblockable mechanic)
- Catch Special (gap closer)
- Enemy dash ability

### P11 - Tiered Vulnerability System
- Convert current 8x multiplier to tiered system
  - **Stunned** (Yellow): 1 critical hit
  - **Crippled** (Orange): 2-3 critical hits
  - **Exposed** (Red): 3-5 critical hits
- Visual indicators per tier
- Different recovery times

---

## Phase 2: Station Integrity System

### P12 - Dual Health Bar System
- Station Integrity component and UI
- Integrity damage calculation
- Ability risk categorization (Low/Medium/High)
- Integrity-damage events

### P13 - Station Instability Effects
- **80% Integrity**: Lights flicker, steam leaks
- **60% Integrity**: Gravity fluctuations
- **40% Integrity**: Structural failures, floor collapses
- **20% Integrity**: Hull breaches, defensive systems activate

---

## Phase 3: Expanded Player Abilities

### P14 - Low-Risk Abilities (No Integrity Cost)
- **Kinetic Pulse**: Short-range force push
- **Debris Pull**: Magnetic object manipulation
- **Coolant Spray**: Creates slip hazard
- **Basic System Hack**: Remote interaction

### P15 - Medium-Risk Abilities (Minor Integrity Cost)
- **Floor Destabilizer**: Buckles floor sections
- **Impact Gauntlet**: Charged punch with extended knockback
- **Localized EMP**: Disables tech in radius

### P16 - High-Risk Abilities (Major Integrity Cost)
- **Seismic Stamp**: AoE ground smash
- **Gravity Anchor**: Rips and launches heavy objects
- **Airlock Breach**: Creates decompression hazard

---

## Phase 4: AI & Enemy System

### P17 - Enemy Archetype Framework
- **Duelist**: Balanced, adaptive combat
- **Brute**: Heavy, resistant, high-damage
- **Controller**: Trapper, environmental focus

### P18 - Adaptive AI Behavior
- Environmental hazard awareness
- Strategic positioning
- Player pattern recognition
- Behavior tree implementation
- Dynamic difficulty adjustment

---

## Phase 5: Environmental Systems

### P19 - Dynamic Environmental Hazards
- Chemical spills (DoT zones)
- Explosive consoles
- Magnetic cranes
- Security fields
- Breakable specimen tanks
- Overchargeable machinery

### P20 - Interactive Arena Elements
- Destructible walls/floors
- Dynamic throwable objects
- Activation panels
- Moveable cover
- Environmental traps

---

## Phase 6: Wife's Arm System

### P21 - Passive Ability Framework
- AI-controlled activation system
- Condition-based triggers
- Visual feedback system
- Upgrade framework

### P22 - Core Arm Abilities
- **Firewall**: Auto-blocks first vulnerability attempt
- **Predictive Algorithm**: Hazard trajectory warnings
- **Emergency Sealant**: Prevents integrity failure (1x per run)
- **Overclock**: Speed boost after critical hits

---

## Phase 7: Level Structure

### P23 - Room Theme System
- **Laboratory**: Chemical hazards, stat buff rewards
- **Supply Bay**: Crates/drones, ability rewards
- **Control Room**: Turrets/consoles, utility rewards
- **Hydroponics**: Foliage/irrigation, sustain rewards
- **Data Archives**: Security fields, arm upgrades

### P24 - Procedural Level Generation
- Room ordering algorithm
- Hazard placement rules
- Enemy spawn variations
- Reward distribution

---

## Phase 8: Roguelite Framework

### P25 - Run Management
- 10+ levels per run
- Death/restart system
- Checkpoint system
- Run statistics
- Persistent unlocks

### P26 - Reward System
- Stat buffs (HP, damage, integrity repair)
- New abilities
- Environmental access (turret control, blast doors)
- Arm upgrades
- Meta-currency

---

## Phase 9: Polish & Balance

### P27 - Combat Feel
- Animation polish
- Hit pause/slowdown
- Camera shake
- Particle effects
- Sound design integration

### P28 - Difficulty Balancing
- Damage value tuning
- Cooldown optimization
- Integrity cost balancing
- Enemy health/damage scaling
- Vulnerability window timing

---

## Phase 10: Meta Systems

### P29 - Progression System
- Unlock conditions
- Meta-currency spending
- Achievement tracking
- Statistics tracking
- Leaderboards

### P30 - Narrative Integration
- Story beats between levels
- Environmental storytelling
- Dialogue system
- Multiple endings
- Lore collectibles

---

## Technical Debt & Optimization
- Performance profiling
- Memory optimization
- Loading optimization
- Network preparation (if multiplayer considered)
- Save system implementation

## Post-Launch Considerations
- New enemy types
- Additional room themes
- Extended ability sets
- Difficulty modes
- Community-requested features

---

*Note: Each phase builds upon the previous work. Phases can be adjusted based on testing feedback and priority changes.*