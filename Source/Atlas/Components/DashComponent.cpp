#include "DashComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CombatComponent.h"
#include "HealthComponent.h"
#include "../Core/AtlasGameplayTags.h"

UDashComponent::UDashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentDashState = EDashState::Ready;
	DashTimer = 0.0f;
	CooldownTimer = 0.0f;
	InvincibilityTimer = 0.0f;
	bIsInvincible = false;
}

void UDashComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		CombatComponent = OwnerCharacter->FindComponentByClass<UCombatComponent>();
		HealthComponent = OwnerCharacter->FindComponentByClass<UHealthComponent>();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DashComponent: Failed to cast GetOwner() to ACharacter!"));
	}
}

void UDashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	static float LogTimer = 0.0f;
	LogTimer += DeltaTime;
	
	if (CurrentDashState == EDashState::Dashing)
	{
		UpdateDash(DeltaTime);
	}
	else if (CurrentDashState == EDashState::Cooldown)
	{
		CooldownTimer -= DeltaTime;
		
		// Log cooldown status every 0.5 seconds
		if (LogTimer >= 0.5f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Dash Cooldown: %.2f seconds remaining"), CooldownTimer);
			LogTimer = 0.0f;
		}
		
		if (CooldownTimer <= 0.0f)
		{
			CurrentDashState = EDashState::Ready;
			CooldownTimer = 0.0f;
		}
	}

	if (bIsInvincible)
	{
		InvincibilityTimer -= DeltaTime;
		if (InvincibilityTimer <= 0.0f)
		{
			SetInvincibility(false);
		}
	}
}

void UDashComponent::TryDash(const FVector2D& InputDirection)
{
	
	if (!CanDash())
	{
		return;
	}

	FVector DashDir = CalculateDashDirection(InputDirection);
	StartDash(DashDir);
}

bool UDashComponent::CanDash() const
{
	if (CurrentDashState != EDashState::Ready)
	{
		return false;
	}

	if (!CheckStateRestrictions())
	{
		return false;
	}
	return true;
}

void UDashComponent::StartDash(const FVector& Direction)
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("StartDash: No OwnerCharacter!"));
		return;
	}

	
	CurrentDashState = EDashState::Dashing;
	DashDirection = Direction;
	DashStartLocation = OwnerCharacter->GetActorLocation();
	
	FDashSettings Settings = GetDashSettings();
	DashTargetLocation = DashStartLocation + (DashDirection * Settings.DashDistance);
	DashTimer = 0.0f;
	

	if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
	{
		MovementComp->SetMovementMode(MOVE_Flying);
		MovementComp->StopMovementImmediately();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get CharacterMovementComponent!"));
	}

	if (Settings.bEnableInvincibility)
	{
		SetInvincibility(true);
		InvincibilityTimer = Settings.InvincibilityDuration;
	}

	if (CombatComponent)
	{
		CombatComponent->EndAttack();
		CombatComponent->EndBlock();
	}

#if WITH_EDITOR
	DrawDebugLine(GetWorld(), DashStartLocation, DashTargetLocation, FColor::Yellow, false, 2.0f, 0, 2.0f);
#endif
}

void UDashComponent::UpdateDash(float DeltaTime)
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("UpdateDash: No OwnerCharacter!"));
		EndDash();
		return;
	}

	FDashSettings Settings = GetDashSettings();
	DashTimer += DeltaTime;
	
	float DashProgress = FMath::Clamp(DashTimer / Settings.DashDuration, 0.0f, 1.0f);
	float EasedProgress = FMath::InterpEaseInOut(0.0f, 1.0f, DashProgress, 2.0f);
	
	FVector NewLocation = FMath::Lerp(DashStartLocation, DashTargetLocation, EasedProgress);
	
	if (CheckForCollision(NewLocation))
	{
		UE_LOG(LogTemp, Warning, TEXT("Dash collision detected! Ending dash early"));
		EndDash();
		return;
	}
	
	OwnerCharacter->SetActorLocation(NewLocation, true);
	
	if (DashProgress >= 1.0f)
	{
		EndDash();
	}
}

void UDashComponent::EndDash()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("EndDash: No OwnerCharacter!"));
		return;
	}

	
	CurrentDashState = EDashState::Cooldown;
	FDashSettings Settings = GetDashSettings();
	CooldownTimer = Settings.DashCooldown;
	DashTimer = 0.0f;
	
	UE_LOG(LogTemp, Warning, TEXT("Dash ended. Cooldown timer set to: %.2f"), CooldownTimer);

	if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
	{
		MovementComp->SetMovementMode(MOVE_Walking);
		MovementComp->Velocity = FVector::ZeroVector;
		UE_LOG(LogTemp, Warning, TEXT("Movement mode restored to Walking"));
	}
}

bool UDashComponent::CheckStateRestrictions() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	// Check if staggered via HealthComponent
	if (UHealthComponent* HealthComp = OwnerCharacter->FindComponentByClass<UHealthComponent>())
	{
		if (HealthComp->IsStaggered())
		{
			return false;
		}
	}

	if (CombatComponent)
	{
		if (CombatComponent->IsAttacking())
		{
			return false;
		}
		
		if (CombatComponent->IsBlocking())
		{
			return false;
		}
		
	}

	if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
	{
		if (MovementComp->IsFalling())
		{
			return false;
		}
	}

	return true;
}

FVector UDashComponent::CalculateDashDirection(const FVector2D& InputDirection) const
{
	if (!OwnerCharacter)
	{
		return FVector::ForwardVector;
	}

	FVector2D DashInput = InputDirection;
	
	if (DashInput.IsNearlyZero())
	{
		FVector ForwardDir = OwnerCharacter->GetActorForwardVector();
		return ForwardDir;
	}
	
	// Determine cardinal direction based on input
	if (FMath::Abs(DashInput.X) > FMath::Abs(DashInput.Y))
	{
		DashInput.Y = 0.0f;
		DashInput.X = FMath::Sign(DashInput.X);
	}
	else
	{
		DashInput.X = 0.0f;
		DashInput.Y = FMath::Sign(DashInput.Y);
	}

	if (AController* Controller = OwnerCharacter->GetController())
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		FVector ResultDirection = (ForwardDirection * DashInput.Y) + (RightDirection * DashInput.X);
		return ResultDirection.GetSafeNormal();
	}

	UE_LOG(LogTemp, Warning, TEXT("No controller found, using actor forward"));
	return OwnerCharacter->GetActorForwardVector();
}

bool UDashComponent::CheckForCollision(const FVector& TestLocation) const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	
	UCapsuleComponent* CapsuleComp = OwnerCharacter->GetCapsuleComponent();
	if (!CapsuleComp)
	{
		return false;
	}

	FHitResult Hit;
	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		OwnerCharacter->GetActorLocation(),
		TestLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeCapsule(CapsuleComp->GetScaledCapsuleRadius(), CapsuleComp->GetScaledCapsuleHalfHeight()),
		QueryParams
	);

	return bHit;
}

void UDashComponent::SetInvincibility(bool bEnable)
{
	bIsInvincible = bEnable;
	
	if (HealthComponent)
	{
		HealthComponent->SetInvincible(bEnable);
	}
}

FDashSettings UDashComponent::GetDashSettings() const
{
	if (DashDataAsset)
	{
		return DashDataAsset->DashSettings;
	}
	return DefaultDashSettings;
}