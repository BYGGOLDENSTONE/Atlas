#pragma once

#include "CoreMinimal.h"
#include "BaseAction.h"
#include "UniversalAction.generated.h"

/**
 * Universal action that handles all action types through data configuration.
 * No need for separate classes - everything is data-driven!
 */
UCLASS(Blueprintable, BlueprintType)
class ATLAS_API UUniversalAction : public UBaseAction
{
	GENERATED_BODY()

public:
	UUniversalAction();

	// Override base action functions
	virtual bool CanActivate(AGameCharacterBase* Owner) override;
	virtual void OnActivate(AGameCharacterBase* Owner) override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnRelease() override;
	virtual void OnInterrupted() override;

protected:
	// Handle different action types based on tags
	void ExecuteDash();
	void ExecuteBlock();
	void ExecuteMeleeAttack();
	void ExecuteGenericAbility();

	// State for different action types
	bool bIsBlocking;
	bool bIsDashing;
	bool bIsAttacking;
	
	float DashTimer;
	float AttackTimer;
	
	FVector DashDirection;
};