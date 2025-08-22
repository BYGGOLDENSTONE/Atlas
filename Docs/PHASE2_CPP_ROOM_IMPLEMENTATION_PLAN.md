# Phase 2: C++ Room System Implementation Plan
**Engine**: UE 5.5 | **Date**: 2025-01-22

## Overview
This document outlines what can and should be implemented in C++ for the room system, minimizing Blueprint dependency.

---

## Current C++ Components Analysis

### Already Implemented:
1. **URoomDataAsset** - Room configuration data structure
2. **URunManagerComponent** - Run progression and state management  
3. **ARoomBase** - Base room actor class with spawn points
4. **Enemy/Reward/Action Systems** - Full C++ implementation

### What Can Be Done in C++:

## 1. Room Streaming Management
```cpp
// RoomStreamingManager.h
class URoomStreamingManager : public UObject
{
    - Handle level streaming/unloading
    - Manage async level loading
    - Preload adjacent rooms
    - Handle streaming callbacks
    - Manage world composition
};
```

## 2. Specific Room Classes (Inherit from ARoomBase)
```cpp
// Each room has unique C++ logic
class AEngineeringBayRoom : public ARoomBase
{
    - Steam vent spawning logic
    - Pipe burst sequences
    - Machinery interaction
    - Industrial hazard patterns
};

class ACombatArenaRoom : public ARoomBase  
{
    - Wall spike damage zones
    - Crowd cheer audio management
    - Arena-specific combat modifiers
    - Gladiatorial event triggers
};

class AMedicalBayRoom : public ARoomBase
{
    - Chemical leak spawning
    - Emergency station interactions
    - Healing zone management
    - Sterile field effects
};

class ACargoHoldRoom : public ARoomBase
{
    - Container physics setup
    - Crane movement system
    - Multi-level navigation
    - Cargo net interactions
};

class ABridgeRoom : public ARoomBase
{
    - Viewport breach mechanics
    - Holographic display updates
    - Command console interactions
    - Two-tier combat zones
};
```

## 3. Hazard System Implementation
```cpp
class AHazardBase : public AActor
{
    - Damage dealing logic
    - Activation patterns
    - Visual/audio feedback
    - Player proximity detection
};

// Specific hazard types
class ASteamVentHazard : public AHazardBase
class AElectricalSurgeHazard : public AHazardBase
class AToxicLeakHazard : public AHazardBase
class AGravityAnomalyHazard : public AHazardBase
class AHullBreachHazard : public AHazardBase
```

## 4. Interactable System
```cpp
class AInteractableBase : public AActor
{
    - Interaction prompt system
    - Activation requirements
    - Cooldown management
    - Reward integration
};

// Specific interactables
class AExplosiveBarrel : public AInteractableBase
class ATurretStation : public AInteractableBase
class AEmergencyVentControl : public AInteractableBase
class APowerSurgePanel : public AInteractableBase
class AGravityWellGenerator : public AInteractableBase
```

## 5. Room Transition Effects
```cpp
class URoomTransitionManager : public UActorComponent
{
    - Screen fade effects
    - Loading screens
    - Transition animations
    - Audio crossfading
    - Memory cleanup
};
```

## 6. Environmental Effect System
```cpp
class UEnvironmentalEffectComponent : public UActorComponent
{
    - Apply room-specific physics changes
    - Manage atmospheric effects
    - Handle integrity degradation
    - Control lighting changes
    - Manage post-process effects
};
```

## 7. Room Audio Manager
```cpp
class URoomAudioComponent : public UActorComponent
{
    - Combat music management
    - Ambient sound control
    - Dynamic audio mixing
    - 3D sound positioning
    - Audio occlusion handling
};
```

## 8. Room Enemy Manager
```cpp
class URoomEnemyManager : public UActorComponent
{
    - Enemy spawn timing
    - Difficulty scaling
    - AI behavior modification
    - Power level calculation
    - Death handling
};
```

## 9. Room Reward Manager
```cpp
class URoomRewardManager : public UActorComponent
{
    - Reward pool selection
    - Weighted randomization
    - Tier scaling
    - UI presentation calls
    - Persistence handling
};
```

## 10. Room Debug System
```cpp
class URoomDebugComponent : public UActorComponent
{
    - Visual debug overlays
    - Spawn point visualization
    - Hazard zone display
    - Performance metrics
    - Console command integration
};
```

---

## What Still Requires Blueprint/Editor:

1. **Level Geometry** - Physical room layouts
2. **Static Mesh Placement** - Props and decorations
3. **Material Assignments** - Visual appearance
4. **Lighting Setup** - Light placement and settings
5. **Particle Effect Placement** - VFX positioning
6. **Navigation Mesh** - AI pathfinding
7. **Collision Volumes** - Physical boundaries

---

## Implementation Priority:

### Phase 1: Core Systems (Immediate)
1. ✅ ARoomBase (Already done)
2. Specific room classes (5 rooms)
3. RoomStreamingManager
4. Basic hazard classes

### Phase 2: Enhancement Systems
1. InteractableBase and specific types
2. EnvironmentalEffectComponent
3. RoomAudioComponent
4. RoomTransitionManager

### Phase 3: Polish Systems
1. RoomEnemyManager (enhanced AI)
2. RoomRewardManager (advanced selection)
3. RoomDebugComponent
4. Performance optimizations

---

## Benefits of C++ Implementation:

1. **Performance** - Better optimization for room transitions
2. **Debugging** - Easier to debug logic in C++
3. **Version Control** - Code changes tracked better than Blueprint
4. **Reusability** - Inheritance and composition patterns
5. **Testing** - Unit tests possible for C++ code
6. **Consistency** - All game logic in one language
7. **Modularity** - Clean separation of concerns

---

## Next Steps:

1. Create specific room classes inheriting from ARoomBase
2. Implement RoomStreamingManager for level management
3. Create hazard actor classes
4. Update RunManagerComponent to use new C++ room classes
5. Add console commands for testing each system
6. Create minimal Blueprint wrappers only for asset references

This approach keeps 90% of logic in C++, with Blueprints only used for:
- Asset references (meshes, sounds, materials)
- Level layout (geometry placement)
- Visual tweaking (particle positions, light colors)