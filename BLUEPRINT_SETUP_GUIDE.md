# Atlas Blueprint Setup Guide - Phase 1

## Overview
This guide walks through setting up all the Blueprint assets needed to test the Phase 1 combat system implementation.

## Step 1: Create Data Asset Folders
1. In Content Browser, create the following folder structure:
   ```
   Content/
   ├── DataAssets/
   │   ├── Combat/
   │   │   ├── AttackData/
   │   │   └── Rules/
   ```

## Step 2: Create Combat Rules Data Asset

### 2.1 Create the Asset
1. Right-click in `Content/DataAssets/Combat/Rules/`
2. Select **Miscellaneous > Data Asset**
3. Choose **CombatRulesDataAsset** as the class
4. Name it: `DA_CombatRules_Default`

### 2.2 Configure Combat Rules
Double-click `DA_CombatRules_Default` and set these values:

**Combat Rules:**
- **Block Damage Reduction:** `0.4` (40% reduction)
- **Parry Window Duration:** `0.3` seconds
- **Vulnerability Multiplier:** `8.0`
- **Vulnerability Duration:** `1.0` second
- **Default Vulnerability Charges:** `1`
- **Max Poise:** `100`
- **Poise Regen Rate:** `15.0` per second
- **Poise Regen Delay:** `1.5` seconds
- **Focus Range:** `1000` units

Leave **Damage Multipliers** array empty for now.

## Step 3: Create Attack Data Assets

### 3.1 Create Jab Attack
1. Right-click in `Content/DataAssets/Combat/AttackData/`
2. Select **Miscellaneous > Data Asset**
3. Choose **AttackDataAsset** as the class
4. Name it: `DA_Attack_Jab`

Configure `DA_Attack_Jab`:
- **Attack Name:** `Jab`
- **Base Damage:** `5.0`
- **Knockback:** `100.0`
- **Stagger Damage:** `20.0`
- **Attack Tags:** 
  - Add: `Attack.Type.Jab`
  - Add: `Attack.Property.Blockable`
  - Add: `Attack.Property.Parryable`
- **Causes Ragdoll:** `False`
- **Attack Range:** `150.0`
- **Attack Angle:** `45.0`
- **Attack Montage:** Leave empty for now
- **Cooldown:** `0.0`
- **Attack Type Tag:** `Attack.Type.Jab`

### 3.2 Create Heavy Attack
1. Duplicate `DA_Attack_Jab` (Ctrl+W)
2. Rename to: `DA_Attack_Heavy`

Configure `DA_Attack_Heavy`:
- **Attack Name:** `Heavy`
- **Base Damage:** `15.0`
- **Knockback:** `500.0`
- **Stagger Damage:** `50.0`
- **Attack Tags:**
  - Add: `Attack.Type.Heavy`
  - Add: `Attack.Property.Blockable`
  - Add: `Attack.Property.Parryable`
- **Causes Ragdoll:** `True`
- **Attack Range:** `200.0`
- **Attack Angle:** `60.0`
- **Cooldown:** `1.0`
- **Attack Type Tag:** `Attack.Type.Heavy`

## Step 4: Configure BP_Player

### 4.1 Open BP_Player
1. Navigate to `Content/Blueprints/`
2. Open `BP_Player`

### 4.2 Configure Combat Component
1. Select the **CombatComponent** in the Components panel
2. In Details panel, find **Combat** section:
   - **Combat Rules:** Set to `DA_CombatRules_Default`
   - **Attack Data Map:** Add two entries:
     - Key: `Attack.Type.Jab` → Value: `DA_Attack_Jab`
     - Key: `Attack.Type.Heavy` → Value: `DA_Attack_Heavy`

### 4.3 Configure Health Component
1. Select the **HealthComponent** in the Components panel
2. In Details panel, find **Health** section:
   - **Max Health:** `100` (default)

### 4.4 Compile and Save

## Step 5: Configure BP_Enemy

### 5.1 Open BP_Enemy
1. Navigate to `Content/Blueprints/`
2. Open `BP_Enemy`

### 5.2 Configure Components
Repeat the same steps as BP_Player:
1. **CombatComponent:**
   - **Combat Rules:** `DA_CombatRules_Default`
   - **Attack Data Map:**
     - Key: `Attack.Type.Jab` → Value: `DA_Attack_Jab`
     - Key: `Attack.Type.Heavy` → Value: `DA_Attack_Heavy`

2. **HealthComponent:**
   - **Max Health:** `100`

### 5.3 Compile and Save

