#pragma once

#include "CoreMinimal.h"
#include "BaseAction.h"
#include "DashAction.generated.h"

UENUM(BlueprintType)
enum class EDashActionState : uint8
{
	Ready,
	Dashing,
	Recovery
};

/**
 * Dash action that provides quick directional movement with invincibility frames.
 */
UCLASS()
class ATLAS_API UDashAction : public UBaseAction
{
	GENERATED_BODY()

public:
	UDashAction();

	// Override BaseAction functions
	virtual bool CanActivate(AGameCharacterBase* Owner) override;
	virtual void OnActivate(AGameCharacterBase* Owner) override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnRelease() override;
	virtual void OnInterrupted() override;

protected:
	// Dash execution
	void StartDash(const FVector& Direction);
	void UpdateDash(float DeltaTime);
	void EndDash();

	// Helper functions
	FVector CalculateDashDirection() const;
	bool CheckStateRestrictions() const;
	bool CheckForCollision(const FVector& TestLocation) const;
	void SetInvincibility(bool bEnable);

private:
	// Dash state
	EDashActionState DashState;
	FVector DashStartLocation;
	FVector DashTargetLocation;
	FVector DashDirection;
	float DashTimer;
	float InvincibilityTimer;
	bool bIsInvincible;

	// Cached movement input
	FVector2D LastMovementInput;
};