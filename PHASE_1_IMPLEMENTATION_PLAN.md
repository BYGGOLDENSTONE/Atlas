# Phase 1: Core Combat Polish & Parry System - Detailed Implementation Plan

## Architecture Overview
All new abilities follow the existing pattern:
- **UniversalAction** handles execution via function map routing
- **ActionDataAssets** define all ability parameters
- **Animation notifies** control combat timing
- **ActionManagerComponent** manages slot assignment and activation

## Task 1: Re-implement Parry System

### 1.1 Data Asset Creation
**File**: Create `DA_ParryAction.uasset` in Content/Data/Actions/
```
Configuration:
- ActionTag: Action.Parry
- ActionType: Defense
- ActionClass: UniversalAction
- Cooldown: 0.5f
- DamageReduction: 1.0f (100% - perfect parry negates all damage)
- bCanBlockUnblockables: false
- bRequiresHoldInput: false
- ActionDuration: 0.3f (parry window)
- bCanBeInterrupted: false
- ActionMontage: AM_Parry
```

### 1.2 Parry Window System
**File**: Create `ParryNotifyState.h/.cpp` in Source/Atlas/Animation/
```cpp
class UParryNotifyState : public UAnimNotifyState
{
    // Parry window timing (0.2s perfect, 0.1s late)
    float PerfectParryWindow = 0.2f;
    float LateParryWindow = 0.1f;
    bool bIsInPerfectWindow = true;
    
    // On successful parry, apply vulnerability to attacker
    void OnParrySuccess(AActor* Attacker);
};
```

### 1.3 UniversalAction Integration
**File**: Modify `UniversalAction.cpp`
```cpp
// Add to InitializeExecutorMap():
ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Parry"), 
                      &UUniversalAction::ExecuteParry);

// New function:
void UUniversalAction::ExecuteParry()
{
    // Set parry state
    bIsParrying = true;
    
    // Subscribe to incoming damage events
    if (ICombatInterface* Combat = Cast<ICombatInterface>(CurrentOwner))
    {
        Combat->SetParryState(true);
    }
    
    // Play parry montage
    if (ActionData && ActionData->ActionMontage)
    {
        CurrentOwner->PlayAnimMontage(ActionData->ActionMontage);
    }
}
```

### 1.4 Combat Interface Updates
**File**: Modify `ICombatInterface.h`
```cpp
// Add parry-related functions:
virtual void SetParryState(bool bParrying) = 0;
virtual bool IsParrying() const = 0;
virtual void OnParrySuccess(AActor* Attacker) = 0;
```

### 1.5 Damage Processing Updates
**File**: Modify damage processing in `ActionManagerComponent.cpp`
```cpp
// In ProcessIncomingDamage():
if (IsParrying() && IsWithinParryWindow())
{
    // Perfect parry
    if (IsPerfectParryTiming())
    {
        // Apply vulnerability to attacker
        if (IVulnerabilityInterface* VulnInterface = Cast<IVulnerabilityInterface>(Attacker))
        {
            VulnInterface->ApplyVulnerability(EVulnerabilityTier::Stunned);
        }
        return 0.0f; // No damage taken
    }
    // Late parry
    else
    {
        return Damage * 0.5f; // 50% damage reduction
    }
}
```

### 1.6 Animation Setup
**Blueprint Work**:
1. Create `AM_Parry` montage
2. Add `ParryNotifyState` covering frames 5-15 (0.2s window at 30fps)
3. Add recovery animation frames 16-30

### 1.7 Visual Feedback
- Parry success: Blue flash VFX
- Perfect parry: Time dilation effect (0.5 for 0.2s)
- Failed parry: Red damage indicator

---

## Task 2: Soul Attack Implementation

### 2.1 Data Asset Creation
**File**: Create `DA_SoulAttack.uasset` in Content/Data/Actions/
```
Configuration:
- ActionTag: Action.SoulAttack
- ActionType: Special
- ActionClass: UniversalAction
- MeleeDamage: 50.0f
- bIsUnblockable: true
- IntegrityCost: 0.0f (no station damage)
- Cooldown: 8.0f
- KnockbackForce: 800.0f
- PoiseDamage: 100.0f (instant stagger)
- bCanBeInterrupted: false
- ActionDuration: 1.5f
- ActionMontage: AM_SoulAttack
```

### 2.2 Special Properties Implementation
**File**: Extend `ActionDataAsset.h`
```cpp
// Add to Special category:
UPROPERTY(EditAnywhere, Category = "9. Special")
bool bBypassesStationDamage = false;

UPROPERTY(EditAnywhere, Category = "9. Special")
bool bDrainsPlayerResource = false;

UPROPERTY(EditAnywhere, Category = "9. Special")
float ResourceCost = 0.0f;
```

