# Atlas - Quick Reference

**Goal**: Build 1v1 Roguelite Dueler from GDD.txt | **Engine**: UE 5.5

## Target (per GDD.txt)
- 5-room runs with persistent rewards between runs
- Dual fail-states: Health (player dies) & Integrity (station destroyed)
- 25 unique rewards (5 per room type)
- Combat: Action-based with combos, vulnerability stacking
- Rooms: Bridge, Medical Bay, Engineering, Cargo Hold, Combat Arena

## ✅ Implemented Features

### Core Combat System
- **Action System**: Unified ActionInstance class with data-driven abilities
- **Animation-Driven Combat**: States managed by animation notifies
- **Damage Application**: Timed with animation frames via AttackNotify
- **Combat Actions**: BasicAttack, HeavyAttack, Block, Dash, Parry
- **State Management**: Attacking, Blocking, Dashing states with proper transitions
- **Hit Detection**: Animation-based collision detection at damage frames

### Room & Run System
- **5-Room Structure**: Bridge, Medical Bay, Engineering Bay, Cargo Hold, Combat Arena
- **Run Manager**: Handles room progression, enemy spawning, completion tracking
- **Equal Randomization**: Fair room selection without level-based bias
- **Auto-Progression**: Automatic advancement on enemy defeat

### Character Components
- **ActionManagerComponent**: 5 action slots, combo system, state tags
- **HealthComponent**: Health, poise, damage calculations
- **VulnerabilityComponent**: Damage multipliers (2x/4x/8x)
- **StationIntegrityComponent**: Secondary fail state system
- **SlotManagerComponent**: Reward equipment system

### Reward System
- **Target: 25 Unique Rewards**: 5 per room type (currently 3 test rewards per room)
- **Slot-Based Equipment**: Head, Body, Arms, Legs, Accessory slots
- **Persistent Upgrades**: Rewards carry between runs
- ✅ **Reward Selection UI**: Pure Slate modal UI after enemy defeat (SRewardSelectionWidget)
- ✅ **Inventory System**: Separated slot manager (compact display) and inventory (modal) widgets
  - **SSimpleSlotManagerWidget**: Always visible at bottom-right, shows equipped rewards
  - **SInventoryWidget**: Modal for reward slot selection after choosing reward
- ✅ **Room-Specific Pools**: Each room type offers themed rewards:
  - Bridge → Interactables (3 test rewards)
  - Cargo Hold → Defense (3 test rewards)
  - Medical Bay → Passive Abilities (3 test rewards)
  - Engineering Bay → Passive Stats (3 test rewards)
  - Combat Arena → Offense (3 test rewards)
- ✅ **Selection Blocking**: Room progression blocked until reward selected/cancelled
- ✅ **Test System**: Dynamic reward generation working without DataAssets
- ⏳ **Next Session**: Create proper DataAssets for all 25 rewards (5 per room)

### Environment Systems  
- **Hazards**: Fire, Electric, Toxic, Explosive, Gravity types
- **Destructible Objects**: Multi-stage destruction with debris
- **Interactables**: Valves, vents, terminals with gameplay effects

## UI System (Pure Slate - No UMG/Blueprint Dependencies)

### ✅ Implemented
- **SRewardSelectionWidget**: Modal reward selection after enemy defeat
- **SRunProgressWidget**: Room progression, health/poise/integrity bars (bottom-left, appears on Atlas.StartRun)
- **SEnemyHealthWidget**: Enemy name, health, and poise bars (top-center, working correctly)
- **SSimpleSlotManagerWidget**: Compact equipment display (bottom-right, always visible during run)
- **SInventoryWidget**: Modal for equipping rewards to slots (appears after reward selection)
- **Pure C++ Implementation**: All widgets created directly via SNew() and added with GEngine->GameViewport->AddViewportWidgetContent()
- **No Blueprint Setup Required**: Works entirely from C++ without any engine configuration

## ⏳ Not Yet Implemented

### Priority (Next Session)
1. **25 Final Rewards**: Design & create DataAssets (5 per room)
2. **Enemy Blueprints**: 5 room-specific enemies with AI
3. **Fix Slot Replacement**: Rewards should replace existing items in slots (currently not working)

### UI
- Action slot indicators
- Combo window display

### Other Systems
- **Enemies**: 5 room-specific types (need BP implementation)
- **AI**: Behavior trees, adaptive difficulty
- **VFX**: Combat effects, environmental visuals
- **Audio**: Combat sounds, music system
- **Soul Attack**: 50 unblockable damage (not yet assigned)

## Important Console Commands
```
# Run Management
Atlas.StartRun            # Start a new 5-room run (shows UI)
Atlas.CompleteRoom        # Complete current room and trigger reward selection
Atlas.GoToRoom [name]      # Teleport to specific room
Atlas.ShowMap             # Display current run progress

# Reward Selection
Atlas.SelectReward 0      # Select first reward option
Atlas.SelectReward 1      # Select second reward option
Atlas.CancelRewardSelection # Skip reward selection

# Combat Testing
Atlas_AssignAction [Slot] [ActionTag]
Atlas_ShowSlots

# Reward Testing  
Atlas.GiveReward [name]
Atlas.ShowRewards

# Environment Testing
Atlas.SpawnHazard [Type]
Atlas.TestEmergency [Type]
```

## Architecture
```
GameCharacterBase
├── ActionManagerComponent (combat/actions)
├── HealthComponent (health/poise)  
├── VulnerabilityComponent (damage mult)
├── SlotManagerComponent (rewards)
└── StationIntegrityComponent (station)
```

