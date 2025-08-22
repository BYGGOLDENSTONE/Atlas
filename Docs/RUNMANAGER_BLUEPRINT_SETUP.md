# RunManager Blueprint Setup Guide
**Date**: 2025-01-22

## The Issue
The RunManager component might not be found because:
1. The GameMode in use is a Blueprint (BP_AtlasGameMode) not the C++ class
2. The Blueprint doesn't have the RunManagerComponent

## Solution Options

### Option 1: Update BP_AtlasGameMode (Recommended)
1. Open **BP_AtlasGameMode** in the editor
2. In the Components panel, click **Add Component**
3. Search for **RunManagerComponent**
4. Add it to the Blueprint
5. Compile and Save

### Option 2: Use C++ GameMode Directly
1. Open **Project Settings** → **Maps & Modes**
2. Change **Default GameMode** to **AtlasGameMode** (C++ class)
3. Or in **World Settings** for current level, set GameMode Override to AtlasGameMode

### Option 3: Create RunManager as Singleton (Alternative Approach)
If the above doesn't work, we can make RunManager accessible globally:

```cpp
// In RunManagerComponent.h
static URunManagerComponent* GetInstance(UWorld* World);

// In RunManagerComponent.cpp
static URunManagerComponent* GlobalRunManager = nullptr;

URunManagerComponent* URunManagerComponent::GetInstance(UWorld* World)
{
    if (!GlobalRunManager && World)
    {
        // Create it on first access
        AActor* DummyActor = World->SpawnActor<AActor>();
        GlobalRunManager = NewObject<URunManagerComponent>(DummyActor);
        GlobalRunManager->RegisterComponent();
    }
    return GlobalRunManager;
}
```

## Debugging Steps

After running `Atlas.Phase3.StartRun`, check the error message:

1. **"GEngine is null"** - Engine not initialized (shouldn't happen)
2. **"No world contexts"** - No game world loaded
3. **"World is null"** - Failed to get world
4. **"GameMode is null"** - No GameMode in current world
5. **"GameMode is not AAtlasGameMode"** - Using wrong GameMode class
6. **"RunManager component is null"** - Component not created

## Verification

### Check Current GameMode
Run this console command:
```
DisplayAll GameMode
```

This will show what GameMode class is currently active.

### Check for RunManager
After adding to Blueprint or changing GameMode:
```
Atlas.Phase3.QuickTest
```

Should show "RunManager found and active"

## Blueprint vs C++ Components

### If using Blueprint GameMode:
- Components must be added in Blueprint editor
- C++ components created in constructor won't appear automatically
- Need to manually add each component

### If using C++ GameMode:
- Components created in constructor work immediately
- No Blueprint editing needed
- But loses Blueprint flexibility

## Recommended Setup

1. **Keep BP_AtlasGameMode** for flexibility
2. **Add RunManagerComponent** in Blueprint
3. **Add any other needed components** (SaveManager, etc.)
4. **Test with console commands**

This allows both C++ logic and Blueprint customization.