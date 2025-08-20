#pragma once

#include "CoreMinimal.h"
#include "GameCharacterBase.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "PlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UFocusModeComponent;
class UDashComponent;
class UActionManagerComponent;

UCLASS()
class ATLAS_API APlayerCharacter : public AGameCharacterBase
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* CombatMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	// NEW: Universal Action Slots
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Slots", meta = (AllowPrivateAccess = "true"))
	UInputAction* Slot1Action; // Default: LMB

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Slots", meta = (AllowPrivateAccess = "true"))
	UInputAction* Slot2Action; // Default: RMB

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Slots", meta = (AllowPrivateAccess = "true"))
	UInputAction* Slot3Action; // Default: E

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Slots", meta = (AllowPrivateAccess = "true"))
	UInputAction* Slot4Action; // Default: R

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Action Slots", meta = (AllowPrivateAccess = "true"))
	UInputAction* Slot5Action; // Default: Space

	// DEPRECATED: Old direct action inputs (will be removed after migration)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Deprecated", meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackLMBAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Deprecated", meta = (AllowPrivateAccess = "true"))
	UInputAction* BlockRMBHoldAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Deprecated", meta = (AllowPrivateAccess = "true"))
	UInputAction* DashSpaceAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Deprecated", meta = (AllowPrivateAccess = "true"))
	UInputAction* FocusQHoldAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Deprecated", meta = (AllowPrivateAccess = "true"))
	UInputAction* HeavyEAction;

	// NEW: Action Manager Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UActionManagerComponent* ActionManagerComponent;

	// DEPRECATED: Will be removed after migration to action system
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UFocusModeComponent* FocusModeComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UDashComponent* DashComponent;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void AttackLMB();
	void BlockStart();
	void BlockStop();
	void FocusStart();
	void FocusStop();
	void HeavyAttack();
	void Dash();
	
	FVector2D LastMovementInput;
};