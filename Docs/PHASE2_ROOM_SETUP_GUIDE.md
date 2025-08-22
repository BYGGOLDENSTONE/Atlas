# Phase 2: Room System Blueprint Implementation Guide
**Engine**: UE 5.5 | **Date**: 2025-01-22

## Overview
This guide provides step-by-step instructions for implementing the room system in Unreal Engine 5.5 using the existing C++ framework.

## Prerequisites
- Completed Phase 1 (Core Foundation)
- Atlas C++ project compiled successfully
- Basic understanding of UE5 level streaming

---

## Part 1: Master Level Setup

### 1.1 Create Master Persistent Level
1. **File → New Level → Empty Level**
2. **Save As**: `Content/Maps/L_Atlas_Master`
3. **World Settings**:
   - Game Mode Override: `BP_AtlasGameMode`
   - Kill Z: -10000

### 1.2 Configure Lighting
1. **Add Directional Light**:
   - Intensity: 0.5 lux (emergency lighting)
   - Color: Orange-red (FFA500)
   - Atmosphere Sun Light: Disabled
   
2. **Add Sky Atmosphere**:
   - Rayleigh Scattering: Dark blue
   - Mie Scattering: Minimal
   
3. **Add Exponential Height Fog**:
   - Fog Density: 0.02
   - Fog Inscattering Color: Dark blue (001020)
   - Start Distance: 500

### 1.3 Add Post Process Volume
1. **Place Actors → Volumes → Post Process Volume**
2. **Settings**:
   - Infinite Extent: True
   - Bloom Intensity: 0.3
   - Vignette Intensity: 0.4
   - Color Grading: Cool temperature
   - Film Grain: 0.1

### 1.4 Create Room Loading Zone
1. **Add Box Collision** (Name: "RoomLoadingZone")
   - Location: (0, 0, 0)
   - Scale: (50, 50, 20)
2. **Add Player Start**:
   - Location: (0, 0, 100)
   - Rotation: (0, 0, 0)

---

## Part 2: Base Room Blueprint

### 2.1 Create BP_RoomBase
1. **Content Browser → Blueprint Class → Actor**
2. **Name**: `BP_RoomBase`
3. **Location**: `Content/Blueprints/Rooms/`

### 2.2 Add Components
```
Root (DefaultSceneRoot)
├── RoomBounds (BoxComponent)
│   └── Scale: (20, 20, 10)
├── PlayerSpawnPoint (ArrowComponent)
│   └── Location: (0, -800, 100)
├── EnemySpawnPoint (ArrowComponent)
│   └── Location: (0, 800, 100)
├── RewardSpawnPoint (ArrowComponent)
│   └── Location: (0, 0, 100)
├── ExitTrigger (BoxComponent)
│   └── Location: (0, 1000, 100)
│   └── Scale: (2, 2, 3)
└── HazardSpawnPoints (Array of ArrowComponents)
    ├── HazardPoint1: (-500, 0, 50)
    ├── HazardPoint2: (500, 0, 50)
    ├── HazardPoint3: (0, -500, 50)
    ├── HazardPoint4: (0, 500, 50)
    └── HazardPoint5: (0, 0, 200)
```

### 2.3 Add Variables
- **RoomDataAsset** (URoomDataAsset*): Reference to room configuration
- **IsRoomActive** (bool): Current room state
- **SpawnedEnemy** (AGameCharacterBase*): Reference to spawned enemy
- **SpawnedHazards** (TArray<AActor*>): List of active hazards

### 2.4 Blueprint Logic

#### BeginPlay Event
```blueprint
Event BeginPlay
├── Set IsRoomActive = false
└── Bind OnComponentBeginOverlap (ExitTrigger)
    └── Call CompleteRoom()
```

#### ActivateRoom Function
```blueprint
Function ActivateRoom(URoomDataAsset* RoomData)
├── Set RoomDataAsset = RoomData
├── Set IsRoomActive = true
├── Spawn Enemy at EnemySpawnPoint
│   └── Class: RoomData->UniqueEnemy
├── Apply Room Hazards
└── Start Combat Music
```

#### CompleteRoom Function
```blueprint
Function CompleteRoom()
├── Set IsRoomActive = false
├── Clear Spawned Hazards
├── Stop Combat Music
└── Broadcast OnRoomCompleted Event
```

---

## Part 3: Individual Room Levels

### 3.1 Engineering Bay Room

#### Create Level
1. **File → New Level → Empty Level**
2. **Save As**: `Content/Maps/Rooms/L_Room_EngineeringBay`
3. **Window → Levels → Set as Streaming Level**

#### Environment Setup
1. **Floor**: 
   - Static Mesh: `SM_Floor_Industrial`
   - Material: `M_Metal_Grated`
   - Size: 20x20 meters

