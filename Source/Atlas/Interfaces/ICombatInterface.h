#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "ICombatInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Unified interface for all combat-related functionality.
 * Consolidates combat states, blocking, vulnerability, poise/stagger, and damage.
 * Allows components to communicate without direct dependencies.
 */
class ATLAS_API ICombatInterface
{
	GENERATED_BODY()

public:
	// ========== Combat State Queries ==========
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|State")
	bool IsInCombat() const;
	virtual bool IsInCombat_Implementation() const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|State")
	bool IsAttacking() const;
	virtual bool IsAttacking_Implementation() const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|State")
	bool IsVulnerable() const;
	virtual bool IsVulnerable_Implementation() const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|State")
	bool HasIFrames() const;
	virtual bool HasIFrames_Implementation() const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|State")
	bool IsStaggered() const;
	virtual bool IsStaggered_Implementation() const { return false; }
	
	// ========== Combat State Management ==========
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|State")
	void AddCombatStateTag(const FGameplayTag& Tag);
	virtual void AddCombatStateTag_Implementation(const FGameplayTag& Tag) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|State")
	void RemoveCombatStateTag(const FGameplayTag& Tag);
	virtual void RemoveCombatStateTag_Implementation(const FGameplayTag& Tag) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|State")
	bool HasCombatStateTag(const FGameplayTag& Tag) const;
	virtual bool HasCombatStateTag_Implementation(const FGameplayTag& Tag) const { return false; }
	
	// ========== Attack System ==========
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Attack")
	bool StartAttack(const FGameplayTag& AttackTag);
	virtual bool StartAttack_Implementation(const FGameplayTag& AttackTag) { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Attack")
	void EndAttack();
	virtual void EndAttack_Implementation() {}
	
	// ========== Block System ==========
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Block")
	bool StartBlock();
	virtual bool StartBlock_Implementation() { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Block")
	void EndBlock();
	virtual void EndBlock_Implementation() {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Block")
	bool IsBlocking() const;
	virtual bool IsBlocking_Implementation() const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Block")
	bool CanBlock() const;
	virtual bool CanBlock_Implementation() const { return true; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Block")
	float GetBlockDamageReduction() const;
	virtual float GetBlockDamageReduction_Implementation() const { return 0.4f; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Block")
	void OnBlockedAttack(const FGameplayTag& AttackTag, float DamageBlocked, AActor* Attacker);
	virtual void OnBlockedAttack_Implementation(const FGameplayTag& AttackTag, float DamageBlocked, AActor* Attacker) {}
	
	// ========== Parry System ==========
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Parry")
	void SetParryState(bool bParrying);
	virtual void SetParryState_Implementation(bool bParrying) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Parry")
	bool IsParrying() const;
	virtual bool IsParrying_Implementation() const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Parry")
	void OnParrySuccess(AActor* Attacker, bool bPerfectParry);
	virtual void OnParrySuccess_Implementation(AActor* Attacker, bool bPerfectParry) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Parry")
	bool IsInParryWindow(bool& bIsPerfectWindow, bool& bIsLateWindow) const;
	virtual bool IsInParryWindow_Implementation(bool& bIsPerfectWindow, bool& bIsLateWindow) const { return false; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Parry")
	float GetParryDamageReduction(bool bPerfectParry) const;
	virtual float GetParryDamageReduction_Implementation(bool bPerfectParry) const { return bPerfectParry ? 1.0f : 0.5f; }
	
	// ========== Vulnerability System ==========
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Vulnerability")
	void ApplyVulnerability(int32 Charges, float Duration);
	virtual void ApplyVulnerability_Implementation(int32 Charges, float Duration) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Vulnerability")
	int32 GetVulnerabilityCharges() const;
	virtual int32 GetVulnerabilityCharges_Implementation() const { return 0; }
	
	// ========== Poise & Stagger System ==========
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Poise")
	void ApplyPoiseDamage(float PoiseDamage, AActor* DamageInstigator);
	virtual void ApplyPoiseDamage_Implementation(float PoiseDamage, AActor* DamageInstigator) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Poise")
	float GetCurrentPoise() const;
	virtual float GetCurrentPoise_Implementation() const { return 100.0f; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Poise")
	float GetMaxPoise() const;
	virtual float GetMaxPoise_Implementation() const { return 100.0f; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Poise")
	void ResetPoise();
	virtual void ResetPoise_Implementation() {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Stagger")
	void ApplyStagger(float Duration, AActor* Instigator);
	virtual void ApplyStagger_Implementation(float Duration, AActor* Instigator) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Stagger")
	void RecoverFromStagger();
	virtual void RecoverFromStagger_Implementation() {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Stagger")
	float GetStaggerTimeRemaining() const;
	virtual float GetStaggerTimeRemaining_Implementation() const { return 0.0f; }
	
	// ========== Damage System ==========
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Damage")
	float TakeDamage(float DamageAmount, const FGameplayTagContainer& DamageTags, AActor* DamageInstigator);
	virtual float TakeDamage_Implementation(float DamageAmount, const FGameplayTagContainer& DamageTags, AActor* DamageInstigator) { return 0.0f; }
	
	// ========== Knockback System ==========
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Knockback")
	void ApplyKnockback(const FVector& KnockbackDirection, float KnockbackForce, bool bCauseRagdoll);
	virtual void ApplyKnockback_Implementation(const FVector& KnockbackDirection, float KnockbackForce, bool bCauseRagdoll) {}
	
	// ========== Block Stability (Optional Resource) ==========
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Block")
	float GetBlockStability() const;
	virtual float GetBlockStability_Implementation() const { return 100.0f; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Block")
	void ConsumeBlockStability(float Amount);
	virtual void ConsumeBlockStability_Implementation(float Amount) {}
};