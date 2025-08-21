# Phase 2: Reward System UI, Room Progression & Persistence - Detailed Implementation Plan

## Overview
Phase 2 transforms Atlas from a combat prototype into a complete roguelite loop with room progression, reward selection, persistent meta-progression, and dynamic enemy scaling.

## Core Systems to Implement

### 1. Reward Selection UI System
### 2. Slot Management Interface  
### 3. Room Progression Manager
### 4. Save/Load Persistence
### 5. Enemy AI Scaling System
### 6. Initial Content Creation (15+ rewards per category)

---

## Task 1: Reward Selection UI System

### 1.1 Create Reward Selection Widget
**File**: Create `WBP_RewardSelection` Blueprint Widget
```
Layout:
- Title: "Choose Your Reward"
- 2 Reward Cards side-by-side
- Each card shows:
  - Icon
  - Name
  - Category badge
  - Description
  - Slot cost
  - Stack level indicator
- Selection button per card
- Skip button (if allowed)
```

### 1.2 Reward Card Component
**File**: Create `URewardCardWidget.h/.cpp`
```cpp
UCLASS()
class URewardCardWidget : public UUserWidget
{
    UPROPERTY(meta = (BindWidget))
    class UImage* RewardIcon;
    
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* RewardName;
    
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Description;
    
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SlotCost;
    
    UPROPERTY(meta = (BindWidget))
    class UButton* SelectButton;
    
    UFUNCTION(BlueprintCallable)
    void SetRewardData(URewardDataAsset* Reward);
    
    UFUNCTION()
    void OnSelectClicked();
};
```

### 1.3 Reward Selection Manager
**File**: Create `URewardSelectionComponent.h/.cpp`
```cpp
UCLASS()
class URewardSelectionComponent : public UActorComponent
{
    UFUNCTION(BlueprintCallable)
    void PresentRewardChoice(const TArray<URewardDataAsset*>& Rewards);
    
    UFUNCTION(BlueprintCallable)
    void OnRewardSelected(URewardDataAsset* SelectedReward);
    
    UPROPERTY(BlueprintAssignable)
    FOnRewardSelected OnRewardChosen;
    
protected:
    UPROPERTY()
    TSubclassOf<UUserWidget> RewardSelectionWidgetClass;
    
    UPROPERTY()
    UUserWidget* ActiveSelectionWidget;
};
```

### 1.4 UI Flow Integration
```cpp
// In GameMode after enemy defeat:
void AAtlasGameMode::OnEnemyDefeated()
{
    // Get current room's reward pool
    URoomDataAsset* CurrentRoom = RoomManager->GetCurrentRoom();
    TArray<URewardDataAsset*> RewardChoices = CurrentRoom->SelectRandomRewards(2, PlayerLevel);
    
    // Present selection UI
    RewardSelectionComponent->PresentRewardChoice(RewardChoices);
}
```

---

## Task 2: Slot Management Interface

### 2.1 Slot Management Widget
**File**: Create `WBP_SlotManager` Blueprint Widget
```
Layout:
- Grid of 6 slot frames
- Each slot shows:
  - Equipped reward icon (or empty)
  - Stack level indicator
  - Slot number
  - Hover tooltip with details
- Drag & drop support for rearranging
- Replace confirmation dialog
```

### 2.2 Slot Widget Component
**File**: Create `USlotWidget.h/.cpp`
```cpp
UCLASS()
class USlotWidget : public UUserWidget
{
    UPROPERTY(meta = (BindWidget))
    class UImage* SlotIcon;
    
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* SlotNumber;
    
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* StackLevel;
    
    UPROPERTY()
    int32 SlotIndex;
    
    UPROPERTY()
    FEquippedReward EquippedReward;
    
    UFUNCTION(BlueprintCallable)
    void UpdateSlotDisplay();
    
    // Drag & Drop
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
```

