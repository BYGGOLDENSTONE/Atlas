# Atlas Engine Setup Guide

## Overview
This guide covers the setup required in Unreal Engine after the code quality improvements. Follow each section in order.

---

## 1. Create Data Assets

### A. Combat Rules Data Asset
1. **Create the Asset:**
   - In Content Browser, right-click → Miscellaneous → Data Asset
   - Choose `CombatRulesDataAsset` as the class
   - Name it: `DA_CombatRules`
   - Save in: `Content/Data/Combat/`

2. **Configure Values:**
   ```
   Defense:
   - Block Damage Reduction: 0.4
   
   Vulnerability:
   - Vulnerability Multiplier: 8.0
   - Vulnerability Duration: 1.0
   - Default Vulnerability Charges: 1
   
   Poise:
   - Max Poise: 100.0
   - Poise Regen Rate: 15.0
   - Poise Regen Delay: 1.5
   
   Stagger:
   - Stagger Duration: 2.0
   - Wall Impact Stagger Duration: 2.0
   - Floor Impact Ragdoll Duration: 1.0
   
   Focus:
   - Focus Range: 2000.0
   - Focus Target Priority Weight: 0.3
   
   Dash:
   - Dash Distance: 400.0
   - Dash Cooldown: 2.0
   - Dash Speed: 2000.0
   - Dash IFrame Duration: 0.3
   
   Combat:
   - Combat Timeout Duration: 3.0
   
   Knockback:
   - Min Knockback For Wall Impact: 300.0
   - Wall Impact Check Radius: 50.0
   
   Interactables:
   - Interactable Cooldown: 10.0
   - Vent Projectile Speed: 2000.0
   - Vent Poise Damage: 50.0
   - Valve AoE Radius: 500.0
   ```

### B. Debug Commands Data Asset
1. **Create the Asset:**
   - Right-click → Miscellaneous → Data Asset
   - Choose `DebugCommandsDataAsset`
   - Name it: `DA_DebugCommands`
   - Save in: `Content/Data/Debug/`

2. **Add Debug Commands:**
   Click '+' to add commands with these settings:
   ```
   Command 1:
   - Command Name: "ToggleCombatDebug"
   - Description: "Toggle combat debug visualization"
   - Console Command: "Atlas.ToggleCombatDebug"
   - Command Type: Toggle
   - Category: "Combat"
   - Enabled In Shipping: False
   
   Command 2:
   - Command Name: "ShowFocusDebug"
   - Description: "Show focus mode debug info"
   - Console Command: "Atlas.ShowFocusDebug"
   - Command Type: Toggle
   - Category: "Interactions"
   
   Command 3:
   - Command Name: "SetDashCooldown"
   - Description: "Set dash cooldown time"
   - Console Command: "Atlas.SetDashCooldown"
   - Command Type: Value
   - Category: "Movement"
   - Default Value: "2.0"
   ```

---

## 2. Update Character Blueprints

### A. Player Character (BP_PlayerCharacter)
1. **Add New Components:**
   - Open `BP_PlayerCharacter` in Blueprint Editor
   - In Components panel, click "Add Component"
   - Add these components:
     - `DebugRenderComponent`
     - `AnimationManagerComponent`

2. **Configure DebugRenderComponent:**
   - Select the DebugRenderComponent
   - In Details panel:
     - Global Debug Enabled: True (for development)
     - Enabled Categories: Check all for now
     - Default Draw Duration: 0.0

3. **Configure AnimationManagerComponent:**
   - Select the AnimationManagerComponent
   - In Details panel, set up Montage Map:
     - Click '+' to add entries
     - Add gameplay tags and corresponding montages:
       ```
       Combat.Attack.Light → AM_LightAttack
       Combat.Attack.Heavy → AM_HeavyAttack
       Animation.HitReaction.Light → AM_HitLight
       Animation.HitReaction.Heavy → AM_HitHeavy
       Animation.Stagger → AM_Stagger
       Animation.Block.Success → AM_BlockSuccess
       Animation.Death → AM_Death
       ```

4. **Update CombatComponent:**
   - Select the CombatComponent
   - In Details panel:
     - Combat Rules: Set to `DA_CombatRules`
     - Attack Data Map: Ensure all attacks are mapped

5. **Wire Component References:**
   - In Event Graph, add initialization in BeginPlay:
   ```
   BeginPlay → Get CombatComponent → Set Combat Rules (DA_CombatRules)
   ```

### B. Enemy Character (BP_EnemyCharacter)
1. **Repeat the same steps as Player Character**
2. **Additional AI Setup:**
   - Ensure AI Controller is set
   - Components should be configured identically

---

## 3. Create Gameplay Tags

### In Project Settings:
1. **Open:** Edit → Project Settings → Gameplay Tags
2. **Add these tags if missing:**
   ```
   Combat
   ├── State
   │   ├── Attacking
   │   ├── Blocking
   │   ├── Vulnerable
   │   ├── Staggered
   │   └── IFrames
   ├── Attack
   │   ├── Light
   │   ├── Heavy
   │   ├── Soul
   │   └── Catch
   └── Status
       ├── Stunned
       ├── Crippled
       └── Exposed
   
   Animation
   ├── HitReaction
   │   ├── Light
   │   └── Heavy
   ├── Stagger
   ├── Block
   │   ├── Success
   │   └── Impact
   └── Death
   
   Ability
   ├── Dash
   └── Focus
   
   Interactable
   ├── Vent
   └── Valve
   ```

