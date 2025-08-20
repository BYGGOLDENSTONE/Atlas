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
	// Initialize action execution map
	virtual void BeginDestroy() override;
	
	// Action execution functions
	void ExecuteDash();
	void ExecuteBlock();
	void ExecuteMeleeAttack();
	void ExecuteGenericAbility();
	void ExecuteFocusMode();
	void ExecuteAreaEffect();
	void ExecuteRangedAttack();
	void ExecuteUtility();

	// Helper to get execution function based on action type
	void ExecuteActionByType();

	// State for different action types
	bool bIsBlocking;
	bool bIsDashing;
	bool bIsAttacking;
	bool bIsChanneling;
	
	float ActionTimer;  // Universal timer for any timed actions
	float ChannelProgress;  // For channeled abilities
	
	FVector DashDirection;
	
	// Function pointer type for action execution
	typedef void (UUniversalAction::*ActionExecutor)();
	
	// Map of action tags to execution functions
	TMap<FGameplayTag, ActionExecutor> ActionExecutorMap;
	
	// Initialize the executor map
	void InitializeExecutorMap();
};