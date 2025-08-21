# Phase 3: Content Creation & UI Implementation Plan

## Overview
With Phase 2's core systems complete, Phase 3 focuses on creating the actual content and UI that brings the game to life. This includes reward DataAssets, UI widgets, room configurations, and enemy blueprints.

## Prerequisites Completed (Phase 2)
✅ RunManagerComponent - Room progression system
✅ RewardSelectionComponent - Reward UI management
✅ SlotManagerComponent - Slot system
✅ SaveManagerSubsystem - Persistence
✅ AIDifficultyComponent - Enemy scaling

## Task 1: Create Reward DataAssets

### Defense Category (5 Rewards)
1. **DA_Reward_ImprovedBlock**
   - Increases block reduction to 60%
   - Slot Cost: 1
   - Stack: 3 levels (60%, 70%, 80%)

2. **DA_Reward_ParryMaster**
   - Extends parry window by 0.1s
   - Slot Cost: 1
   - Stack: 2 levels

3. **DA_Reward_CounterStrike**
   - Auto-counter after successful parry
   - Slot Cost: 2
   - Stack: 1 level

4. **DA_Reward_IronSkin**
   - Passive 10% damage reduction
   - Slot Cost: 1
   - Stack: 3 levels (10%, 15%, 20%)

5. **DA_Reward_LastStand**
   - Unbreakable block below 25% health
   - Slot Cost: 2
   - Stack: 1 level

### Offense Category (5 Rewards)
1. **DA_Reward_SharpBlade**
   - +20% melee damage
   - Slot Cost: 1
   - Stack: 3 levels

2. **DA_Reward_HeavyImpact**
   - Heavy attacks cause AOE
   - Slot Cost: 2
   - Stack: 2 levels

3. **DA_Reward_BleedingStrikes**
   - Attacks apply bleed DOT
   - Slot Cost: 1
   - Stack: 3 levels

4. **DA_Reward_Executioner**
   - +100% damage to stunned
   - Slot Cost: 2
   - Stack: 1 level

5. **DA_Reward_RapidStrikes**
   - +30% attack speed
   - Slot Cost: 1
   - Stack: 3 levels

### Passive Stats (5 Rewards)
1. **DA_Reward_Vitality**
   - +50 max health
   - Slot Cost: 1
   - Stack: 3 levels

2. **DA_Reward_Swiftness**
   - +20% movement speed
   - Slot Cost: 1
   - Stack: 2 levels

3. **DA_Reward_Heavyweight**
   - +50% knockback force
   - Slot Cost: 1
   - Stack: 2 levels

4. **DA_Reward_Regeneration**
   - 2 HP/sec out of combat
   - Slot Cost: 2
   - Stack: 2 levels

5. **DA_Reward_Fortitude**
   - +50% poise
   - Slot Cost: 1
   - Stack: 3 levels

### Passive Abilities (5 Rewards)
1. **DA_Reward_SecondWind**
   - Revive once with 50% health
   - Slot Cost: 3
   - Stack: 1 level

2. **DA_Reward_Vampirism**
   - Heal 10% of damage dealt
   - Slot Cost: 2
   - Stack: 2 levels

3. **DA_Reward_Berserker**
   - +50% damage below 30% health
   - Slot Cost: 2
   - Stack: 1 level

4. **DA_Reward_Momentum**
   - Each hit +5% damage (max 50%)
   - Slot Cost: 2
   - Stack: 1 level

5. **DA_Reward_StationShield**
   - 50% damage goes to station
   - Slot Cost: 2
   - Stack: 1 level

### Interactables (5 Rewards)
1. **DA_Reward_ExplosiveValves**
   - Hack valves for 100 damage
   - Slot Cost: 1
   - Stack: 2 levels

2. **DA_Reward_GravityWells**
   - Create gravity anomalies
   - Slot Cost: 2
   - Stack: 1 level

3. **DA_Reward_TurretHack**
   - Convert turrets to allies
   - Slot Cost: 2
   - Stack: 1 level

4. **DA_Reward_EmergencyVent**
   - Launch enemies with vents
   - Slot Cost: 1
   - Stack: 2 levels

5. **DA_Reward_PowerSurge**
   - Overload panels to stun
   - Slot Cost: 1
   - Stack: 3 levels

## Task 2: Create Room DataAssets

### Room A: Engineering Bay
- **Theme**: Defense rewards
- **Enemy**: Heavy Technician
- **Hazard**: Electrical Surges
- **Special**: Shield generators

### Room B: Medical Ward
- **Theme**: Passive Stats
- **Enemy**: Corrupted Medic
- **Hazard**: Toxic Leak
- **Special**: Healing stations

### Room C: Weapons Lab
- **Theme**: Offense rewards
- **Enemy**: Security Chief
- **Hazard**: None
- **Special**: Weapon racks

