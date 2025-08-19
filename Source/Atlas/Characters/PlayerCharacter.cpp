#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/LocalPlayer.h"
#include "../Components/CombatComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/FocusModeComponent.h"
#include "../Components/DashComponent.h"
#include "GameplayTagContainer.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	FocusModeComponent = CreateDefaultSubobject<UFocusModeComponent>(TEXT("FocusModeComponent"));
	DashComponent = CreateDefaultSubobject<UDashComponent>(TEXT("DashComponent"));
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
		
		if (MoveAction)
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		if (LookAction)
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		if (AttackLMBAction)
			EnhancedInputComponent->BindAction(AttackLMBAction, ETriggerEvent::Triggered, this, &APlayerCharacter::AttackLMB);
		
		if (BlockRMBHoldAction)
		{
			EnhancedInputComponent->BindAction(BlockRMBHoldAction, ETriggerEvent::Started, this, &APlayerCharacter::BlockStart);
			EnhancedInputComponent->BindAction(BlockRMBHoldAction, ETriggerEvent::Completed, this, &APlayerCharacter::BlockStop);
		}
		
		if (DashSpaceAction)
		{
			EnhancedInputComponent->BindAction(DashSpaceAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Dash);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("DashSpaceAction is NULL - Dash will not work!"));
		}
		
		if (FocusQHoldAction)
		{
			EnhancedInputComponent->BindAction(FocusQHoldAction, ETriggerEvent::Started, this, &APlayerCharacter::FocusStart);
			EnhancedInputComponent->BindAction(FocusQHoldAction, ETriggerEvent::Completed, this, &APlayerCharacter::FocusStop);
		}
		
		if (HeavyEAction)
			EnhancedInputComponent->BindAction(HeavyEAction, ETriggerEvent::Triggered, this, &APlayerCharacter::HeavyAttack);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast to EnhancedInputComponent"));
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
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

void APlayerCharacter::AttackLMB()
{
	if (CombatComponent)
	{
		CombatComponent->StartAttack(FGameplayTag::RequestGameplayTag(FName("Attack.Type.Jab")));
	}
}


void APlayerCharacter::BlockStart()
{
	if (CombatComponent)
	{
		CombatComponent->StartBlock();
	}
}

void APlayerCharacter::BlockStop()
{
	if (CombatComponent)
	{
		CombatComponent->EndBlock();
	}
}

void APlayerCharacter::FocusStart()
{
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

void APlayerCharacter::HeavyAttack()
{
	if (CombatComponent)
	{
		CombatComponent->StartAttack(FGameplayTag::RequestGameplayTag(FName("Attack.Type.Heavy")));
	}
}

void APlayerCharacter::Dash()
{
	
	if (DashComponent)
	{
		DashComponent->TryDash(LastMovementInput);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerCharacter::Dash() - No DashComponent found!"));
	}
}