### 2.3 Inventory Panel Integration
**File**: Create `UInventoryPanelWidget.h/.cpp`
```cpp
UCLASS()
class UInventoryPanelWidget : public UUserWidget
{
    UPROPERTY(meta = (BindWidget))
    class UUniformGridPanel* SlotGrid;
    
    UPROPERTY()
    TArray<USlotWidget*> SlotWidgets;
    
    UFUNCTION(BlueprintCallable)
    void InitializeSlots();
    
    UFUNCTION(BlueprintCallable)
    void RefreshAllSlots();
    
    UFUNCTION()
    void OnSlotManagerChanged();
};
```

### 2.4 HUD Integration
```cpp
// Add to main HUD
void AAtlasHUD::BeginPlay()
{
    // Create persistent slot display
    if (SlotManagerWidgetClass)
    {
        SlotManagerWidget = CreateWidget<UInventoryPanelWidget>(GetWorld(), SlotManagerWidgetClass);
        SlotManagerWidget->AddToViewport(1);
        SlotManagerWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

// Toggle with Tab key
void AAtlasPlayerController::ToggleInventory()
{
    if (SlotManagerWidget)
    {
        ESlateVisibility NewVis = SlotManagerWidget->GetVisibility() == ESlateVisibility::Visible ? 
            ESlateVisibility::Collapsed : ESlateVisibility::Visible;
        SlotManagerWidget->SetVisibility(NewVis);
    }
}
```

---

## Task 3: Room Progression Manager

### 3.1 Run Manager Component
**File**: Create `URunManagerComponent.h/.cpp`
```cpp
UCLASS()
class URunManagerComponent : public UActorComponent
{
public:
    UPROPERTY(BlueprintReadOnly)
    int32 CurrentLevel = 1;
    
    UPROPERTY(BlueprintReadOnly)
    TArray<URoomDataAsset*> RemainingRooms;
    
    UPROPERTY(BlueprintReadOnly)
    URoomDataAsset* CurrentRoom;
    
    UPROPERTY(BlueprintReadOnly)
    TArray<URoomDataAsset*> CompletedRooms;
    
    UFUNCTION(BlueprintCallable)
    void InitializeRun();
    
    UFUNCTION(BlueprintCallable)
    URoomDataAsset* SelectNextRoom();
    
    UFUNCTION(BlueprintCallable)
    void CompleteCurrentRoom();
    
    UFUNCTION(BlueprintCallable)
    bool IsRunComplete() const { return CurrentLevel > 5; }
    
    UFUNCTION(BlueprintCallable)
    void TransitionToNextRoom();
    
protected:
    UPROPERTY(EditDefaultsOnly)
    TArray<URoomDataAsset*> AllRoomDataAssets;
    
    void LoadRoomLevel(URoomDataAsset* Room);
    void SpawnRoomEnemy(URoomDataAsset* Room);
    void ApplyRoomHazards(URoomDataAsset* Room);
};
```

### 3.2 Room Transition System
**File**: Create `ARoomTransitionActor.h/.cpp`
```cpp
UCLASS()
class ARoomTransitionActor : public AActor
{
    UPROPERTY(VisibleAnywhere)
    class UBoxComponent* TransitionTrigger;
    
    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<UWorld> NextRoomLevel;
    
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor);
    
    void BeginRoomTransition();
    void FadeOutScreen();
    void LoadNextRoom();
    void FadeInScreen();
};
```

### 3.3 Room Flow State Machine
```cpp
UENUM(BlueprintType)
enum class ERunState : uint8
{
    PreRun,          // Main menu
    RoomIntro,       // Show room name/enemy
    Combat,          // Active combat
    Victory,         // Enemy defeated
    RewardSelection, // Choosing reward
    RoomComplete,    // Ready to transition
    RunComplete,     // All 5 rooms done
    RunFailed        // Death or station destroyed
};

// In GameMode
void AAtlasGameMode::UpdateRunState(ERunState NewState)
{
    CurrentRunState = NewState;
    
    switch(NewState)
    {
        case ERunState::RoomIntro:
            ShowRoomIntroUI();
            StartCombatAfterDelay(3.0f);
            break;
            
        case ERunState::Victory:
            ShowVictoryUI();
            PresentRewardChoices();
            break;
            
        case ERunState::RunComplete:
            SaveRunProgress();
            ShowRunCompleteUI();
            break;
    }
}
```

