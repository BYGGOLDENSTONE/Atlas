#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "BaseAction.generated.h"

// Forward declarations
class AGameCharacterBase;
class UActionDataAsset;

UENUM(BlueprintType)
enum class EActionState : uint8
{
	Idle,
	Active,
	Cooldown
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionStateChanged, EActionState, NewState);

/**
 * Base class for all player actions in the unified action system.
 * Actions are self-contained units of gameplay that can be assigned to any slot.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class ATLAS_API UBaseAction : public UObject
{
	GENERATED_BODY()

public:
	UBaseAction();

	// Core execution functions
	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual bool CanActivate(AGameCharacterBase* Owner);

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual void OnActivate(AGameCharacterBase* Owner);

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual void OnTick(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual void OnRelease();

	UFUNCTION(BlueprintCallable, Category = "Action")
	virtual void OnInterrupted();

	// State management
	UFUNCTION(BlueprintPure, Category = "Action")
	bool IsActive() const { return bIsActive; }

	UFUNCTION(BlueprintPure, Category = "Action")
	float GetCooldownRemaining() const { return CurrentCooldown; }

	UFUNCTION(BlueprintPure, Category = "Action")
	FGameplayTag GetActionTag() const { return ActionTag; }

	// Configuration
	void SetDataAsset(UActionDataAsset* InDataAsset);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Action|Events")
	FOnActionStateChanged OnActionStateChanged;

protected:
	// Update functions
	void UpdateCooldown(float DeltaTime);
	void StartCooldown();
	void SetActionState(EActionState NewState);

	// Helper functions for derived classes
	UFUNCTION(BlueprintPure, Category = "Action")
	class UCombatComponent* GetOwnerCombatComponent() const;

	UFUNCTION(BlueprintPure, Category = "Action")
	class UHealthComponent* GetOwnerHealthComponent() const;

	UFUNCTION(BlueprintPure, Category = "Action")
	class UStationIntegrityComponent* GetStationIntegrity() const;

	UFUNCTION(BlueprintPure, Category = "Action")
	bool HasRequiredTags(AGameCharacterBase* Owner) const;

	UFUNCTION(BlueprintPure, Category = "Action")
	bool IsBlockedByTags(AGameCharacterBase* Owner) const;

protected:
	// Identity
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	FGameplayTag ActionTag;

	// State
	UPROPERTY(BlueprintReadOnly, Category = "Action|State")
	bool bIsActive;

	UPROPERTY(BlueprintReadOnly, Category = "Action|State")
	float CurrentCooldown;

	UPROPERTY(BlueprintReadOnly, Category = "Action|State")
	EActionState CurrentState;

	// Current owner (set during activation)
	UPROPERTY()
	AGameCharacterBase* CurrentOwner;

	// Data Asset reference
	UPROPERTY()
	UActionDataAsset* ActionData;

	// Cached component references (set during activation)
	UPROPERTY()
	class UCombatComponent* CachedCombatComponent;

	UPROPERTY()
	class UHealthComponent* CachedHealthComponent;

	UPROPERTY()
	class UStationIntegrityComponent* CachedStationIntegrity;
};