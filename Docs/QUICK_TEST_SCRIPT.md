# Quick Test Script - Phase 2 Room System
**Copy and paste these commands one by one or all together**

## Essential Test Commands (Copy All)

```
// Test basic registration
Atlas.ShowIntegrityStatus

// Test Phase3 command registration
Atlas.Phase3.QuickTest

// Start a run (this tests RunManager + Room creation)
Atlas.Phase3.StartRun

// Check run status
Atlas.Phase3.ShowRunProgress

// List created rooms
Atlas.Phase3.ListRooms

// Test reward system
Atlas.Phase3.ListRewards
Atlas.Phase3.GiveReward Berserker
Atlas.Phase3.ShowSlots

// Complete current room and progress
Atlas.Phase3.CompleteRoom
Atlas.Phase3.ShowRunProgress

// Test Phase 4
Atlas.Phase4.TestAll
```

## What Each Command Tests

| Command | Tests | Expected Result |
|---------|-------|-----------------|
| `Atlas.ShowIntegrityStatus` | Basic commands work | Shows station integrity |
| `Atlas.Phase3.QuickTest` | Phase3 registered | Runs quick test suite |
| `Atlas.Phase3.StartRun` | RunManager + Rooms | Creates test rooms, starts run |
| `Atlas.Phase3.ShowRunProgress` | Run state | Shows current room 1/5 |
| `Atlas.Phase3.ListRooms` | Room creation | Lists 5 test rooms |
| `Atlas.Phase3.CompleteRoom` | Room progression | Advances to next room |

## Minimal Smoke Test (3 commands)

```
Atlas.Phase3.StartRun
Atlas.Phase3.ShowRunProgress
Atlas.Phase3.CompleteRoom
```

## If StartRun Fails - Debug Commands

```
// See detailed debug output
Atlas.Phase3.StartRun

// Check what's in the log
// Look for:
// - [PHASE3] World found
// - [PHASE3] GameMode class
// - [PHASE3] Got RunManager from [method]
// - RunManagerComponent: Created 5 test rooms
```

## Success Output Example

```
> Atlas.Phase3.StartRun
[PHASE3] Got RunManager from global singleton (fallback)
RunManagerComponent: No rooms configured, creating 5 test rooms
  Created test room: Test Room 1
  Created test room: Test Room 2
  Created test room: Test Room 3
  Created test room: Test Room 4
  Created test room: Test Room 5
[PHASE3] Starting new run
[PHASE3] Loading room: Test Room 1
[PHASE3] New run started!

> Atlas.Phase3.ShowRunProgress
[PHASE3] === RUN PROGRESS ===
[PHASE3] Current Level: 1/5
[PHASE3] Current Room: Test Room 1
[PHASE3] Rooms Completed: 0
```

Copy and run these to verify everything works!