### 3.4 Level Streaming Integration
```cpp
// Use level streaming for seamless transitions
void URunManagerComponent::LoadRoomLevel(URoomDataAsset* Room)
{
    if (Room->RoomLevel.IsValid())
    {
        FLatentActionInfo LatentInfo;
        UGameplayStatics::LoadStreamLevel(GetWorld(), Room->RoomLevel.GetAssetName(), true, true, LatentInfo);
    }
}
```

---

## Task 4: Save/Load Persistence System

### 4.1 Save Game Object
**File**: Create `UAtlasSaveGame.h/.cpp`
```cpp
UCLASS()
class UAtlasSaveGame : public USaveGame
{
    GENERATED_BODY()
    
public:
    // Persistent progression
    UPROPERTY()
    TArray<FEquippedReward> EquippedRewards;
    
    UPROPERTY()
    int32 TotalRunsCompleted = 0;
    
    UPROPERTY()
    int32 HighestRoomReached = 0;
    
    UPROPERTY()
    TMap<FGameplayTag, int32> UnlockedRewards;
    
    // Current run state (for resume)
    UPROPERTY()
    int32 CurrentRunLevel = 0;
    
    UPROPERTY()
    float CurrentHealth = 100.0f;
    
    UPROPERTY()
    float StationIntegrity = 100.0f;
    
    UPROPERTY()
    TArray<FName> CompletedRoomIDs;
    
    // Statistics
    UPROPERTY()
    int32 TotalEnemiesDefeated = 0;
    
    UPROPERTY()
    float TotalDamageDealt = 0.0f;
    
    UPROPERTY()
    int32 PerfectParries = 0;
};
```

### 4.2 Save Manager Subsystem
**File**: Create `USaveManagerSubsystem.h/.cpp`
```cpp
UCLASS()
class USaveManagerSubsystem : public UGameInstanceSubsystem
{
public:
    UFUNCTION(BlueprintCallable)
    void SaveGame();
    
    UFUNCTION(BlueprintCallable)
    void LoadGame();
    
    UFUNCTION(BlueprintCallable)
    void SaveRewards(USlotManagerComponent* SlotManager);
    
    UFUNCTION(BlueprintCallable)
    void LoadRewards(USlotManagerComponent* SlotManager);
    
    UFUNCTION(BlueprintCallable)
    void SaveRunProgress(const FRunProgressData& RunData);
    
    UFUNCTION(BlueprintCallable)
    bool HasSaveGame() const;
    
    UFUNCTION(BlueprintCallable)
    void DeleteSaveGame();
    
protected:
    UPROPERTY()
    UAtlasSaveGame* CurrentSaveGame;
    
    FString SaveSlotName = TEXT("AtlasSaveSlot");
    int32 UserIndex = 0;
};
```

### 4.3 Auto-Save Integration
```cpp
// Auto-save at key points
void AAtlasGameMode::OnRoomCompleted()
{
    if (USaveManagerSubsystem* SaveManager = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
    {
        SaveManager->SaveGame();
    }
}

void USlotManagerComponent::OnRewardEquipped()
{
    // Save immediately when rewards change
    if (USaveManagerSubsystem* SaveManager = GetGameInstance()->GetSubsystem<USaveManagerSubsystem>())
    {
        SaveManager->SaveRewards(this);
    }
}
```

