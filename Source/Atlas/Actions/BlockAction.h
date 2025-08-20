#pragma once

#include "CoreMinimal.h"
#include "BaseAction.h"
#include "BlockAction.generated.h"

/**
 * Block action that provides damage reduction while held.
 * This is a held action that remains active as long as the input is held.
 */
UCLASS()
class ATLAS_API UBlockAction : public UBaseAction
{
	GENERATED_BODY()

public:
	UBlockAction();

	// Override BaseAction functions
	virtual bool CanActivate(AGameCharacterBase* Owner) override;
	virtual void OnActivate(AGameCharacterBase* Owner) override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnRelease() override;
	virtual void OnInterrupted() override;

protected:
	// Block execution
	void StartBlock();
	void EndBlock();
	
	// Helper functions
	bool CheckBlockRestrictions() const;
	
private:
	// Block state
	bool bIsBlocking;
	float BlockHoldTime;
};