2. **Walls**:
   - Use modular wall pieces
   - Height: 5 meters
   - Add pipe meshes along walls

3. **Props**:
   - Industrial machinery (non-blocking)
   - Steam vents at hazard points
   - Explosive barrels (3-5)
   - Work benches for cover

4. **Lighting**:
   - Flickering industrial lights
   - Red emergency lights
   - Steam particle effects

5. **Audio**:
   - Ambient: Industrial hum
   - Steam hiss sound cues
   - Metal creaking

### 3.2 Combat Arena Room

#### Create Level
1. **File → New Level → Empty Level**
2. **Save As**: `Content/Maps/Rooms/L_Room_CombatArena`

#### Environment Setup
1. **Floor**:
   - Circular arena (25m diameter)
   - Sand/dirt material
   - Blood stains decals

2. **Walls**:
   - High walls (8m)
   - Spectator stands (aesthetic)
   - Wall spikes for knockback damage

3. **Props**:
   - 4 Stone pillars (2m diameter)
   - Weapon racks (decorative)
   - Bones/debris scattered

4. **Lighting**:
   - Bright arena spotlights
   - Dramatic shadows
   - Dust particles in light beams

### 3.3 Medical Bay Room

#### Create Level
1. **File → New Level → Empty Level**
2. **Save As**: `Content/Maps/Rooms/L_Room_MedicalBay`

#### Environment Setup
1. **Floor**:
   - Clean white tiles
   - Material: `M_Medical_Floor`
   - Size: 15x20 meters

2. **Layout**:
   - Central corridor
   - Side rooms with glass walls
   - Medical equipment placement

3. **Props**:
   - Hospital beds (destructible)
   - Medical monitors
   - Chemical containers
   - Privacy screens (cover)

4. **Lighting**:
   - Bright fluorescent
   - Green emergency lights
   - Clean, sterile atmosphere

### 3.4 Cargo Hold Room

#### Create Level
1. **File → New Level → Empty Level**
2. **Save As**: `Content/Maps/Rooms/L_Room_CargoHold`

#### Environment Setup
1. **Floor**:
   - Industrial plating
   - Size: 30x20 meters
   - Multiple elevation levels

2. **Props**:
   - Shipping containers (climbable)
   - Stacked crates (cover)
   - Cargo nets
   - Crane system overhead

3. **Layout**:
   - Maze-like container placement
   - Multiple paths
   - Vertical gameplay areas

### 3.5 Bridge Room

#### Create Level
1. **File → New Level → Empty Level**
2. **Save As**: `Content/Maps/Rooms/L_Room_Bridge`

#### Environment Setup
1. **Floor**:
   - Command deck styling
   - Size: 25x15 meters
   - Two-tier with stairs

2. **Props**:
   - Captain's chair (center)
   - Console stations
   - Holographic displays
   - Large viewport (space view)

3. **Layout**:
   - Central command area
   - Side console stations
   - Upper observation deck

---

## Part 4: Room Manager Blueprint

### 4.1 Create BP_RoomManager
1. **Blueprint Class → Actor Component**
2. **Parent**: `URunManagerComponent`
3. **Name**: `BP_RoomManager`

### 4.2 Override Functions

#### InitializeRun
```blueprint
Function InitializeRun (Override)
├── Parent: InitializeRun
├── Load All Room Data Assets
│   ├── Engineering Bay
│   ├── Combat Arena
│   ├── Medical Bay
│   ├── Cargo Hold
│   └── Bridge
└── Randomize Room Order
```

#### LoadRoom
```blueprint
Function LoadRoom (Override)
├── Parent: LoadRoom
├── Unload Previous Room Level
├── Load New Room Level (Streaming)
│   └── Level Name: Room->RoomLevel
├── Wait for Level Load
└── Spawn Room Blueprint
    └── Call ActivateRoom on BP_RoomBase
```

#### SpawnRoomEnemy
```blueprint
Function SpawnRoomEnemy (Override)
├── Get Current Room Data
├── Get Enemy Spawn Point from Room
├── Spawn Enemy Actor
│   └── Class: RoomData->UniqueEnemy
├── Apply Enemy Scaling
│   └── Power = Player Slots + 1
└── Register Enemy Death Callback
```

---

## Part 5: Room Data Assets Creation

### 5.1 Create Data Assets

For each room, create a data asset:

1. **Right Click → Miscellaneous → Data Asset**
2. **Pick Class**: `URoomDataAsset`

