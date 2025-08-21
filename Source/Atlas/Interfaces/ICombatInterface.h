#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "ICombatInterface.generated.h"

// Forward declarations

UINTERFACE(MinimalAPI, Blueprintable)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for combat-related functionality.
 * Allows components to communicate without direct dependencies.
 */
class ATLAS_API ICombatInterface
{
	GENERATED_BODY()

public:
	// State queries
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool IsInCombat() const;
	virtual bool IsInCombat_Implementation() const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool IsAttacking() const;
	virtual bool IsAttacking_Implementation() const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool IsBlocking() const;
	virtual bool IsBlocking_Implementation() const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool IsVulnerable() const;
	virtual bool IsVulnerable_Implementation() const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool HasIFrames() const;
	virtual bool HasIFrames_Implementation() const { return false; }
	
	// State management
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void AddCombatStateTag(const FGameplayTag& Tag);
	virtual void AddCombatStateTag_Implementation(const FGameplayTag& Tag) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void RemoveCombatStateTag(const FGameplayTag& Tag);
	virtual void RemoveCombatStateTag_Implementation(const FGameplayTag& Tag) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool HasCombatStateTag(const FGameplayTag& Tag) const;
	virtual bool HasCombatStateTag_Implementation(const FGameplayTag& Tag) const { return false; }
	
	// Combat actions
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool StartAttack(const FGameplayTag& AttackTag);
	virtual bool StartAttack_Implementation(const FGameplayTag& AttackTag) { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void EndAttack();
	virtual void EndAttack_Implementation() {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool StartBlock();
	virtual bool StartBlock_Implementation() { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void EndBlock();
	virtual void EndBlock_Implementation() {}
	
	// Vulnerability
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void ApplyVulnerability(int32 Charges, float Duration);
	virtual void ApplyVulnerability_Implementation(int32 Charges, float Duration) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	int32 GetVulnerabilityCharges() const;
	virtual int32 GetVulnerabilityCharges_Implementation() const { return 0; }
	
	// Damage application
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	float TakeDamage(float DamageAmount, const FGameplayTagContainer& DamageTags, AActor* DamageInstigator);
	virtual float TakeDamage_Implementation(float DamageAmount, const FGameplayTagContainer& DamageTags, AActor* DamageInstigator) { return 0.0f; }
	
	// Poise
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void ApplyPoiseDamage(float PoiseDamage);
	virtual void ApplyPoiseDamage_Implementation(float PoiseDamage) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	float GetCurrentPoise() const;
	virtual float GetCurrentPoise_Implementation() const { return 100.0f; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	float GetMaxPoise() const;
	virtual float GetMaxPoise_Implementation() const { return 100.0f; }
	
	// Knockback
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void ApplyKnockback(const FVector& KnockbackDirection, float KnockbackForce, bool bCauseRagdoll);
	virtual void ApplyKnockback_Implementation(const FVector& KnockbackDirection, float KnockbackForce, bool bCauseRagdoll) {}
};