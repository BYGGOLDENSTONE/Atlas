#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "ActionInstance.generated.h"

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
 * Simplified action instance that handles all action types through data configuration.
 * Single, efficient class for all action types without complex inheritance.
 */
UCLASS(BlueprintType)
class ATLAS_API UActionInstance : public UObject
{
	GENERATED_BODY()

public:
	UActionInstance();

	// Core execution functions
	UFUNCTION(BlueprintCallable, Category = "Action")
	bool CanExecute(AGameCharacterBase* Owner);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void Execute(AGameCharacterBase* Owner);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void Update(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void Interrupt();

	// State queries
	UFUNCTION(BlueprintPure, Category = "Action")
	bool IsActive() const { return CurrentState == EActionState::Active; }

	UFUNCTION(BlueprintPure, Category = "Action")
	bool IsOnCooldown() const { return CurrentState == EActionState::Cooldown; }

	UFUNCTION(BlueprintPure, Category = "Action")
	float GetCooldownRemaining() const { return CooldownTimer; }

	UFUNCTION(BlueprintPure, Category = "Action")
	FGameplayTag GetActionTag() const;

	UFUNCTION(BlueprintPure, Category = "Action")
	EActionState GetCurrentState() const { return CurrentState; }

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Action")
	void Initialize(UActionDataAsset* InActionData);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Action|Events")
	FOnActionStateChanged OnActionStateChanged;

protected:
	// Internal state management
	void SetActionState(EActionState NewState);
	void StartCooldown();
	
	// Action execution by type
	void ExecuteMovementAction(AGameCharacterBase* Owner);
	void ExecuteDefenseAction(AGameCharacterBase* Owner);
	void ExecuteAttackAction(AGameCharacterBase* Owner);
	void ExecuteUtilityAction(AGameCharacterBase* Owner);

	// Helper functions
	UFUNCTION(BlueprintPure, Category = "Action")
	class UActionManagerComponent* GetOwnerActionManagerComponent(AGameCharacterBase* Owner) const;

	UFUNCTION(BlueprintPure, Category = "Action")
	class UHealthComponent* GetOwnerHealthComponent(AGameCharacterBase* Owner) const;

	UFUNCTION(BlueprintPure, Category = "Action")
	class UStationIntegrityComponent* GetStationIntegrity(AGameCharacterBase* Owner) const;

	bool HasRequiredTags(AGameCharacterBase* Owner) const;
	bool IsBlockedByTags(AGameCharacterBase* Owner) const;

protected:
	// State
	UPROPERTY(BlueprintReadOnly, Category = "Action|State")
	EActionState CurrentState;

	UPROPERTY(BlueprintReadOnly, Category = "Action|State")
	float CooldownTimer;

	UPROPERTY(BlueprintReadOnly, Category = "Action|State")
	float ActionTimer;

	// Data reference
	UPROPERTY()
	UActionDataAsset* ActionData;

	// Current owner (set during execution)
	UPROPERTY()
	AGameCharacterBase* CurrentOwner;

	// Simple state flags for active actions
	bool bIsExecuting;
	bool bIsBlocking;
	bool bIsDashing;
	bool bIsAttacking;
};