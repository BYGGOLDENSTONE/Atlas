# Atlas Comprehensive Test Commands
**Date**: 2025-01-22
**Purpose**: Test all implemented systems in logical order

## Copy-Paste Test Script

Copy and paste this entire block into the console to test everything:

```
// ========================================
// BASIC SYSTEM VERIFICATION
// ========================================

// Test basic command registration
Atlas.ShowIntegrityStatus

// Test that Phase3 commands are registered
Atlas.Phase3.QuickTest

// Test that Phase4 commands are registered  
Atlas.Phase4.TestAll

// ========================================
// ROOM SYSTEM & RUN MANAGER
// ========================================

// Start a new run (creates test rooms if needed)
Atlas.Phase3.StartRun

// Show current run progress
Atlas.Phase3.ShowRunProgress

// List available rooms
Atlas.Phase3.ListRooms

// Complete current room
Atlas.Phase3.CompleteRoom

// Show run progress after completion
Atlas.Phase3.ShowRunProgress

// ========================================
// REWARD SYSTEM
// ========================================

// List all available rewards
Atlas.Phase3.ListRewards

// Show current equipped slots
Atlas.Phase3.ShowSlots

// Give a test reward
Atlas.Phase3.GiveReward Berserker

// Show slots again to see the reward
Atlas.Phase3.ShowSlots

// Give another reward
Atlas.Phase3.GiveReward IronSkin

// Clear all slots
Atlas.Phase3.ClearAllSlots

// Verify slots are cleared
Atlas.Phase3.ShowSlots

// ========================================
// STATION INTEGRITY SYSTEM
// ========================================

// Show current integrity
Atlas.ShowIntegrityStatus

// Damage integrity
Atlas.DamageIntegrity 20

// Show status after damage
Atlas.ShowIntegrityStatus

// Set integrity to specific percent
Atlas.SetIntegrityPercent 75

// Verify the change
Atlas.ShowIntegrityStatus

// Reset integrity to full
Atlas.ResetIntegrity

// ========================================
// ENVIRONMENTAL HAZARDS (PHASE 4)
// ========================================

// Test all Phase 4 systems
Atlas.Phase4.TestAll

// Spawn specific hazard types
Atlas.Phase4.SpawnHazard Electrical
Atlas.Phase4.SpawnHazard Steam
Atlas.Phase4.SpawnHazard Toxic

// Test emergency event
Atlas.Phase4.TriggerEmergency PowerOutage

// Clear all hazards
Atlas.Phase4.ClearHazards

// ========================================
// COMBAT & VULNERABILITY
// ========================================

// Apply vulnerability tiers
Atlas.ApplyStunned
Atlas.ApplyCrippled
Atlas.ApplyExposed

// Clear vulnerability
Atlas.ClearVulnerability

// Show vulnerability info
Atlas.ShowVulnerabilityInfo

// ========================================
// ACTION SYSTEM
// ========================================

// List available actions
Atlas.ListActions

// Show current action slots
Atlas.ShowSlots

// Assign an action to a slot
Atlas.AssignAction 0 Action.Combat.BasicAttack

// Show slots to verify
Atlas.ShowSlots

// Clear the slot
Atlas.ClearSlot 0

// ========================================
// FOCUS MODE
// ========================================

// Toggle focus mode
Atlas.ToggleFocusMode

// Show focus debug info
Atlas.ShowFocusDebug

// Toggle off
Atlas.ToggleFocusMode

// ========================================
// COMPLETE RUN TEST
// ========================================

// Start fresh run
Atlas.Phase3.StartRun

// Give some rewards for testing
Atlas.Phase3.GiveReward Berserker
Atlas.Phase3.GiveReward IronSkin
Atlas.Phase3.GiveReward HealthBoost

// Show equipped rewards
Atlas.Phase3.ShowSlots

// Complete first room
Atlas.Phase3.CompleteRoom

// Continue through rooms (repeat 4 more times)
Atlas.Phase3.CompleteRoom
Atlas.Phase3.CompleteRoom
Atlas.Phase3.CompleteRoom
Atlas.Phase3.CompleteRoom

// Show final run status
Atlas.Phase3.ShowRunProgress

// ========================================
// SAVE/LOAD TEST
// ========================================

// Save current rewards
Atlas.Phase3.SaveRewards

// Clear all slots
Atlas.Phase3.ClearAllSlots

// Verify cleared
Atlas.Phase3.ShowSlots

// Load saved rewards
Atlas.Phase3.LoadRewards

// Verify loaded
Atlas.Phase3.ShowSlots
```

