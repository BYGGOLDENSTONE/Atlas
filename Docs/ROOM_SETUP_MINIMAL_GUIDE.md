# Minimal Room Setup Guide - Get Atlas.Phase3.StartRun Working
**Date**: 2025-01-22

## The Problem
`Atlas.Phase3.StartRun` fails because:
1. RunManager exists now ✅
2. But `AllRoomDataAssets` array is empty ❌
3. No rooms = can't start run

## Quick Solution - Create Test Room Data Assets

### Step 1: Create Room Data Assets (In Editor)

1. **Create Folder**: `Content/Data/Rooms/`

2. **Create Test Room Data Asset**:
   - Right Click → **Miscellaneous** → **Data Asset**
   - Choose **URoomDataAsset** as class
   - Name it: `DA_TestRoom_1`

3. **Configure DA_TestRoom_1**:
   ```
   Identity:
   - RoomID: "TestRoom1"
   - RoomName: "Test Engineering Bay"
   
   Configuration:
   - RoomTheme: Defense
   - Difficulty: Easy
   - AppearOnLevels: [1, 2, 3, 4, 5]
   - bCanRepeat: true
   - RoomSelectionWeight: 1.0
   
   Enemy:
   - UniqueEnemy: None (for now)
   - EnemyBasePower: 1
   ```

4. **Create 4 More Test Rooms** (Copy and rename):
   - `DA_TestRoom_2` (RoomID: "TestRoom2")
   - `DA_TestRoom_3` (RoomID: "TestRoom3")
   - `DA_TestRoom_4` (RoomID: "TestRoom4")
   - `DA_TestRoom_5` (RoomID: "TestRoom5")

### Step 2: Add Rooms to RunManager

#### Option A: In Blueprint (If using BP_AtlasGameMode)
1. Open `BP_AtlasGameMode`
2. Find **RunManagerComponent** in components
3. In Details panel, find **All Room Data Assets**
4. Add all 5 test room data assets to the array

#### Option B: In C++ (Quick Test)
Add this to `RunManagerComponent::BeginPlay()`:

```cpp
void URunManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // TEMPORARY: Add test rooms if none configured
    if (AllRoomDataAssets.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No rooms configured, creating test rooms"));
        
        // Create 5 test room data assets
        for (int32 i = 1; i <= 5; i++)
        {
            URoomDataAsset* TestRoom = NewObject<URoomDataAsset>(this);
            TestRoom->RoomID = FName(*FString::Printf(TEXT("TestRoom%d"), i));
            TestRoom->RoomName = FText::FromString(FString::Printf(TEXT("Test Room %d"), i));
            TestRoom->Difficulty = ERoomDifficulty::Easy;
            TestRoom->RoomTheme = ERewardCategory::Defense;
            TestRoom->bCanRepeat = true;
            TestRoom->RoomSelectionWeight = 1.0f;
            TestRoom->AppearOnLevels = {1, 2, 3, 4, 5};
            TestRoom->EnemyBasePower = 1;
            
            AllRoomDataAssets.Add(TestRoom);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Created %d test rooms"), AllRoomDataAssets.Num());
    }
    
    // ... rest of BeginPlay
}
```

### Step 3: Alternative - Hardcode Test Rooms

If you just want to test quickly, add this to `RunManagerComponent.cpp`:

```cpp
URoomDataAsset* URunManagerComponent::SelectNextRoom()
{
    // TEMPORARY: If no rooms configured, create a dummy room
    if (AllRoomDataAssets.Num() == 0)
    {
        URoomDataAsset* DummyRoom = NewObject<URoomDataAsset>(this);
        DummyRoom->RoomID = "DummyRoom";
        DummyRoom->RoomName = FText::FromString("Dummy Test Room");
        DummyRoom->AppearOnLevels = {1, 2, 3, 4, 5};
        return DummyRoom;
    }
    
    // ... rest of original function
}
```

## Simplest Fix - Add to RunManagerComponent Constructor

```cpp
// In RunManagerComponent.cpp constructor
URunManagerComponent::URunManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Set default spawn point
    DefaultEnemySpawnPoint.SetLocation(FVector(0.0f, 500.0f, 100.0f));
    DefaultEnemySpawnPoint.SetRotation(FQuat::MakeFromEuler(FVector(0.0f, 0.0f, 180.0f)));
    
    // TEMPORARY: Create 5 dummy rooms for testing
    for (int32 i = 1; i <= 5; i++)
    {
        URoomDataAsset* TestRoom = CreateDefaultSubobject<URoomDataAsset>(
            *FString::Printf(TEXT("TestRoom_%d"), i));
        TestRoom->RoomID = FName(*FString::Printf(TEXT("TestRoom%d"), i));
        TestRoom->RoomName = FText::FromString(FString::Printf(TEXT("Test Room %d"), i));
        TestRoom->AppearOnLevels.Add(i);
        TestRoom->bCanRepeat = false;
        TestRoom->RoomSelectionWeight = 1.0f;
        AllRoomDataAssets.Add(TestRoom);
    }
}
```

## Testing After Setup

1. **Compile** if you added C++ code
2. **Run these commands**:
   ```
   # Check if rooms are loaded
   Atlas.Phase3.ListRooms
   
   # Start a run
   Atlas.Phase3.StartRun
   
   # Should now work!
   ```

## Expected Output
```
[PHASE3] Starting new run
[PHASE3] Initializing new run
[PHASE3] Randomized 5 rooms for run
[PHASE3] Loading room: Test Room 1
[PHASE3] New run started!
```

## Why This Works

The RunManager needs:
1. ✅ To exist (fixed earlier)
2. ✅ Room data assets in `AllRoomDataAssets` array
3. ✅ Rooms valid for current level (1-5)

Without rooms, `SelectNextRoom()` returns null and the run fails.

## Proper Setup (Later)

Once testing works, create real room data assets:
1. `DA_Room_EngineeringBay`
2. `DA_Room_CombatArena`
3. `DA_Room_MedicalBay`
4. `DA_Room_CargoHold`
5. `DA_Room_Bridge`

With proper configuration for enemies, rewards, and level references.

## Summary

The issue isn't the RunManager - it's that there are no rooms configured! Add test rooms either:
1. In Blueprint (AllRoomDataAssets array)
2. In C++ (temporary creation)
3. As actual Data Assets in Content folder

Then the run system will work!