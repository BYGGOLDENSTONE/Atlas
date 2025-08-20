#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IHealthInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UHealthInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for health-related functionality.
 * Allows components to query health status without direct dependencies.
 */
class ATLAS_API IHealthInterface
{
	GENERATED_BODY()

public:
	// Health queries
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const;
	virtual float GetCurrentHealth_Implementation() const { return 100.0f; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	float GetMaxHealth() const;
	virtual float GetMaxHealth_Implementation() const { return 100.0f; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	float GetHealthPercent() const;
	virtual float GetHealthPercent_Implementation() const { return 1.0f; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	bool IsAlive() const;
	virtual bool IsAlive_Implementation() const { return true; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	bool IsDead() const;
	virtual bool IsDead_Implementation() const { return false; }
	
	// Health modification
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	void ApplyDamage(float DamageAmount, AActor* DamageInstigator);
	virtual void ApplyDamage_Implementation(float DamageAmount, AActor* DamageInstigator) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	void ApplyHealing(float HealAmount, AActor* Healer);
	virtual void ApplyHealing_Implementation(float HealAmount, AActor* Healer) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	void SetHealth(float NewHealth);
	virtual void SetHealth_Implementation(float NewHealth) {}
	
	// Shield queries (if applicable)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	float GetCurrentShield() const;
	virtual float GetCurrentShield_Implementation() const { return 0.0f; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	float GetMaxShield() const;
	virtual float GetMaxShield_Implementation() const { return 0.0f; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	bool HasShield() const;
	virtual bool HasShield_Implementation() const { return false; }
	
	// Death handling
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	void Die(AActor* Killer);
	virtual void Die_Implementation(AActor* Killer) {}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Health")
	void Revive(float HealthAmount);
	virtual void Revive_Implementation(float HealthAmount) {}
};