### 4.4 Cloud Save Support (Steam)
```cpp
// Optional Steam Cloud integration
void USaveManagerSubsystem::EnableCloudSave()
{
    #if WITH_STEAMWORKS
    if (SteamRemoteStorage())
    {
        // Write save data to Steam Cloud
        TArray<uint8> SaveData;
        FMemoryWriter MemoryWriter(SaveData);
        
        FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
        CurrentSaveGame->Serialize(Ar);
        
        SteamRemoteStorage()->FileWrite("AtlasSave.sav", SaveData.GetData(), SaveData.Num());
    }
    #endif
}
```

---

## Task 5: Enemy AI Scaling System

### 5.1 AI Difficulty Calculator
**File**: Create `UAIDifficultyComponent.h/.cpp`
```cpp
UCLASS()
class UAIDifficultyComponent : public UActorComponent
{
public:
    UFUNCTION(BlueprintCallable)
    void CalculateDifficulty(int32 PlayerEquippedSlots);
    
    UFUNCTION(BlueprintPure)
    float GetDamageMultiplier() const { return DamageMultiplier; }
    
    UFUNCTION(BlueprintPure)
    float GetHealthMultiplier() const { return HealthMultiplier; }
    
    UFUNCTION(BlueprintPure)
    float GetReactionTimeModifier() const { return ReactionTimeModifier; }
    
    UFUNCTION(BlueprintPure)
    float GetAggressionLevel() const { return AggressionLevel; }
    
protected:
    // Calculated values based on player power
    float DamageMultiplier = 1.0f;
    float HealthMultiplier = 1.0f;
    float ReactionTimeModifier = 1.0f;
    float AggressionLevel = 0.5f;
    
    // Scaling formula: Enemy Power = Player Slots + 1
    void ApplyDifficultyScaling(int32 PowerLevel);
};
```

### 5.2 AI Behavior Tree Modifications
**File**: Modify existing AI behavior trees
```cpp
// Add difficulty-based decision nodes
UCLASS()
class UBTTask_SelectActionByDifficulty : public UBTTaskNode
{
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override
    {
        AGameCharacterBase* Enemy = Cast<AGameCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
        UAIDifficultyComponent* DiffComp = Enemy->FindComponentByClass<UAIDifficultyComponent>();
        
        float Aggression = DiffComp->GetAggressionLevel();
        
        if (FMath::FRandRange(0.0f, 1.0f) < Aggression)
        {
            // Choose aggressive action
            return ExecuteAggressiveAction(OwnerComp);
        }
        else
        {
            // Choose defensive action
            return ExecuteDefensiveAction(OwnerComp);
        }
    }
};
```

### 5.3 Dynamic AI Stats
```cpp
void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player's equipped slot count
    APlayerCharacter* Player = GetWorld()->GetFirstPlayerController()->GetPawn<APlayerCharacter>();
    USlotManagerComponent* SlotManager = Player->FindComponentByClass<USlotManagerComponent>();
    int32 UsedSlots = SlotManager->GetUsedSlotCount();
    
    // Apply scaling
    UAIDifficultyComponent* DiffComp = FindComponentByClass<UAIDifficultyComponent>();
    DiffComp->CalculateDifficulty(UsedSlots);
    
    // Apply to stats
    UHealthComponent* Health = FindComponentByClass<UHealthComponent>();
    Health->SetMaxHealth(Health->GetMaxHealth() * DiffComp->GetHealthMultiplier());
    
    // Adjust AI perception
    UAIPerceptionComponent* Perception = FindComponentByClass<UAIPerceptionComponent>();
    Perception->SetPeripheralVisionAngle(90.0f + (DiffComp->GetAggressionLevel() * 45.0f));
}
```

### 5.4 Adaptive Combat Patterns
```cpp
// Enemy learns from player patterns
USTRUCT()
struct FPlayerPatternData
{
    GENERATED_BODY()
    
    int32 BlockCount = 0;
    int32 ParryCount = 0;
    int32 DashCount = 0;
    int32 AttackCount = 0;
    
    float AverageReactionTime = 0.0f;
    FGameplayTag MostUsedAction;
};

class UAIAdaptiveComponent : public UActorComponent
{
    void AnalyzePlayerPattern();
    void AdaptStrategy(const FPlayerPatternData& Pattern);
    
    // Counter strategies
    void CounterDefensivePlayer();  // More guard breaks
    void CounterAggressivePlayer(); // More parries and counters
    void CounterEvasivePlayer();    // Area attacks, prediction
};
```