### 2.3 UniversalAction Integration
**File**: Modify `UniversalAction.cpp`
```cpp
// Add to InitializeExecutorMap():
ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.SoulAttack"), 
                      &UUniversalAction::ExecuteSoulAttack);

// New function:
void UUniversalAction::ExecuteSoulAttack()
{
    // Check resource if needed
    if (ActionData && ActionData->bDrainsPlayerResource)
    {
        if (!HasSufficientResource(ActionData->ResourceCost))
            return;
    }
    
    // Play soul attack montage with VFX
    if (ActionData && ActionData->ActionMontage)
    {
        CurrentOwner->PlayAnimMontage(ActionData->ActionMontage);
        
        // Spawn soul VFX (ethereal/ghostly effect)
        SpawnSoulAttackVFX();
    }
    
    // Set unblockable flag for damage processing
    bIsExecutingSoulAttack = true;
}
```

### 2.4 Damage Processing Updates
**File**: Modify `ActionManagerComponent.cpp`
```cpp
// In ApplyDamageToTarget():
if (bIsSoulAttack)
{
    // Bypass all defenses
    FinalDamage = BaseDamage;
    
    // Skip station integrity damage
    bShouldDamageStation = false;
    
    // Apply full poise damage for guaranteed stagger
    if (IHealthInterface* Health = Cast<IHealthInterface>(Target))
    {
        Health->ReducePoise(100.0f);
    }
}
```

### 2.5 Animation & VFX Setup
**Blueprint Work**:
1. Create `AM_SoulAttack` montage (1.5s duration)
2. Windup: Frames 1-20 (gather soul energy)
3. Attack: Frames 21-35 (soul projection)
4. Recovery: Frames 36-45
5. Add `AttackNotifyState` at frames 21-35
6. Create ethereal/ghostly particle system

### 2.6 Unique Mechanics
- Soul Attack pierces through multiple enemies in a line
- Leaves a lingering "soul trail" that slows enemies
- Visual: Character glows with ethereal energy during windup

---

## Task 3: Combat Value Tuning

### 3.1 Verify Core Values
**Files to Check**: All DataAssets in Content/Data/Actions/

| Action | Current | Target | File |
|--------|---------|--------|------|
| Basic Attack | Check | 5 damage | DA_BasicAttack |
| Heavy Attack | Check | 15 damage, 500 knockback | DA_HeavyAttack |
| Soul Attack | New | 50 damage, unblockable | DA_SoulAttack |
| Block | Check | 40% reduction | DA_Block |
| Parry | New | 100% reduction (perfect) | DA_Parry |
| Dash | Check | 400 units, 2s cooldown | DA_Dash |

### 3.2 Component Value Verification
**File**: `VulnerabilityComponent.cpp`
```cpp
// Verify multipliers:
Stunned = 2.0f
Crippled = 4.0f  
Exposed = 8.0f
```

**File**: `HealthComponent.cpp`
```cpp
// Verify poise:
MaxPoise = 100.0f
StaggerDuration = 2.0f
PoiseRegenRate = 10.0f/s (after 3s delay)
```

### 3.3 Animation Timing Verification
- Attack windup: 0.2-0.3s
- Attack active: 0.1-0.2s
- Recovery: 0.3-0.4s
- Total attack time: ~0.7-0.9s

### 3.4 Console Commands for Testing
```cpp
// Add new debug commands:
Atlas_TestParry [Damage] - Simulates incoming attack for parry testing
Atlas_TestSoulAttack - Executes soul attack with debug output
Atlas_VerifyCombatValues - Prints all combat values for verification
Atlas_SetParryWindow [Duration] - Adjusts parry timing for testing
```

---

## Task 4: Begin Reward System Architecture

### 4.1 Base Reward Structure
**File**: Create `RewardDataAsset.h` in Source/Atlas/Data/
```cpp
UENUM(BlueprintType)
enum class ERewardCategory : uint8
{
    Defense,      // Block, Parry, Dodge
    Offense,      // Attacks
    PassiveStats, // Speed, Health, Knockback
    PassiveAbility, // Second Life, etc
    Interactable  // Hacking abilities
};

UCLASS()
class URewardDataAsset : public UPrimaryDataAsset
{
    UPROPERTY(EditAnywhere)
    ERewardCategory Category;
    
    UPROPERTY(EditAnywhere)
    FGameplayTag RewardTag;
    
    UPROPERTY(EditAnywhere)
    int32 SlotCost = 1;
    
    UPROPERTY(EditAnywhere)
    int32 MaxStackLevel = 3;
    
    UPROPERTY(EditAnywhere)
    TArray<float> StackMultipliers = {1.0f, 1.5f, 2.0f};
    
    UPROPERTY(EditAnywhere)
    UActionDataAsset* LinkedAction; // For active abilities
    
    UPROPERTY(EditAnywhere)
    TMap<FName, float> StatModifiers; // For passive stats
};
```