---

## 4. Update Existing Components

### A. Update ALL Combat Components
For each character Blueprint with CombatComponent:
1. **Open the Blueprint**
2. **Select CombatComponent**
3. **In Details panel:**
   - Set Combat Rules: `DA_CombatRules`
   - Compile and Save

### B. Update Health Components
1. **Check values match data asset:**
   - Max Poise: 100
   - Poise Regen Rate: 15
   - Poise Regen Delay: 1.5
   - Stagger Duration: 2.0

### C. Update Dash Components
1. **Check values match data asset:**
   - Dash Distance: 400
   - Dash Cooldown: 2.0
   - Dash Speed: 2000

### D. Update Focus Mode Components
1. **Check values match data asset:**
   - Focus Range: 2000
   - Debug Draw Focus Info: True (for development)

---

## 5. Interface Implementation

### For Characters that can be staggered:
1. **Open Character Blueprint**
2. **Class Settings → Interfaces → Add → IStaggerable**
3. **Implement interface functions:**
   - Right-click in Event Graph → Add Event → Interface Events
   - Implement each IStaggerable function:
     ```
     ApplyStagger: Call HealthComponent → Apply Stagger
     IsStaggered: Get HealthComponent → Return IsStaggered
     GetCurrentPoise: Get HealthComponent → Return CurrentPoise
     GetMaxPoise: Get HealthComponent → Return MaxPoise
     ```

### For Characters that can block:
1. **Add IBlockable interface**
2. **Implement functions:**
   ```
   StartBlock: Call CombatComponent → StartBlock
   EndBlock: Call CombatComponent → EndBlock
   IsBlocking: Get CombatComponent → Return IsBlocking
   GetBlockDamageReduction: Return 0.4 (from data asset)
   ```

---

## 6. Debug System Setup

### A. Enable Debug Visualization
1. **In Play Mode, press ~ (tilde) to open console**
2. **Test commands:**
   ```
   Atlas.ToggleCombatDebug 1
   Atlas.ShowFocusDebug 1
   ```

### B. Create Debug Menu Widget (Optional)
1. **Create Widget Blueprint:** `WBP_DebugMenu`
2. **Add checkboxes for:**
   - Combat Debug
   - Movement Debug
   - Interaction Debug
   - AI Debug
   - Physics Debug
3. **Bind to DebugRenderComponent categories**

---

## 7. Animation Setup

### A. Create Animation Montages if missing:
1. **Required Montages:**
   - AM_LightAttack
   - AM_HeavyAttack
   - AM_HitLight
   - AM_HitHeavy
   - AM_Stagger
   - AM_BlockSuccess
   - AM_BlockImpact
   - AM_Death
   - AM_Dash

### B. Add Notifies to Attack Montages:
1. **Open each attack montage**
2. **Add AnimNotify_AttackHitCheck at impact frames**
3. **Configure notify settings**

---

## 8. Testing Checklist

### A. Core Systems:
- [ ] Character spawns with all components
- [ ] Combat Rules data asset is loaded
- [ ] Debug commands work in console
- [ ] Animation Manager plays montages

### B. Combat:
- [ ] Light attack deals damage
- [ ] Heavy attack deals damage
- [ ] Block reduces damage to 40%
- [ ] Vulnerability multiplies damage by 8x
- [ ] Poise breaks at 0, causing stagger

### C. Debug Visualization:
- [ ] Combat hitboxes show when attacking
- [ ] Focus mode shows target selection
- [ ] Knockback arrows display
- [ ] Wall impacts show debug spheres

### D. Interfaces:
- [ ] Staggerable interface responds
- [ ] Blockable interface works
- [ ] Characters can query each other's states

---

## 9. Common Issues

### Issue: "CombatRules is null"
**Solution:** Assign DA_CombatRules to CombatComponent in Blueprint

### Issue: "No montage found for tag"
**Solution:** Add montage mappings in AnimationManagerComponent

### Issue: Debug visualization not showing
**Solution:** 
1. Check DebugRenderComponent exists
2. Verify Global Debug Enabled = true
3. Check category is enabled

### Issue: Interfaces not working
**Solution:** 
1. Add interface to Class Settings
2. Implement all interface events
3. Compile Blueprint

---

## 10. Performance Settings

### For Development:
- Global Debug Enabled: True
- All debug categories: Enabled
- Debug Draw Duration: 0.0 (persistent)

### For Testing/Release:
- Global Debug Enabled: False
- All debug categories: Disabled
- Remove DebugRenderComponent from shipping builds

---

## 11. Save Locations

Organize your assets:
```
Content/
├── Data/
│   ├── Combat/
│   │   ├── DA_CombatRules
│   │   └── DA_AttackData_[Name]
│   ├── Debug/
│   │   └── DA_DebugCommands
│   └── Interactables/
├── Characters/
│   ├── Player/
│   │   └── BP_PlayerCharacter
│   └── Enemies/
│       └── BP_EnemyCharacter
└── UI/
    └── Debug/
        └── WBP_DebugMenu
```

---

## Next Steps

After completing this setup:
1. **Test all systems in PIE (Play In Editor)**
2. **Verify combat damage calculations**
3. **Check debug visualization works**
4. **Ensure no null reference errors**
5. **Ready for P10: Advanced AI implementation**

Remember to **Compile and Save** after each change!