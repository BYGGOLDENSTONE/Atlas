#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "IActionInterface.generated.h"

// Forward declarations
class UActionInstance;
class UActionDataAsset;

UINTERFACE(MinimalAPI, Blueprintable)
class UActionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for action system functionality.
 * Allows actors and components to interact with the action system.
 */
class ATLAS_API IActionInterface
{
	GENERATED_BODY()

public:
	// Action execution
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	bool CanPerformAction(const FGameplayTag& ActionTag) const;
	virtual bool CanPerformAction_Implementation(const FGameplayTag& ActionTag) const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	bool TryPerformAction(const FGameplayTag& ActionTag);
	virtual bool TryPerformAction_Implementation(const FGameplayTag& ActionTag) { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	void InterruptCurrentAction();
	virtual void InterruptCurrentAction_Implementation() {}
	
	// Action queries
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	bool IsPerformingAction() const;
	virtual bool IsPerformingAction_Implementation() const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	UActionInstance* GetCurrentAction() const;
	virtual UActionInstance* GetCurrentAction_Implementation() const { return nullptr; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	FGameplayTag GetCurrentActionTag() const;
	virtual FGameplayTag GetCurrentActionTag_Implementation() const { return FGameplayTag(); }
	
	// Slot management
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	bool AssignActionToSlot(FName SlotName, const FGameplayTag& ActionTag);
	virtual bool AssignActionToSlot_Implementation(FName SlotName, const FGameplayTag& ActionTag) { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	void ClearActionSlot(FName SlotName);
	virtual void ClearActionSlot_Implementation(FName SlotName) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	UActionInstance* GetActionInSlot(FName SlotName) const;
	virtual UActionInstance* GetActionInSlot_Implementation(FName SlotName) const { return nullptr; }
	
	// Cooldown management
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	float GetActionCooldownRemaining(const FGameplayTag& ActionTag) const;
	virtual float GetActionCooldownRemaining_Implementation(const FGameplayTag& ActionTag) const { return 0.0f; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	bool IsActionOnCooldown(const FGameplayTag& ActionTag) const;
	virtual bool IsActionOnCooldown_Implementation(const FGameplayTag& ActionTag) const { return false; }
	
	// Action state tags
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	void AddActionStateTag(const FGameplayTag& Tag);
	virtual void AddActionStateTag_Implementation(const FGameplayTag& Tag) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	void RemoveActionStateTag(const FGameplayTag& Tag);
	virtual void RemoveActionStateTag_Implementation(const FGameplayTag& Tag) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	bool HasActionStateTag(const FGameplayTag& Tag) const;
	virtual bool HasActionStateTag_Implementation(const FGameplayTag& Tag) const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Actions")
	FGameplayTagContainer GetActionStateTags() const;
	virtual FGameplayTagContainer GetActionStateTags_Implementation() const { return FGameplayTagContainer(); }
};