## Individual Test Sections

### 1. Quick Smoke Test (Minimal)
```
Atlas.Phase3.QuickTest
Atlas.Phase3.StartRun
Atlas.Phase3.ShowRunProgress
```

### 2. Room System Only
```
Atlas.Phase3.StartRun
Atlas.Phase3.ListRooms
Atlas.Phase3.ShowRunProgress
Atlas.Phase3.CompleteRoom
Atlas.Phase3.ShowRunProgress
```

### 3. Reward System Only
```
Atlas.Phase3.ListRewards
Atlas.Phase3.ShowSlots
Atlas.Phase3.GiveReward Berserker
Atlas.Phase3.GiveReward IronSkin
Atlas.Phase3.ShowSlots
Atlas.Phase3.ClearAllSlots
```

### 4. Station Integrity Only
```
Atlas.ShowIntegrityStatus
Atlas.DamageIntegrity 25
Atlas.ShowIntegrityStatus
Atlas.SetIntegrityPercent 50
Atlas.ShowIntegrityStatus
Atlas.ResetIntegrity
```

### 5. Environmental Hazards Only
```
Atlas.Phase4.TestAll
Atlas.Phase4.SpawnHazard Electrical
Atlas.Phase4.TriggerEmergency PowerOutage
Atlas.Phase4.ClearHazards
```

## Expected Results

### ✅ Success Indicators:
- All commands execute without "command not found"
- RunManager is found (via one of the methods)
- Test rooms are created if none exist
- Run starts and progresses through rooms
- Rewards can be equipped and shown
- Station integrity changes properly
- Hazards spawn without errors

### ⚠️ Warning Messages (OK):
- "No rooms configured, creating test rooms" - Expected on first run
- "RunManager from global singleton" - OK, fallback working
- "Created RunManagerComponent at runtime" - OK, fallback working

### ❌ Error Messages (Problems):
- "Command not found" - Commands not registered
- "RunManager not found" - All fallbacks failed
- "Failed to select first room" - Room system broken
- "SlotManager not found" - Player components missing

## Debug Commands

If something fails, use these for diagnosis:

```
// Check what GameMode is active
DisplayAll GameMode

// Check if player exists
DisplayAll PlayerCharacter

// Check console command registration
Atlas.Phase3.QuickTest

// Force verbose logging
Atlas.Phase3.ShowRunProgress
```

## Performance Test

Test system under load:
```
// Spawn multiple hazards
Atlas.Phase4.SpawnHazard Electrical
Atlas.Phase4.SpawnHazard Steam
Atlas.Phase4.SpawnHazard Toxic
Atlas.Phase4.SpawnHazard Electrical
Atlas.Phase4.SpawnHazard Steam

// Rapid room completion
Atlas.Phase3.StartRun
Atlas.Phase3.CompleteRoom
Atlas.Phase3.CompleteRoom
Atlas.Phase3.CompleteRoom
Atlas.Phase3.CompleteRoom
Atlas.Phase3.CompleteRoom
```

## Save System Test

Test persistence:
```
// Setup state
Atlas.Phase3.StartRun
Atlas.Phase3.GiveReward Berserker
Atlas.Phase3.GiveReward IronSkin
Atlas.Phase3.ShowSlots

// Save
Atlas.Phase3.SaveRewards

// Clear
Atlas.Phase3.ClearAllSlots
Atlas.Phase3.ShowSlots

// Restore
Atlas.Phase3.LoadRewards
Atlas.Phase3.ShowSlots
```

This comprehensive test suite covers all implemented systems!