### 5.2 DA_Room_EngineeringBay
```
Identity:
- RoomID: "EngineeringBay"
- RoomName: "Engineering Bay"
- Description: "The station's industrial heart"

Configuration:
- RoomTheme: Defense
- Difficulty: Medium
- EnvironmentalHazard: ElectricalSurges
- AppearOnLevels: [1, 2, 3]

Enemy:
- UniqueEnemy: BP_Enemy_Mechanic
- EnemyName: "Chief Engineer"
- EnemyBasePower: 3

Rewards:
- RewardPool: Defense + Interactables categories
- RewardChoiceCount: 2

Environment:
- RoomLevel: /Game/Maps/Rooms/L_Room_EngineeringBay
```

### 5.3 DA_Room_CombatArena
```
Identity:
- RoomID: "CombatArena"
- RoomName: "Combat Arena"
- Description: "Where warriors prove their worth"

Configuration:
- RoomTheme: Offense
- Difficulty: Hard
- EnvironmentalHazard: None
- AppearOnLevels: [2, 3, 4]

Enemy:
- UniqueEnemy: BP_Enemy_Gladiator
- EnemyName: "Arena Champion"
- EnemyBasePower: 4

Rewards:
- RewardPool: Offense + Defense categories
- RewardChoiceCount: 2
```

### 5.4 DA_Room_MedicalBay
```
Identity:
- RoomID: "MedicalBay"
- RoomName: "Medical Bay"
- Description: "Once a place of healing"

Configuration:
- RoomTheme: PassiveStats
- Difficulty: Easy
- EnvironmentalHazard: ToxicLeak
- AppearOnLevels: [1, 2]

Enemy:
- UniqueEnemy: BP_Enemy_Medic
- EnemyName: "Corrupted Medic"
- EnemyBasePower: 2
```

### 5.5 DA_Room_CargoHold
```
Identity:
- RoomID: "CargoHold"
- RoomName: "Cargo Hold"
- Description: "Cluttered storage facility"

Configuration:
- RoomTheme: Offense
- Difficulty: Medium
- EnvironmentalHazard: LowGravity
- AppearOnLevels: [2, 3, 4]

Enemy:
- UniqueEnemy: BP_Enemy_Brute
- EnemyName: "Cargo Loader"
- EnemyBasePower: 5
```

### 5.6 DA_Room_Bridge
```
Identity:
- RoomID: "Bridge"
- RoomName: "Command Bridge"
- Description: "The station's command center"

Configuration:
- RoomTheme: All
- Difficulty: Boss
- EnvironmentalHazard: SystemMalfunction
- AppearOnLevels: [5]

Enemy:
- UniqueEnemy: BP_Enemy_Captain
- EnemyName: "Station Commander"
- EnemyBasePower: 6
```

---

## Part 6: Integration & Testing

### 6.1 GameMode Configuration
1. Open `BP_AtlasGameMode`
2. Add `BP_RoomManager` component
3. Set default pawn: `BP_PlayerCharacter`

### 6.2 Level Streaming Setup
1. **Window → Levels**
2. Add Sub Levels:
   - L_Room_EngineeringBay
   - L_Room_CombatArena
   - L_Room_MedicalBay
   - L_Room_CargoHold
   - L_Room_Bridge
3. Set all to "Blueprint" streaming

### 6.3 Testing Checklist
- [ ] Master level loads correctly
- [ ] Room transitions work smoothly
- [ ] Each room has unique appearance
- [ ] Enemy spawns at correct location
- [ ] Hazard points are functional
- [ ] Exit trigger works
- [ ] Performance maintains 60+ FPS

### 6.4 Console Commands for Testing
```
# Start a new run
Atlas.Phase3.StartRun

# Load specific room
Atlas.Phase3.LoadRoom EngineeringBay

# Spawn enemy for current room
Atlas.Phase3.SpawnEnemy Mechanic

# Complete current room
Atlas.Phase3.CompleteRoom

# Skip to next room
Atlas.Phase3.NextRoom
```

---

## Part 7: Troubleshooting

### Common Issues

#### Rooms Not Loading
- Check level streaming setup
- Verify room level paths in data assets
- Ensure streaming volumes are configured

#### Enemy Not Spawning
- Verify enemy blueprint exists
- Check spawn point location
- Ensure RunManagerComponent is initialized

#### Performance Issues
- Reduce number of dynamic lights
- Use LODs for complex meshes
- Optimize particle effects
- Check draw call count (stat scenerendering)

#### Transition Problems
- Verify unload previous level is called
- Check for level streaming overlaps
- Ensure proper garbage collection

---

## Next Steps
After completing Phase 2:
1. Test complete room flow (1-5)
2. Verify save/load between rooms
3. Begin Phase 3: Combat & Animation
4. Create enemy blueprints for each room

---

## Additional Notes
- Keep rooms modular for easy iteration
- Use level streaming for performance
- Test frequently with stat commands
- Document any custom modifications