---

## Task 6: Initial Reward Content Creation

### 6.1 Defense Category Rewards (5 rewards)
```
1. DA_Reward_ImprovedBlock
   - Increases block damage reduction to 60%
   - Slot Cost: 1
   - Max Stack: 3 (60%, 70%, 80%)

2. DA_Reward_ParryMaster
   - Extends parry window by 0.1s
   - Slot Cost: 1
   - Max Stack: 2

3. DA_Reward_CounterStrike
   - Successful parries trigger automatic counter
   - Slot Cost: 2
   - Max Stack: 1

4. DA_Reward_IronSkin
   - Passive 10% damage reduction
   - Slot Cost: 1
   - Max Stack: 3 (10%, 15%, 20%)

5. DA_Reward_LastStand
   - Block becomes unbreakable below 25% health
   - Slot Cost: 2
   - Max Stack: 1
```

### 6.2 Offense Category Rewards (5 rewards)
```
1. DA_Reward_SharpBlade
   - +20% melee damage
   - Slot Cost: 1
   - Max Stack: 3 (20%, 35%, 50%)

2. DA_Reward_HeavyImpact
   - Heavy attacks cause AOE damage
   - Slot Cost: 2
   - Max Stack: 2

3. DA_Reward_BleedingStrikes
   - Attacks apply bleed DOT
   - Slot Cost: 1
   - Max Stack: 3

4. DA_Reward_ExecutionersAxe
   - +100% damage to stunned enemies
   - Slot Cost: 2
   - Max Stack: 1

5. DA_Reward_RapidStrikes
   - +30% attack speed
   - Slot Cost: 1
   - Max Stack: 3
```

### 6.3 Passive Stats Rewards (5 rewards)
```
1. DA_Reward_Vitality
   - +50 max health
   - Slot Cost: 1
   - Max Stack: 3 (+50, +75, +100)

2. DA_Reward_Swiftness
   - +20% movement speed
   - Slot Cost: 1
   - Max Stack: 2

3. DA_Reward_Heavyweight
   - +50% knockback force, +25% knockback resistance
   - Slot Cost: 1
   - Max Stack: 2

4. DA_Reward_Regeneration
   - 2 HP/sec when out of combat
   - Slot Cost: 2
   - Max Stack: 2

5. DA_Reward_Fortitude
   - +50% poise, faster poise recovery
   - Slot Cost: 1
   - Max Stack: 3
```

### 6.4 Passive Abilities (5 rewards)
```
1. DA_Reward_SecondWind
   - Revive once per run with 50% health
   - Slot Cost: 3
   - Max Stack: 1

2. DA_Reward_Vampirism
   - Heal 10% of damage dealt
   - Slot Cost: 2
   - Max Stack: 2 (10%, 15%)

3. DA_Reward_Berserker
   - +50% damage when below 30% health
   - Slot Cost: 2
   - Max Stack: 1

4. DA_Reward_MomentumBuilder
   - Each hit increases damage by 5% (max 50%)
   - Slot Cost: 2
   - Max Stack: 1

5. DA_Reward_StationShield
   - 50% of player damage taken goes to station instead
   - Slot Cost: 2
   - Max Stack: 1
```

### 6.5 Interactable Rewards (5 rewards)
```
1. DA_Reward_ExplosiveValves
   - Hack valves to explode for 100 damage
   - Slot Cost: 1
   - Max Stack: 2 (damage increase)

2. DA_Reward_GravityWells
   - Create gravity anomalies that pull enemies
   - Slot Cost: 2
   - Max Stack: 1

3. DA_Reward_TurretHack
   - Convert station turrets to attack enemies
   - Slot Cost: 2
   - Max Stack: 1

4. DA_Reward_EmergencyVent
   - Trigger vents to launch enemies
   - Slot Cost: 1
   - Max Stack: 2

5. DA_Reward_PowerSurge
   - Overload panels to stun nearby enemies
   - Slot Cost: 1
   - Max Stack: 3
```

