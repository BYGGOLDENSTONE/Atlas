#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ActionManagerComponent.generated.h"

// Forward declarations
class UBaseAction;
class UActionDataAsset;
class AGameCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionSlotChanged, FName, SlotName, UBaseAction*, NewAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionActivated, FName, SlotName, UBaseAction*, Action);

/**
 * Component that manages the unified action system.
 * Handles 5 universal slots that can hold any action.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UActionManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActionManagerComponent();

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
	UBaseAction* GetActionInSlot(FName SlotName) const;

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
	UBaseAction* GetCurrentAction() const { return CurrentAction; }

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

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Action Manager|Events")
	FOnActionSlotChanged OnActionSlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Action Manager|Events")
	FOnActionActivated OnActionActivated;

protected:
	// Helper functions
	UBaseAction* CreateActionInstance(UActionDataAsset* ActionData);
	void InitializeSlots();
	void TickActions(float DeltaTime);

protected:
	// The 5 universal action slots
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Manager")
	TMap<FName, UBaseAction*> ActionSlots;

	// Currently active action
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action Manager")
	UBaseAction* CurrentAction;

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
};