## Technical Rules
- NO GAS - Custom components only
- Data-Driven via DataAssets
- Animation-driven combat timing
- GameplayTags for all states
- `bCanBeInterrupted = false` for attacks

## File Locations
- **Actions**: `/Source/Atlas/Actions/`
- **Components**: `/Source/Atlas/Components/`
- **Data Assets**: `/Source/Atlas/Data/`
- **Console Cmds**: `/Source/Atlas/Debug/`

## Key Documentation
- `GDD.txt` - Full game design
- `IMPLEMENTATION_ROADMAP.md` - Development phases
- `BLUEPRINT_IMPLEMENTATION_ROADMAP.md` - Blueprint guide
- `ATLAS_CONSOLE_COMMANDS.txt` - All 100+ commands
- `GAMEPLAYTAG_MIGRATION_STATUS.md` - Tag migration (COMPLETE)

## Combat Values (Configured)
- **Basic Attack**: 5 damage, 0.5s duration
- **Heavy Attack**: 15 damage, 500 knockback, 1.0s duration  
- **Soul Attack**: 50 unblockable damage (not yet assigned)
- **Block**: 40% damage reduction
- **Parry Windows**: Perfect (100% reduction, 0.2s) / Late (50%, 0.1s)
- **Vulnerability Multipliers**: 2x/4x/8x based on stacks
- **Dash**: 400 units distance, 0.3s duration, 2s cooldown

## Recent Changes (Session 2025-02-01)
1. **Separated Slot Manager and Inventory Systems**:
   - Split SSlotManagerWidget into two distinct widgets
   - **SSimpleSlotManagerWidget**: Compact display at bottom-right (always visible)
   - **SInventoryWidget**: Modal for reward equipment (temporary)
   - Removed all mode-switching complexity
2. **Improved Widget Architecture**:
   - Single responsibility for each widget
   - Cleaner separation of persistent UI vs modals
   - Better flow: Reward Selection → Inventory → Equipment
3. **Fixed Critical Crashes**:
   - Added safety checks for delegate binding/unbinding
   - Fixed CurrentRoomActor validation issues
   - Added IsValid() checks throughout transition flow
4. **Known Issues**:
   - Slot replacement not working (rewards don't override existing items)
   - Enhancement system needs implementation

## Previous Session (2025-01-31)
1. **Fixed Critical UI Issues After Log Cleanup**:
   - Restored enemy health widget functionality that was accidentally removed
   - Fixed `CurrentRoomEnemy` not being set in RunManagerComponent
   - Enemy health/poise bars now properly update when enemy takes damage
   - Enemy health widget correctly hides when enemy is defeated
2. **Fixed Room Name Display**:
   - Restored `UpdateCurrentRoomInfo` calls in StartNewRun and TransitionToNextRoom
   - Run progress widget now shows actual room names instead of "Unknown Room"
   - Added fallback room data creation when no DataAsset match is found
3. **Improved Event Binding**:
   - Fixed health/poise change event subscriptions with IsAlreadyBound checks
   - Added OnDeath event subscription for proper enemy defeat handling
   - Simplified event handlers to update widgets directly with event values
4. **Cleaned Debug Output**:
   - Removed all LogTemp debug messages for cleaner console output
   - Preserved all functionality while removing debug clutter
5. **Fixed Enemy Name Display**:
   - Added smart fallback logic for enemy names
   - If DataAsset has EnemyName filled: uses that name
   - If DataAsset exists but EnemyName is empty: generates "[RoomType] Enemy"
   - If no DataAsset exists: creates temporary data with generated name
   - **Note**: DataAssets in Content/Dataassets/rooms/ need EnemyName field populated

**IMPORTANT: Compile in Unreal Editor (Ctrl+Alt+F11) or use Live Coding (Ctrl+Alt+F11) to see changes**

## Session (2025-01-30)
1. **Added Enemy Health UI System**:
   - Created SEnemyHealthWidget: Shows enemy name, health bar, and poise bar at top-center
   - Enemy health widget appears when enemy spawns, hides on defeat
   - Color-coded health/poise bars with stagger state display
2. **Enhanced Player HUD**:
   - Added player poise bar to SRunProgressWidget between health and integrity
   - Repositioned player stats to bottom-left (was top-left)
   - Player HUD shows: Room progress, Health, Poise, Station Integrity
3. **Fixed UI Update System**:
   - RunProgressWidget now properly created on GoToRoom if missing
   - Fixed delegate bindings (AddDynamic instead of lambdas)
   - Fixed timer lambda syntax with FTimerDelegate::CreateLambda
4. **Pure Slate Implementation**:
   - Removed all UMG dependencies
   - All UI widgets use SNew() and AddViewportWidgetContent()
   - No Blueprint setup required - works entirely from C++

## Session (2025-01-29)
1. **Removed all UMG widgets** - Converted to pure Slate
2. **Created Slate UI widgets**:
   - SRunProgressWidget: Shows room progress (1-5), health, integrity
   - SSlotManagerWidget: Equipment management (ready for integration)
3. **UI appears on run start** - No UI clutter before Atlas.StartRun
4. **Cleaned debug messages** - Removed all on-screen debug text


## Quick Status
- ✅ Core combat system working
- ✅ 5-room progression with reward selection
- ✅ UI system (Pure Slate, no UMG)
- ⏳ Need 25 final rewards (have 3 test per room)
- ⏳ Need enemy blueprints with AI
- ⏳ Need environment hazards/interactables