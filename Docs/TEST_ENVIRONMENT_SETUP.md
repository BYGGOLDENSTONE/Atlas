# Test Environment Setup Guide
**Date**: 2025-01-22

## Current Status
✅ **Core systems working** - RunManager, Rooms, Progression
⚠️ **Test environment limitations** - No GameMode, abstract RoomBase, no enemies

## Issue 1: No GameMode in ThirdPersonMap

### Solution A: Set Default GameMode
1. **Project Settings** → **Maps & Modes**
2. Set **Default GameMode**: `AtlasGameMode` or `BP_AtlasGameMode`
3. Restart the level

### Solution B: Override in World Settings
1. Open **ThirdPersonMap**
2. **World Settings** panel
3. **GameMode Override**: `AtlasGameMode`

## Issue 2: RoomBase is Abstract

The test rooms try to spawn RoomBase which is abstract. We need to use a concrete class.

### Quick Fix in Code:
Update `RoomStreamingManager::SpawnRoomActor()` to use a default concrete class when RoomClass is abstract:

```cpp
// In RoomStreamingManager.cpp
ARoomBase* URoomStreamingManager::SpawnRoomActor(URoomDataAsset* RoomData, TSubclassOf<ARoomBase> RoomClass)
{
    // If RoomClass is abstract or null, use a default concrete class
    if (!RoomClass || RoomClass->HasAnyClassFlags(CLASS_Abstract))
    {
        // Try to use EngineeringBayRoom as default
        RoomClass = AEngineeringBayRoom::StaticClass();
        UE_LOG(LogTemp, Warning, TEXT("Using EngineeringBayRoom as default room class"));
    }
    
    // ... rest of function
}
```

## Issue 3: No Enemy Classes

Test rooms don't have enemy classes assigned.

### Quick Fix:
Create a simple test enemy in the test room creation:

```cpp
// In RunManagerComponent::BeginPlay()
TestRoom->UniqueEnemy = AGameCharacterBase::StaticClass(); // or any enemy class
```

## For Proper Testing

### Create a Test Map
1. **File** → **New Level** → **Default**
2. Save as `L_AtlasTest`
3. **World Settings**:
   - GameMode Override: `AtlasGameMode`
4. **Add Player Start**

### Use the Test Map
1. Open `L_AtlasTest`
2. Play in Editor
3. Run test commands

## Working Test Commands (Despite Warnings)

These work even with the warnings:

```
// Start and progress through run
Atlas.Phase3.StartRun
Atlas.Phase3.ShowRunProgress
Atlas.Phase3.CompleteRoom
Atlas.Phase3.ShowRunProgress
Atlas.Phase3.CompleteRoom
Atlas.Phase3.ShowRunProgress

// Test rewards
Atlas.Phase3.ListRewards
Atlas.Phase3.GiveReward Berserker
Atlas.Phase3.ShowSlots

// Test integrity
Atlas.ShowIntegrityStatus
Atlas.DamageIntegrity 20
Atlas.ShowIntegrityStatus
```

## Summary

The system is **functionally working** despite the warnings:
- ✅ Rooms progress correctly
- ✅ State management works
- ✅ Commands execute properly
- ⚠️ Visual/spawning issues due to test environment

For production:
1. Use proper game mode setup
2. Create concrete room classes
3. Assign enemy blueprints
4. Create actual level assets

But for testing the C++ systems, it's working as expected!