### 4.2 Slot Manager Component
**File**: Create `SlotManagerComponent.h/.cpp`
```cpp
UCLASS()
class USlotManagerComponent : public UActorComponent
{
    UPROPERTY()
    TArray<URewardDataAsset*> EquippedRewards;
    
    UPROPERTY()
    int32 MaxSlots = 6;
    
    UPROPERTY()
    TMap<FGameplayTag, int32> RewardStacks; // Track enhancement levels
    
public:
    bool EquipReward(URewardDataAsset* Reward, int32 SlotIndex);
    bool EnhanceReward(FGameplayTag RewardTag);
    void SaveRewardsForNextRun();
    void LoadRewardsFromSave();
};
```

### 4.3 Reward Pool Definition
**File**: Create `RoomDataAsset.h`
```cpp
UCLASS()
class URoomDataAsset : public UPrimaryDataAsset
{
    UPROPERTY(EditAnywhere)
    FName RoomName; // "Room A", "Room B", etc
    
    UPROPERTY(EditAnywhere)
    ERewardCategory RoomTheme;
    
    UPROPERTY(EditAnywhere)
    TArray<URewardDataAsset*> RewardPool;
    
    UPROPERTY(EditAnywhere)
    TSubclassOf<AGameCharacterBase> UniqueEnemy;
};
```

### 4.4 Initial Reward Definitions
Create these reward DataAssets:

**Defense Category**:
- DA_Reward_Block (existing)
- DA_Reward_Parry (new)
- DA_Reward_Dodge (new)

**Offense Category**:
- DA_Reward_BasicAttack (existing)
- DA_Reward_HeavyAttack (existing)
- DA_Reward_SoulAttack (new)

**Passive Stats**:
- DA_Reward_SpeedBoost (+20% move/attack speed)
- DA_Reward_HealthBoost (+50 max health)
- DA_Reward_KnockbackBoost (+50% knockback force)

**Passive Abilities**:
- DA_Reward_SecondLife (revive on death, consumes slot)
- DA_Reward_DoubleJump (requires 2 slots)

**Interactables**:
- DA_Reward_ExplodingValve (hack environment)
- DA_Reward_LaunchVent (environmental launcher)

---

## Implementation Order & Timeline

### Week 1: Parry System
- Day 1-2: Create ParryNotifyState and data assets
- Day 3-4: Integrate with UniversalAction and combat system
- Day 5: Animation setup and testing

### Week 2: Soul Attack
- Day 1-2: Create soul attack data and execution
- Day 3-4: Special damage processing and VFX
- Day 5: Polish and balancing

### Week 3: Combat Tuning & Rewards
- Day 1-2: Verify and tune all combat values
- Day 3-4: Create reward system base classes
- Day 5: Initial reward implementations

---

## Testing Checklist

### Parry System
- [ ] Perfect parry window (0.2s) negates all damage
- [ ] Perfect parry applies vulnerability to attacker
- [ ] Late parry (0.1s) reduces 50% damage
- [ ] Cannot parry unblockables
- [ ] Parry animation plays correctly
- [ ] VFX and audio feedback work

### Soul Attack
- [ ] Deals exactly 50 damage
- [ ] Bypasses all blocks and defenses
- [ ] Does NOT damage station integrity
- [ ] 8-second cooldown enforced
- [ ] Instant stagger on hit (100 poise damage)
- [ ] Unique VFX distinguishes from normal attacks

### Combat Values
- [ ] All attacks match GDD damage values
- [ ] Vulnerability multipliers (2x, 4x, 8x) verified
- [ ] Poise system (100 max, 2s stagger) working
- [ ] Dash i-frames (0.3s) functioning
- [ ] Block reduction (40%) accurate

### Reward System Foundation
- [ ] Reward data structure supports all 5 categories
- [ ] Slot management allows equip/replace
- [ ] Enhancement stacking works
- [ ] Save/load persistence functions

---

## Code Quality Requirements

1. **Follow Existing Patterns**
   - Use UniversalAction function map routing
   - All values in DataAssets
   - Animation-driven timing

2. **No Breaking Changes**
   - Maintain interface compatibility
   - Preserve existing functionality
   - Add, don't replace

3. **Documentation**
   - Comment new functions
   - Update CLAUDE.md with changes
   - Add debug commands

4. **Performance**
   - O(1) action lookups via function map
   - No per-frame allocations
   - Efficient collision checks

---

## Success Criteria

Phase 1 is complete when:
1. Parry system fully functional with counter-vulnerability
2. Soul attack implemented with all unique properties
3. All combat values match GDD specifications
4. Basic reward system architecture in place
5. All tests pass and game remains stable
6. Documentation updated with new systems