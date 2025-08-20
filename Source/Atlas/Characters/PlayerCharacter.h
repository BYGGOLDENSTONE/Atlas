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

	// Focus mode input (still needed for focus system)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* FocusQHoldAction;

	// NEW: Action Manager Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UActionManagerComponent* ActionManagerComponent;

	// Focus Mode Component (still needed for focus system)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UFocusModeComponent* FocusModeComponent;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void FocusStart();
	void FocusStop();
	
	FVector2D LastMovementInput;
};