### Room D: Command Center
- **Theme**: Passive Abilities
- **Enemy**: Station Commander
- **Hazard**: System Malfunction
- **Special**: Control panels

### Room E: Maintenance Shaft
- **Theme**: Interactables
- **Enemy**: Maintenance Drone
- **Hazard**: Low Gravity
- **Special**: Environmental objects

## Task 3: UI Widget Implementation

### WBP_RewardSelection
```
Layout:
┌─────────────────────────────────┐
│     Choose Your Reward          │
├─────────────────────────────────┤
│  ┌─────────┐    ┌─────────┐    │
│  │ Card 1  │    │ Card 2  │    │
│  │         │    │         │    │
│  │ [Icon]  │    │ [Icon]  │    │
│  │ Name    │    │ Name    │    │
│  │ Desc    │    │ Desc    │    │
│  │ Cost: 1 │    │ Cost: 2 │    │
│  │         │    │         │    │
│  │[SELECT] │    │[SELECT] │    │
│  └─────────┘    └─────────┘    │
│                                 │
│        [Skip Selection]         │
└─────────────────────────────────┘
```

### WBP_SlotManager
```
Layout:
┌─────────────────────────────────┐
│     Equipment Slots             │
├─────────────────────────────────┤
│  [1]  [2]  [3]                  │
│   ▢    ▢    ▢                   │
│                                 │
│  [4]  [5]  [6]                  │
│   ▢    ▢    ▢                   │
│                                 │
│  Drag to rearrange              │
│  Click to view details          │
└─────────────────────────────────┘
```

### WBP_RunProgress
```
Layout:
┌─────────────────────────────────┐
│  Room 1/5    Health: ████░      │
│              Station: ███░░      │
├─────────────────────────────────┤
│  Current Room: Engineering Bay   │
│  Enemy: Heavy Technician         │
│  Hazard: Electrical Surges      │
└─────────────────────────────────┘
```

## Task 4: Enemy Blueprint Creation

### BP_Enemy_HeavyTechnician
- High health, slow movement
- Uses shield abilities
- Drops Defense rewards

### BP_Enemy_CorruptedMedic
- Medium stats
- Heals during combat
- Drops Passive Stats

### BP_Enemy_SecurityChief
- High damage, aggressive
- Combo attacks
- Drops Offense rewards

### BP_Enemy_StationCommander
- Balanced stats
- Multiple phases
- Drops Passive Abilities

### BP_Enemy_MaintenanceDrone
- Fast, evasive
- Uses environment
- Drops Interactables

## Task 5: Integration Checklist

### GameMode Integration
- [ ] Hook up RunManagerComponent
- [ ] Connect reward selection flow
- [ ] Implement save triggers
- [ ] Add room transitions

### UI Flow
- [ ] Main Menu → Run Start
- [ ] Combat Victory → Reward Selection
- [ ] Reward Selected → Slot Management
- [ ] Room Complete → Transition
- [ ] Run Complete → Results Screen

### Audio Implementation
- [ ] Combat music per room
- [ ] Reward selection fanfare
- [ ] UI interaction sounds
- [ ] Environmental ambience
- [ ] Victory/defeat stingers

## Task 6: Testing & Balancing

### Reward Testing
- [ ] Each reward functions correctly
- [ ] Stacking works as intended
- [ ] Slot costs are balanced
- [ ] Synergies are fun but not broken

### Enemy Testing
- [ ] AI scaling works (slots + 1)
- [ ] Each enemy feels unique
- [ ] Difficulty curve is appropriate
- [ ] Pattern learning functions

### Run Testing
- [ ] 5 rooms complete properly
- [ ] Save/load works mid-run
- [ ] Rewards persist correctly
- [ ] No soft locks

## Console Commands for Testing

```cpp
// Reward Testing
Atlas_GiveReward [RewardTag]
Atlas_EnhanceReward [RewardTag]
Atlas_ClearAllRewards
Atlas_MaxOutSlots

// Room Testing
Atlas_SkipToRoom [RoomNumber]
Atlas_CompleteCurrentRoom
Atlas_ForceRoomReward [RewardTag]

// Enemy Testing
Atlas_SetEnemyPower [Level]
Atlas_ShowAIDifficulty
Atlas_ForceAIPattern [Pattern]

// Save Testing
Atlas_SaveGame
Atlas_LoadGame
Atlas_ClearSave
```

## Success Metrics

Phase 3 is complete when:
1. ✅ All 25 rewards are functional
2. ✅ 5 rooms with unique enemies exist
3. ✅ UI flows smoothly through entire run
4. ✅ Save/load works reliably
5. ✅ Game loop is fun and replayable
6. ✅ No critical bugs or soft locks
7. ✅ Performance is stable (60+ FPS)

## Estimated Timeline
- Week 1: Create all DataAssets
- Week 2: Build UI widgets
- Week 3: Implement enemy blueprints
- Week 4: Integration and testing
- Week 5: Polish and balancing