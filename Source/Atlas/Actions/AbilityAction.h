#pragma once

#include "CoreMinimal.h"
#include "BaseAction.h"
#include "../Abilities/AbilityBase.h"
#include "AbilityAction.generated.h"

/**
 * Action wrapper for abilities to integrate them into the action system
 */
UCLASS(Blueprintable, BlueprintType)
class ATLAS_API UAbilityAction : public UBaseAction
{
	GENERATED_BODY()

public:
	UAbilityAction();

	// Override base action functions
	virtual bool CanActivate(AGameCharacterBase* Owner) override;
	virtual void OnActivate(AGameCharacterBase* Owner) override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnRelease() override;
	virtual void OnInterrupted() override;

	// Set the ability component class to use
	void SetAbilityClass(TSubclassOf<UAbilityBase> InAbilityClass);

protected:
	// The ability component class to instantiate
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<UAbilityBase> AbilityClass;

	// The instantiated ability component
	UPROPERTY()
	UAbilityBase* AbilityInstance;

private:
	void CreateAbilityInstance();
	void DestroyAbilityInstance();
};