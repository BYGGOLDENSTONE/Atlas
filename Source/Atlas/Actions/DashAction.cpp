#include "DashAction.h"
#include "../Characters/GameCharacterBase.h"
#include "../Components/CombatComponent.h"
#include "../Components/HealthComponent.h"
#include "../DataAssets/ActionDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "../Core/AtlasGameplayTags.h"

UDashAction::UDashAction()
{
	// ActionTag will be set from DataAsset
	DashState = EDashActionState::Ready;
	DashTimer = 0.0f;
	InvincibilityTimer = 0.0f;
	bIsInvincible = false;
}

bool UDashAction::CanActivate(AGameCharacterBase* Owner)
{
	if (!Super::CanActivate(Owner))
	{
		return false;
	}

	// Check if dash is ready
	if (DashState != EDashActionState::Ready)
	{
		return false;
	}

	// Check state restrictions
	if (!CheckStateRestrictions())
	{
		return false;
	}

	return true;
}

void UDashAction::OnActivate(AGameCharacterBase* Owner)
{
	Super::OnActivate(Owner);

	// Get movement input direction
	FVector DashDir = CalculateDashDirection();
	
	// If no input, dash forward
	if (DashDir.IsNearlyZero())
	{
		DashDir = Owner->GetActorForwardVector();
	}

	StartDash(DashDir);
}

void UDashAction::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);

	if (DashState == EDashActionState::Dashing)
	{
		UpdateDash(DeltaTime);
	}

	// Update invincibility
	if (bIsInvincible)
	{
		InvincibilityTimer -= DeltaTime;
		if (InvincibilityTimer <= 0.0f)
		{
			SetInvincibility(false);
		}
	}
}

void UDashAction::OnRelease()
{
	// Dash doesn't respond to release since it's instant activation
}

void UDashAction::OnInterrupted()
{
	if (DashState == EDashActionState::Dashing)
	{
		EndDash();
	}
	Super::OnInterrupted();
}

void UDashAction::StartDash(const FVector& Direction)
{
	if (!CurrentOwner)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(CurrentOwner);
	if (!Character)
	{
		return;
	}

	DashState = EDashActionState::Dashing;
	DashDirection = Direction;
	DashStartLocation = Character->GetActorLocation();
	
	// Get dash distance from data asset
	float DashDistance = 400.0f; // Default
	if (ActionData)
	{
		DashDistance = ActionData->DashDistance;
	}
	
	DashTargetLocation = DashStartLocation + (DashDirection * DashDistance);
	DashTimer = 0.0f;

	// Set movement mode to flying for dash
	if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
	{
		MovementComp->SetMovementMode(MOVE_Flying);
		MovementComp->StopMovementImmediately();
	}

	// Enable invincibility if configured
	if (ActionData && ActionData->bGrantsInvincibility)
	{
		SetInvincibility(true);
		InvincibilityTimer = ActionData->InvincibilityDuration;
	}

	// Interrupt any ongoing combat actions
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		CombatComp->EndAttack();
		CombatComp->EndBlock();
	}

#if WITH_EDITOR
	if (GetWorld())
	{
		DrawDebugLine(GetWorld(), DashStartLocation, DashTargetLocation, FColor::Yellow, false, 2.0f, 0, 2.0f);
	}
#endif
}

void UDashAction::UpdateDash(float DeltaTime)
{
	if (!CurrentOwner)
	{
		EndDash();
		return;
	}

	ACharacter* Character = Cast<ACharacter>(CurrentOwner);
	if (!Character)
	{
		EndDash();
		return;
	}

	// Get dash duration from data asset
	float DashDuration = 0.3f; // Default
	if (ActionData)
	{
		DashDuration = ActionData->DashDuration;
	}

	DashTimer += DeltaTime;
	float Alpha = FMath::Clamp(DashTimer / DashDuration, 0.0f, 1.0f);

	// Use a smooth curve for dash movement
	float CurveAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);
	FVector NewLocation = FMath::Lerp(DashStartLocation, DashTargetLocation, CurveAlpha);

	// Check for collision before moving
	if (!CheckForCollision(NewLocation))
	{
		Character->SetActorLocation(NewLocation, true);
	}
	else
	{
		// Hit something, end dash early
		EndDash();
		return;
	}

	// Check if dash is complete
	if (Alpha >= 1.0f)
	{
		EndDash();
	}
}

void UDashAction::EndDash()
{
	if (!CurrentOwner)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(CurrentOwner);
	if (!Character)
	{
		return;
	}

	DashState = EDashActionState::Ready;
	bIsActive = false;

	// Restore normal movement
	if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
	{
		MovementComp->SetMovementMode(MOVE_Walking);
	}

	// Start cooldown
	StartCooldown();
}

FVector UDashAction::CalculateDashDirection() const
{
	if (!CurrentOwner)
	{
		return FVector::ZeroVector;
	}

	// Get movement input from character
	// This would need to be passed in or cached from the character's movement input
	// For now, we'll use the character's forward vector
	ACharacter* Character = Cast<ACharacter>(CurrentOwner);
	if (Character && Character->GetCharacterMovement())
	{
		FVector Velocity = Character->GetCharacterMovement()->GetLastInputVector();
		if (!Velocity.IsNearlyZero())
		{
			return Velocity.GetSafeNormal();
		}
	}

	// Default to forward if no input
	return CurrentOwner->GetActorForwardVector();
}

bool UDashAction::CheckStateRestrictions() const
{
	if (!CurrentOwner)
	{
		return false;
	}

	// Check if character is stunned
	if (UHealthComponent* HealthComp = GetOwnerHealthComponent())
	{
		if (HealthComp->IsStaggered())
		{
			return false;
		}
	}

	// Check if character is attacking (can be configured)
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		// Allow dash to cancel attacks for now
		// This can be configured via data asset
	}

	return true;
}

bool UDashAction::CheckForCollision(const FVector& TestLocation) const
{
	if (!CurrentOwner)
	{
		return false;
	}

	ACharacter* Character = Cast<ACharacter>(CurrentOwner);
	if (!Character)
	{
		return false;
	}

	// Simple sphere trace for collision
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);

	float CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Character->GetActorLocation(),
		TestLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(CapsuleRadius),
		QueryParams
	);

	return bHit;
}

void UDashAction::SetInvincibility(bool bEnable)
{
	bIsInvincible = bEnable;

	if (CurrentOwner)
	{
		// Set invincibility on health component
		if (UHealthComponent* HealthComp = GetOwnerHealthComponent())
		{
			// We would need to add an invincibility system to HealthComponent
			// For now, we'll just track it locally
			UE_LOG(LogTemp, Log, TEXT("DashAction: Invincibility %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
		}
	}
}