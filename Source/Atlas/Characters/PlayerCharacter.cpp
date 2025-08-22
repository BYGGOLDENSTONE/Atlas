#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/LocalPlayer.h"
#include "../Components/HealthComponent.h"
#include "../Components/FocusModeComponent.h"
#include "../Components/ActionManagerComponent.h"
#include "GameplayTagContainer.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// All components are created in parent class GameCharacterBase
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();


	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(CombatMappingContext, 0);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get Enhanced Input Subsystem"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast Controller to PlayerController"));
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement and camera controls remain the same
		if (MoveAction)
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		if (LookAction)
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		
		// NEW: Bind universal action slots
		if (ActionManagerComponent)
		{
			if (Slot1Action)
			{
				EnhancedInputComponent->BindAction(Slot1Action, ETriggerEvent::Started, ActionManagerComponent, &UActionManagerComponent::OnSlot1Pressed);
				EnhancedInputComponent->BindAction(Slot1Action, ETriggerEvent::Completed, ActionManagerComponent, &UActionManagerComponent::OnSlot1Released);
			}
			if (Slot2Action)
			{
				EnhancedInputComponent->BindAction(Slot2Action, ETriggerEvent::Started, ActionManagerComponent, &UActionManagerComponent::OnSlot2Pressed);
				EnhancedInputComponent->BindAction(Slot2Action, ETriggerEvent::Completed, ActionManagerComponent, &UActionManagerComponent::OnSlot2Released);
			}
			if (Slot3Action)
			{
				EnhancedInputComponent->BindAction(Slot3Action, ETriggerEvent::Started, ActionManagerComponent, &UActionManagerComponent::OnSlot3Pressed);
				EnhancedInputComponent->BindAction(Slot3Action, ETriggerEvent::Completed, ActionManagerComponent, &UActionManagerComponent::OnSlot3Released);
			}
			if (Slot4Action)
			{
				EnhancedInputComponent->BindAction(Slot4Action, ETriggerEvent::Started, ActionManagerComponent, &UActionManagerComponent::OnSlot4Pressed);
				EnhancedInputComponent->BindAction(Slot4Action, ETriggerEvent::Completed, ActionManagerComponent, &UActionManagerComponent::OnSlot4Released);
			}
			if (Slot5Action)
			{
				EnhancedInputComponent->BindAction(Slot5Action, ETriggerEvent::Started, ActionManagerComponent, &UActionManagerComponent::OnSlot5Pressed);
				EnhancedInputComponent->BindAction(Slot5Action, ETriggerEvent::Completed, ActionManagerComponent, &UActionManagerComponent::OnSlot5Released);
			}
		}
		
		// Focus mode input (still needed for focus system)
		if (FocusQHoldAction)
		{
			EnhancedInputComponent->BindAction(FocusQHoldAction, ETriggerEvent::Started, this, &APlayerCharacter::FocusStart);
			EnhancedInputComponent->BindAction(FocusQHoldAction, ETriggerEvent::Completed, this, &APlayerCharacter::FocusStop);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast to EnhancedInputComponent"));
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// Block movement input if disabled
	if (!bMovementInputEnabled)
	{
		return;
	}
	
	FVector2D MovementVector = Value.Get<FVector2D>();
	LastMovementInput = MovementVector;

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::FocusStart()
{
	// Block focus mode if ability inputs are disabled
	if (!bAbilityInputsEnabled)
	{
		return;
	}
	
	if (FocusModeComponent)
	{
		FocusModeComponent->StartFocusMode();
	}
}

void APlayerCharacter::FocusStop()
{
	if (FocusModeComponent)
	{
		// Try to interact with focused target before stopping focus mode
		if (FocusModeComponent->IsFocusModeActive())
		{
			FocusModeComponent->TryInteractWithFocusedTarget();
		}
		FocusModeComponent->StopFocusMode();
	}
}

void APlayerCharacter::SetAbilityInputsEnabled(bool bEnabled)
{
	bAbilityInputsEnabled = bEnabled;
	
	if (!bEnabled)
	{
		// Force stop focus mode if disabling inputs
		if (FocusModeComponent && FocusModeComponent->IsFocusModeActive())
		{
			FocusModeComponent->StopFocusMode();
		}
	}
	
	// Ability inputs state changed
}

void APlayerCharacter::SetMovementInputEnabled(bool bEnabled)
{
	bMovementInputEnabled = bEnabled;
	
	if (!bEnabled)
	{
		// Clear any pending movement input
		LastMovementInput = FVector2D::ZeroVector;
	}
	
	// Movement input state changed
}

