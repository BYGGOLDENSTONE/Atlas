#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/LocalPlayer.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
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
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		EnhancedInputComponent->BindAction(AttackLMBAction, ETriggerEvent::Triggered, this, &APlayerCharacter::AttackLMB);
		EnhancedInputComponent->BindAction(ParryRMBAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ParryRMB);
		EnhancedInputComponent->BindAction(BlockSpaceHoldAction, ETriggerEvent::Started, this, &APlayerCharacter::BlockStart);
		EnhancedInputComponent->BindAction(BlockSpaceHoldAction, ETriggerEvent::Completed, this, &APlayerCharacter::BlockStop);
		EnhancedInputComponent->BindAction(FocusQHoldAction, ETriggerEvent::Started, this, &APlayerCharacter::FocusStart);
		EnhancedInputComponent->BindAction(FocusQHoldAction, ETriggerEvent::Completed, this, &APlayerCharacter::FocusStop);
		EnhancedInputComponent->BindAction(HeavyEAction, ETriggerEvent::Triggered, this, &APlayerCharacter::HeavyAttack);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

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

void APlayerCharacter::AttackLMB()
{
	UE_LOG(LogTemp, Log, TEXT("Attack LMB Pressed"));
}

void APlayerCharacter::ParryRMB()
{
	UE_LOG(LogTemp, Log, TEXT("Parry RMB Pressed"));
}

void APlayerCharacter::BlockStart()
{
	UE_LOG(LogTemp, Log, TEXT("Block Started"));
}

void APlayerCharacter::BlockStop()
{
	UE_LOG(LogTemp, Log, TEXT("Block Stopped"));
}

void APlayerCharacter::FocusStart()
{
	UE_LOG(LogTemp, Log, TEXT("Focus Mode Started"));
}

void APlayerCharacter::FocusStop()
{
	UE_LOG(LogTemp, Log, TEXT("Focus Mode Stopped"));
}

void APlayerCharacter::HeavyAttack()
{
	UE_LOG(LogTemp, Log, TEXT("Heavy Attack Pressed"));
}