---

## Implementation Timeline

### Week 1: Core UI Systems
- Day 1-2: Reward selection UI and flow
- Day 3-4: Slot management interface with drag & drop
- Day 5: HUD integration and polish

### Week 2: Progression & Persistence
- Day 1-2: Room progression manager
- Day 3-4: Save/Load system
- Day 5: Testing save persistence

### Week 3: AI & Content
- Day 1-2: Enemy AI scaling system
- Day 3-4: Create all reward DataAssets
- Day 5: Balance testing and tuning

### Week 4: Polish & Integration
- Day 1-2: Room transition effects
- Day 3: Audio integration
- Day 4: Bug fixes
- Day 5: Final testing

---

## Testing Checklist

### UI Systems
- [ ] Reward selection appears after enemy defeat
- [ ] Can select between 2 reward options
- [ ] Slot management UI opens with Tab
- [ ] Drag & drop works between slots
- [ ] Stack indicators update correctly
- [ ] Tooltips show reward details

### Room Progression
- [ ] 5 rooms per run in correct order
- [ ] Each room has unique enemy
- [ ] Room hazards apply correctly
- [ ] Transitions are smooth
- [ ] Run completes after room 5
- [ ] Run fails on death or station destruction

### Save/Load
- [ ] Rewards persist between runs
- [ ] Can resume interrupted run
- [ ] Statistics track correctly
- [ ] Cloud save works (if implemented)
- [ ] Save file isn't corrupted

### Enemy Scaling
- [ ] Enemy power = player slots + 1
- [ ] AI behavior changes with difficulty
- [ ] Health/damage scale appropriately
- [ ] Enemies adapt to player patterns

### Rewards
- [ ] All 25 rewards function correctly
- [ ] Stacking works as designed
- [ ] Passive effects apply immediately
- [ ] Active abilities integrate with action system
- [ ] Interactables spawn and function

---

## Console Commands for Testing

```cpp
// Room progression
Atlas_SkipToRoom [RoomNumber]
Atlas_CompleteCurrentRoom
Atlas_ForceRoomReward [RewardTag]
Atlas_ResetRun

// Save/Load
Atlas_SaveGame
Atlas_LoadGame
Atlas_ClearSave
Atlas_ExportSaveData

// AI Difficulty
Atlas_SetEnemyPower [Level]
Atlas_ShowAIDifficulty
Atlas_ForceAIPattern [Aggressive/Defensive/Balanced]

// Reward testing
Atlas_GiveReward [RewardTag]
Atlas_EnhanceReward [RewardTag]
Atlas_ClearAllRewards
Atlas_MaxOutSlots
```

---

## Performance Considerations

1. **UI Updates**: Use event-driven updates, not tick
2. **Save System**: Async save/load for large data
3. **Room Streaming**: Preload next room during combat
4. **AI Calculations**: Cache difficulty values, don't recalculate per-frame
5. **Reward Effects**: Pool VFX objects for reuse

---

## Risk Mitigation

1. **Save Corruption**: Implement save versioning and backup system
2. **UI Scaling**: Test on multiple resolutions
3. **Memory Leaks**: Properly destroy widgets when not needed
4. **Balance Issues**: Implement analytics to track win rates
5. **Soft Locks**: Add failsafe transitions and timeout systems

---

## Success Metrics

Phase 2 is complete when:
1. ✅ Players can complete a full 5-room run
2. ✅ Rewards persist between runs
3. ✅ UI is intuitive and responsive
4. ✅ Enemy difficulty scales appropriately
5. ✅ All 25 initial rewards are functional
6. ✅ Save/Load system is reliable
7. ✅ No soft locks or progression blockers