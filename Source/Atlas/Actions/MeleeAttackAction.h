#pragma once

#include "CoreMinimal.h"
#include "BaseAction.h"
#include "GameplayTagContainer.h"
#include "MeleeAttackAction.generated.h"

/**
 * Melee attack action that can handle both basic and heavy attacks.
 * Attack type is determined by the data asset configuration.
 */
UCLASS()
class ATLAS_API UMeleeAttackAction : public UBaseAction
{
	GENERATED_BODY()

public:
	UMeleeAttackAction();

	// Override BaseAction functions
	virtual bool CanActivate(AGameCharacterBase* Owner) override;
	virtual void OnActivate(AGameCharacterBase* Owner) override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnRelease() override;
	virtual void OnInterrupted() override;

protected:
	// Attack execution
	void StartAttack();
	void EndAttack();
	
	// Helper functions
	FGameplayTag GetAttackTag() const;
	class UAnimMontage* GetAttackMontage() const;
	bool CheckAttackRestrictions() const;

	// Called when the attack animation ends
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	// Attack state
	bool bAttackInProgress;
	FGameplayTag CurrentAttackTag;
	
	// Cached montage reference
	UPROPERTY()
	class UAnimMontage* CurrentMontage;
};