#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Atlas/Actions/ActionInstance.h"
#include "ActionManagerComponent.generated.h"

// Forward declarations
class UActionDataAsset;
class AGameCharacterBase;
class UHealthComponent;
class UVulnerabilityComponent;
class UCombatRulesDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionSlotChanged, FName, SlotName, UActionInstance*, NewAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionActivated, FName, SlotName, UActionInstance*, Action);

// Combat events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackStarted, const FGameplayTag&, AttackTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlockStarted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBlockEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVulnerabilityApplied);

/**
 * Unified component that manages both the action system and combat state.
 * Handles 5 universal action slots and all combat-related functionality.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UActionManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActionManagerComponent();
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Slot Management
	UFUNCTION(BlueprintCallable, Category = "Action Manager")
	bool AssignActionToSlot(FName SlotName, FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "Action Manager")
	bool AssignActionToSlotByDataAsset(FName SlotName, UActionDataAsset* ActionData);

	UFUNCTION(BlueprintCallable, Category = "Action Manager")
	void ClearSlot(FName SlotName);

	UFUNCTION(BlueprintCallable, Category = "Action Manager")
	void SwapSlots(FName Slot1, FName Slot2);

	UFUNCTION(BlueprintPure, Category = "Action Manager")
	UActionInstance* GetActionInSlot(FName SlotName) const;

	UFUNCTION(BlueprintPure, Category = "Action Manager")
	TArray<FName> GetAllSlotNames() const;

	// Input Handlers (called by PlayerCharacter)
	UFUNCTION(BlueprintCallable, Category = "Action Manager")
	void OnSlotPressed(FName SlotName);

	UFUNCTION(BlueprintCallable, Category = "Action Manager")
	void OnSlotReleased(FName SlotName);

	// Slot 1-5 specific handlers for Enhanced Input
	void OnSlot1Pressed() { OnSlotPressed(TEXT("Slot1")); }
	void OnSlot1Released() { OnSlotReleased(TEXT("Slot1")); }
	void OnSlot2Pressed() { OnSlotPressed(TEXT("Slot2")); }
	void OnSlot2Released() { OnSlotReleased(TEXT("Slot2")); }
	void OnSlot3Pressed() { OnSlotPressed(TEXT("Slot3")); }
	void OnSlot3Released() { OnSlotReleased(TEXT("Slot3")); }
	void OnSlot4Pressed() { OnSlotPressed(TEXT("Slot4")); }
	void OnSlot4Released() { OnSlotReleased(TEXT("Slot4")); }
	void OnSlot5Pressed() { OnSlotPressed(TEXT("Slot5")); }
	void OnSlot5Released() { OnSlotReleased(TEXT("Slot5")); }

	// Action Management
	UFUNCTION(BlueprintCallable, Category = "Action Manager")
	void InterruptCurrentAction();

	UFUNCTION(BlueprintPure, Category = "Action Manager")
	UActionInstance* GetCurrentAction() const { return CurrentAction; }

	UFUNCTION(BlueprintPure, Category = "Action Manager")
	bool IsActionActive() const { return CurrentAction != nullptr && CurrentAction->IsActive(); }

	// Available Actions
	UFUNCTION(BlueprintCallable, Category = "Action Manager")
	void LoadAvailableActions();

	UFUNCTION(BlueprintPure, Category = "Action Manager")
	TArray<UActionDataAsset*> GetAvailableActionDataAssets() const { return AvailableActionDataAssets; }

	UFUNCTION(BlueprintPure, Category = "Action Manager")
	UActionDataAsset* GetActionDataByTag(FGameplayTag ActionTag) const;

	// Console Commands
	UFUNCTION(Exec, Category = "Action Manager")
	void ExecuteAssignCommand(const FString& SlotName, const FString& ActionTagString);

	UFUNCTION(Exec, Category = "Action Manager")
	void ExecuteClearSlotCommand(const FString& SlotName);

	UFUNCTION(Exec, Category = "Action Manager")
	void ExecuteSwapSlotsCommand(const FString& Slot1, const FString& Slot2);

	UFUNCTION(Exec, Category = "Action Manager")
	void ExecuteListActionsCommand();

	UFUNCTION(Exec, Category = "Action Manager")
	void ExecuteShowSlotsCommand();

	// Action Events
	UPROPERTY(BlueprintAssignable, Category = "Action Manager|Events")
	FOnActionSlotChanged OnActionSlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Action Manager|Events")
	FOnActionActivated OnActionActivated;
	
	// Combat Events
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnAttackStarted OnAttackStarted;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnAttackEnded OnAttackEnded;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnBlockStarted OnBlockStarted;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnBlockEnded OnBlockEnded;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnVulnerabilityApplied OnVulnerabilityApplied;
	
	// Combo System
	UFUNCTION(BlueprintCallable, Category = "Action Manager|Combo")
	void SetComboWindowActive(bool bActive, FName WindowName);
	
	UFUNCTION(BlueprintCallable, Category = "Action Manager|Combo")
	void ExecuteBufferedAction();
	
	UFUNCTION(BlueprintPure, Category = "Action Manager|Combo")
	bool IsComboWindowActive() const { return bComboWindowActive; }

	// === COMBAT STATE MANAGEMENT ===
	
	// State queries
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInCombat() const;
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const;
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsBlocking() const;
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsVulnerable() const;
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool HasIFrames() const;
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetTimeSinceLastCombatAction() const;
	
	// State management
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AddCombatStateTag(const FGameplayTag& Tag);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RemoveCombatStateTag(const FGameplayTag& Tag);
	
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool HasCombatStateTag(const FGameplayTag& Tag) const;
	
	// Combat actions
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool StartBlock();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndBlock();
	
	// Parry System
	UFUNCTION(BlueprintCallable, Category = "Combat|Parry")
	void SetParryState(bool bParrying);
	
	UFUNCTION(BlueprintPure, Category = "Combat|Parry")
	bool IsParrying() const;
	
	UFUNCTION(BlueprintCallable, Category = "Combat|Parry")
	void OnParrySuccess(AActor* Attacker, bool bPerfectParry);
	
	UFUNCTION(BlueprintPure, Category = "Combat|Parry")
	bool IsInParryWindow(bool& bIsPerfectWindow, bool& bIsLateWindow) const;
	
	UFUNCTION(BlueprintPure, Category = "Combat|Parry")
	float GetParryDamageReduction(bool bPerfectParry) const;
	
	// Vulnerability
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyVulnerabilityWithIFrames(int32 Charges = 1, bool bGrantIFrames = false);
	
	// Hit processing (called by animation notifies)
	void ProcessHitFromAnimation(class AGameCharacterBase* HitCharacter);
	void SetCurrentActionData(class UActionDataAsset* ActionData);
	
	// Damage calculation helpers
	float CalculateFinalDamage(float BaseDamage, bool bIsBlocking, bool bIsVulnerable) const;
	void ApplyKnockback(class AGameCharacterBase* Target, const FVector& Direction, float Force, bool bCauseRagdoll);

protected:
	// Helper functions
	UActionInstance* CreateActionInstance(UActionDataAsset* ActionData);
	void InitializeSlots();
	void TickActions(float DeltaTime);

protected:
	// The 5 universal action slots
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Manager")
	TMap<FName, UActionInstance*> ActionSlots;

	// Currently active action
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Manager")
	UActionInstance* CurrentAction;

	// Owner reference
	UPROPERTY()
	AGameCharacterBase* OwnerCharacter;

	// Available action data assets (loaded from project)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Manager|Config")
	TArray<UActionDataAsset*> AvailableActionDataAssets;

	// Path to action data assets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Manager|Config")
	FString ActionDataAssetPath = TEXT("/Game/Atlas/DataAssets/Actions");

	// Default slot assignments (for testing)
	UPROPERTY(EditDefaultsOnly, Category = "Action Manager|Config")
	TMap<FName, FGameplayTag> DefaultSlotAssignments;
	
	// Combo system state
	bool bComboWindowActive = false;
	FName CurrentComboWindow = NAME_None;
	FName BufferedSlot = NAME_None;
	float BufferedInputTime = 0.0f;
	
	// === COMBAT STATE ===
	
	// Combat state tags
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	FGameplayTagContainer CombatStateTags;
	
	// Combat rules configuration
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatRulesDataAsset* CombatRules;
	
	// Currently active action data (for hit processing)
	UPROPERTY()
	class UActionDataAsset* CurrentActionData;
	
	// Combat timing
	float LastCombatActionTime = 0.0f;
	
	// Parry state tracking
	bool bIsParrying = false;
	float ParryStartTime = 0.0f;
	float PerfectParryWindow = 0.2f;  // Perfect parry window duration
	float LateParryWindow = 0.1f;     // Late parry window after perfect
	
	// Component references
	UPROPERTY()
	UHealthComponent* HealthComponent;
	
	UPROPERTY()
	UVulnerabilityComponent* VulnerabilityComponent;
};