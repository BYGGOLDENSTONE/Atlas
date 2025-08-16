# Gameplay Tags Setup Guide for Unreal Engine

## Overview
We've created a CSV file with gameplay tags, but it needs to be imported into Unreal Engine as a Data Table. Here's how to set it up properly.

## Step 1: Import the CSV as a Data Table

### 1.1 Create Data Table
1. Open Unreal Editor
2. In Content Browser, navigate to `Content/Data/` (create this folder if it doesn't exist)
3. Right-click in the folder → **Import to /Game/Data/**
4. Navigate to `D:\Unreal Projects\Atlas\Config\GameplayTags.csv`
5. Select the CSV file and click **Open**

### 1.2 Configure Import Settings
When the import dialog appears:
1. **Row Type:** Select `GameplayTagTableRow`
   - If this doesn't appear, click **Cancel** and follow Alternative Method below
2. Click **Import**
3. Name the asset: `DT_GameplayTags`

### Alternative Method (if GameplayTagTableRow isn't available):
1. Right-click in Content Browser → **Miscellaneous → Data Table**
2. Choose **GameplayTagTableRow** as the Row Structure
3. Name it: `DT_GameplayTags`
4. Double-click to open it
5. Click **Reimport** button in toolbar
6. Select the CSV file from `Config/GameplayTags.csv`

## Step 2: Configure Project Settings

### 2.1 Open Gameplay Tags Settings
1. Go to **Edit → Project Settings**
2. Navigate to **Project → Gameplay Tags**

### 2.2 Add the Data Table
1. Find **Gameplay Tag Table List**
2. Click the **+** button to add an entry
3. Set the path to: `/Game/Data/DT_GameplayTags`
4. Click the arrow/folder icon to browse and select your imported Data Table

### 2.3 Verify Tags are Loaded
1. Still in Gameplay Tags settings, look for **Gameplay Tag List**
2. You should see all your tags listed:
   - Attack.Type.Jab
   - Attack.Type.Heavy
   - Attack.Property.Blockable
   - Attack.Property.Parryable
   - Combat.State.Blocking
   - Combat.State.Vulnerable
   - etc.

## Step 3: Alternative - Direct INI Configuration

If you prefer to skip the Data Table and use INI directly:

### 3.1 Update DefaultGameplayTags.ini
The file at `Config/DefaultGameplayTags.ini` should contain:

```ini
[/Script/GameplayTags.GameplayTagsSettings]
ImportTagsFromConfig=True
WarnOnInvalidTags=True
ClearInvalidTags=False
AllowEditorTagUnloading=True
AllowGameTagUnloading=False
FastReplication=False
InvalidTagCharacters="\"\',"
NumBitsForContainerSize=6
NetIndexFirstBitSegment=16

; Add each tag directly here instead of using CSV
+GameplayTagList=(Tag="Attack.Type.Jab",DevComment="Basic jab attack")
+GameplayTagList=(Tag="Attack.Type.Heavy",DevComment="Heavy attack with knockback")
+GameplayTagList=(Tag="Attack.Type.Special",DevComment="Special attack")
+GameplayTagList=(Tag="Attack.Type.Soul",DevComment="Soul attack - unblockable and unparryable")
+GameplayTagList=(Tag="Attack.Type.Catch",DevComment="Catch special - anti-kiting attack")
+GameplayTagList=(Tag="Attack.Property.Blockable",DevComment="Attack can be blocked")
+GameplayTagList=(Tag="Attack.Property.Parryable",DevComment="Attack can be parried")
+GameplayTagList=(Tag="Attack.Property.Unblockable",DevComment="Attack cannot be blocked")
+GameplayTagList=(Tag="Attack.Property.Unparryable",DevComment="Attack cannot be parried")
+GameplayTagList=(Tag="Combat.State.Attacking",DevComment="Character is currently attacking")
+GameplayTagList=(Tag="Combat.State.Blocking",DevComment="Character is blocking")
+GameplayTagList=(Tag="Combat.State.Parrying",DevComment="Character is parrying")
+GameplayTagList=(Tag="Combat.State.Vulnerable",DevComment="Character is vulnerable")
+GameplayTagList=(Tag="Combat.State.Staggered",DevComment="Character is staggered")
+GameplayTagList=(Tag="Combat.State.Invincible",DevComment="Character has i-frames")
+GameplayTagList=(Tag="Combat.State.Dead",DevComment="Character is dead")
+GameplayTagList=(Tag="Damage.Type.Physical",DevComment="Physical damage")
+GameplayTagList=(Tag="Damage.Type.Fire",DevComment="Fire damage")
+GameplayTagList=(Tag="Damage.Type.Electric",DevComment="Electric damage")
+GameplayTagList=(Tag="Damage.Type.Poison",DevComment="Poison damage")
+GameplayTagList=(Tag="Damage.Multiplier.Vulnerability",DevComment="Vulnerability multiplier")
+GameplayTagList=(Tag="Character.Type.Player",DevComment="Player character")
+GameplayTagList=(Tag="Character.Type.Enemy",DevComment="Enemy character")
+GameplayTagList=(Tag="Character.Type.Boss",DevComment="Boss character")
+GameplayTagList=(Tag="Ability.Focus",DevComment="Focus mode active")
+GameplayTagList=(Tag="Ability.SoftLock",DevComment="Soft lock targeting")
+GameplayTagList=(Tag="Interactable.Type.Vent",DevComment="Vent interactable")
+GameplayTagList=(Tag="Interactable.Type.Valve",DevComment="Valve interactable")
+GameplayTagList=(Tag="Interactable.State.Ready",DevComment="Interactable ready")
+GameplayTagList=(Tag="Interactable.State.Cooldown",DevComment="Interactable on cooldown")
```

### 3.2 Restart the Editor
After modifying the INI file:
1. Save the file
2. Close Unreal Editor
3. Reopen the project
4. Tags should now be available

## Step 4: Verify Tags in Editor

### 4.1 Open Gameplay Tags Editor
1. **Window → Developer Tools → Gameplay Tags**
2. You should see a tree view of all your tags organized by category

### 4.2 Test Tag Selection
1. Open any Blueprint (like BP_Player)
2. In the CombatComponent details
3. Try to add tags to test - you should see them in the dropdown

## Step 5: Using Tags in Blueprints

### 5.1 In Data Assets
When you create Attack Data Assets:
1. Open the Data Asset (e.g., DA_Attack_Jab)
2. In **Attack Tags** array, click **+**
3. Click the dropdown - your tags should appear
4. Select appropriate tags like:
   - Attack.Type.Jab
   - Attack.Property.Blockable
   - Attack.Property.Parryable

### 5.2 In Blueprint Graphs
To use tags in Blueprint logic:
1. Right-click in graph → search "Gameplay Tag"
2. Use nodes like:
   - **Make Literal Gameplay Tag** - to create a tag reference
   - **Has Gameplay Tag** - to check if container has tag
   - **Add Gameplay Tag** - to add tag to container
   - **Remove Gameplay Tag** - to remove tag

## Step 6: Troubleshooting

### Common Issues:

**1. Tags not showing up:**
- Make sure you've saved the Project Settings
- Try closing and reopening the editor
- Check that the INI file is saved correctly

**2. "None" appears instead of tags:**
- The tag might not be registered yet
- Go to Project Settings → Gameplay Tags
- Click "Refresh" or restart editor

**3. CSV import fails:**
- Make sure the CSV format is correct (Tag,DevComment headers)
- No extra spaces in tag names
- Use the direct INI method instead

**4. Tags work in C++ but not Blueprint:**
- Tags need to be registered before Blueprint compile
- Restart editor after adding new tags
- Use Project Settings method instead of CSV

## Step 7: Quick Verification

### Create a Test:
1. Create a new Blueprint Actor
2. Add a variable of type **Gameplay Tag Container**
3. Set default value - you should see all tags in dropdown
4. If tags appear, setup is successful!

## Recommended Approach

For this project, I recommend using the **Direct INI Configuration** (Step 3) because:
1. It's more reliable
2. Tags are version-controlled in the Config folder
3. No need to maintain separate Data Tables
4. Easier to add new tags (just edit the INI)

## Adding New Tags Later

To add new tags after initial setup:
1. Open `Config/DefaultGameplayTags.ini`
2. Add a new line like:
   ```ini
   +GameplayTagList=(Tag="Attack.Type.NewAttack",DevComment="Description here")
   ```
3. Save file
4. Restart editor
5. New tag is ready to use!

---

**Important:** After setting up tags, you need to restart the Unreal Editor for them to be fully registered and available in all systems.