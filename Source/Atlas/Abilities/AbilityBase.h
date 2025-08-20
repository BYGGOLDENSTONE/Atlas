#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AbilityBase.generated.h"

UENUM(BlueprintType)
enum class EAbilityState : uint8
{
	Ready,
	Executing,
	Cooldown
};

UENUM(BlueprintType)
enum class EAbilityRiskTier : uint8
{
	LowRisk,    // No integrity cost
	MediumRisk, // Minor integrity cost
	HighRisk    // Major integrity cost
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityStateChanged, EAbilityState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilityExecuted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityCooldownUpdate, float, RemainingCooldown);

UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UAbilityBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilityBase();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Main execution function
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool TryExecuteAbility();

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool CanExecuteAbility() const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	EAbilityState GetAbilityState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Ability")
	float GetCooldownRemaining() const { return CooldownTimer; }

	UFUNCTION(BlueprintPure, Category = "Ability")
	float GetCooldownPercent() const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	FString GetAbilityName() const { return AbilityName; }

	UFUNCTION(BlueprintPure, Category = "Ability")
	EAbilityRiskTier GetRiskTier() const { return RiskTier; }

protected:
	// Override these in derived classes
	virtual void ExecuteAbility();
	virtual void OnAbilityEnd();
	virtual bool CheckAbilitySpecificConditions() const { return true; }

	void StartCooldown();
	void UpdateCooldown(float DeltaTime);
	void SetAbilityState(EAbilityState NewState);

	// Utility functions for derived classes
	class ACharacter* GetOwnerCharacter() const { return OwnerCharacter; }
	class UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	class UHealthComponent* GetHealthComponent() const { return HealthComponent; }
	class UStationIntegrityComponent* GetStationIntegrity() const;

public:
	// Events
	UPROPERTY(BlueprintAssignable, Category = "Ability|Events")
	FOnAbilityStateChanged OnAbilityStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Ability|Events")
	FOnAbilityExecuted OnAbilityExecuted;

	UPROPERTY(BlueprintAssignable, Category = "Ability|Events")
	FOnAbilityCooldownUpdate OnAbilityCooldownUpdate;

protected:
	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Config")
	FString AbilityName = "Base Ability";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Config")
	FString AbilityDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Config")
	float Cooldown = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Config")
	float ExecutionDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Config")
	EAbilityRiskTier RiskTier = EAbilityRiskTier::LowRisk;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Config")
	float IntegrityCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Config")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Config")
	FGameplayTagContainer BlockedTags;

private:
	UPROPERTY()
	class ACharacter* OwnerCharacter;

	UPROPERTY()
	class UCombatComponent* CombatComponent;

	UPROPERTY()
	class UHealthComponent* HealthComponent;

	EAbilityState CurrentState;
	float CooldownTimer;
	float ExecutionTimer;
};