## Step 6: Set Up Input Mapping Context

### 6.1 Create Input Actions
In `Content/Blueprints/Input/Actions/`:

1. **IA_Move** (if not exists):
   - Right-click → Input → Input Action
   - Value Type: `Axis2D`

2. **IA_Look** (if not exists):
   - Value Type: `Axis2D`

3. **IA_LMB** (if not exists):
   - Value Type: `Digital (bool)`

4. **IA_RMB** (if not exists):
   - Value Type: `Digital (bool)`

5. **IA_Space** (if not exists):
   - Value Type: `Digital (bool)`

6. **IA_Q** (if not exists):
   - Value Type: `Digital (bool)`

7. **IA_E** (if not exists):
   - Value Type: `Digital (bool)`

### 6.2 Create/Update Input Mapping Context
1. In `Content/Blueprints/Input/`:
2. Create or open `IMC_Default`
3. Add mappings:

**Movement:**
- **IA_Move:**
  - W: Swizzle Input Axis Values (Y = 1.0)
  - S: Swizzle Input Axis Values (Y = -1.0)
  - A: Swizzle Input Axis Values (X = -1.0)
  - D: Swizzle Input Axis Values (X = 1.0)

**Camera:**
- **IA_Look:**
  - Mouse XY 2D-Axis

**Combat:**
- **IA_LMB:** Left Mouse Button
- **IA_RMB:** Right Mouse Button
- **IA_Space:** Space Bar
- **IA_Q:** Q Key
- **IA_E:** E Key

## Step 7: Verify Input Mapping in BP_Player

1. Open `BP_Player`
2. Check that these Input Actions are assigned in Details panel:
   - **Move Action:** `IA_Move`
   - **Look Action:** `IA_Look`
   - **Attack LMB Action:** `IA_LMB`
   - **Parry RMB Action:** `IA_RMB`
   - **Block Space Hold Action:** `IA_Space`
   - **Focus Q Hold Action:** `IA_Q`
   - **Heavy E Action:** `IA_E`
   - **Combat Mapping Context:** `IMC_Default`

## Step 8: Test in PIE (Play In Editor)

### 8.1 Basic Setup Test
1. Open `ThirdPersonMap` or create a test level
2. Place `BP_Player` in the level (delete default ThirdPersonCharacter if present)
3. Place `BP_Enemy` nearby (about 500 units away)
4. Hit Play

### 8.2 What to Test
**Movement:** 
- WASD should move the character
- Mouse should control camera

**Combat (Debug Output):**
- **LMB:** Should trigger jab attack (watch output log)
- **RMB:** Should trigger parry
- **Hold Space:** Should activate blocking
- **E:** Should trigger heavy attack

### 8.3 Debug Visualization
You should see on-screen debug messages when:
- Damage is dealt (yellow text)
- Health changes (red/green text)
- Character dies (red "DIED!" message)

## Step 9: Troubleshooting

### Common Issues:

**1. "CombatRules is null" errors:**
- Make sure `DA_CombatRules_Default` is assigned to CombatComponent

**2. Attacks don't work:**
- Verify Attack Data Map has entries for both attack types
- Check that gameplay tags match exactly (case-sensitive)

**3. Input doesn't work:**
- Ensure IMC_Default is assigned to Combat Mapping Context
- Verify Input Actions are created and mapped correctly

**4. No damage dealt:**
- Attacks need animation notifies to actually hit (Phase 2)
- For now, damage only flows through the system without actual hit detection

## Step 10: Verification Checklist

- [ ] CombatRules Data Asset created with correct values
- [ ] Jab Attack Data Asset created (5 damage)
- [ ] Heavy Attack Data Asset created (15 damage, ragdoll)
- [ ] BP_Player has Combat & Health components configured
- [ ] BP_Enemy has Combat & Health components configured
- [ ] Input Mapping Context created with all actions
- [ ] Can move and look around in PIE
- [ ] Combat inputs trigger (check output log)
- [ ] No crashes when pressing combat buttons

## Notes for Phase 2
Currently, attacks won't actually hit enemies because we haven't implemented:
- Animation montages with attack notifies
- Hitbox collision detection
- Animation-driven combat windows

These will be added in Phase 2: Animation Notifies + Hitbox System.

## Console Commands for Testing
You can use these console commands (~ key) for testing:
```
// Damage player (replace 50 with any value)
damage 50

// God mode
god

// Show debug info
showdebug combat
```

---

**Phase 1 Complete!** 
The damage pipeline is ready. Next phase will add actual hit detection through animation-driven hitboxes.