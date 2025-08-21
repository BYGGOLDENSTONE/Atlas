#include "KineticPulseAbility.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "../Components/HealthComponent.h"
// KineticPulseDataAsset removed - use ActionDataAsset
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UKineticPulseAbility::UKineticPulseAbility()
{
	AbilityName = "Kinetic Pulse";
	AbilityDescription = "Releases a short-range force pulse that pushes enemies and objects away";
	Cooldown = 3.0f;
	ExecutionDuration = 0.2f;
	RiskTier = EAbilityRiskTier::LowRisk;
	IntegrityCost = 0.0f;
}

void UKineticPulseAbility::BeginPlay()
{
	Super::BeginPlay();
}

void UKineticPulseAbility::ExecuteAbility()
{
	Super::ExecuteAbility();

	ACharacter* Owner = GetOwnerCharacter();
	if (!Owner)
	{
		return;
	}

	FKineticPulseConfig Config = GetPulseConfig();
	FVector PulseOrigin = Owner->GetActorLocation();

	// Find all actors in pulse radius
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Config.PulseRadius);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		PulseOrigin,
		FQuat::Identity,
		ECC_Pawn,
		SphereShape,
		QueryParams
	);

	// Also check for physics objects
	if (Config.bAffectPhysicsObjects)
	{
		GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			PulseOrigin,
			FQuat::Identity,
			ECC_PhysicsBody,
			SphereShape,
			QueryParams
		);
	}

	// Apply pulse to all found actors
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* Target = Result.GetActor();
		if (!Target || Target == Owner)
		{
			continue;
		}

		// Check if it's a character
		if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
		{
			ApplyPulseToActor(Target, PulseOrigin);
		}
		// Check if it's a physics object
		else if (Config.bAffectPhysicsObjects && Result.Component.IsValid())
		{
			if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Result.Component.Get()))
			{
				if (PrimComp->IsSimulatingPhysics())
				{
					ApplyPulseToPhysicsActor(Target, PulseOrigin);
				}
			}
		}
	}

	// Visual/audio feedback
	if (GEngine && GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer)
	{
		DrawDebugSphere(GetWorld(), PulseOrigin, Config.PulseRadius, 32, FColor::Cyan, false, 1.0f);
	}

	UE_LOG(LogTemp, Warning, TEXT("Kinetic Pulse executed at %s with radius %f"), *PulseOrigin.ToString(), Config.PulseRadius);
}

bool UKineticPulseAbility::CheckAbilitySpecificConditions() const
{
	// Add any Kinetic Pulse specific conditions here
	return true;
}

void UKineticPulseAbility::ApplyPulseToActor(AActor* Target, const FVector& PulseOrigin)
{
	if (!Target)
	{
		return;
	}

	FKineticPulseConfig Config = GetPulseConfig();

	// Calculate knockback direction
	FVector KnockbackDirection = CalculatePulseDirection(Target->GetActorLocation(), PulseOrigin);
	FVector KnockbackForce = KnockbackDirection * Config.PulseForce;

	// Apply damage directly to health component and knockback
	if (UHealthComponent* HealthComp = Target->FindComponentByClass<UHealthComponent>())
	{
		HealthComp->TakeDamage(Config.PulseDamage, GetOwner());
	}
	
	// Apply knockback
	if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
	{
		if (UCharacterMovementComponent* MovementComp = TargetCharacter->GetCharacterMovement())
		{
			MovementComp->AddImpulse(KnockbackForce, true);
		}
	}
}

void UKineticPulseAbility::ApplyPulseToPhysicsActor(AActor* Target, const FVector& PulseOrigin)
{
	if (!Target)
	{
		return;
	}

	FKineticPulseConfig Config = GetPulseConfig();

	UPrimitiveComponent* PrimComp = Target->FindComponentByClass<UPrimitiveComponent>();
	if (PrimComp && PrimComp->IsSimulatingPhysics())
	{
		FVector ImpulseDirection = CalculatePulseDirection(Target->GetActorLocation(), PulseOrigin);
		FVector Impulse = ImpulseDirection * Config.PulseForce * Config.PhysicsImpulseMultiplier;

		PrimComp->AddImpulseAtLocation(Impulse, Target->GetActorLocation());

		if (GEngine && GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer)
		{
			DrawDebugLine(GetWorld(), PulseOrigin, Target->GetActorLocation(), FColor::Yellow, false, 1.0f);
		}
	}
}

FVector UKineticPulseAbility::CalculatePulseDirection(const FVector& TargetLocation, const FVector& PulseOrigin) const
{
	FKineticPulseConfig Config = GetPulseConfig();

	FVector Direction = (TargetLocation - PulseOrigin).GetSafeNormal();
	
	// Add upward force
	Direction.Z += Config.UpwardForceMultiplier;
	Direction.Normalize();

	return Direction;
}

FKineticPulseConfig UKineticPulseAbility::GetPulseConfig() const
{
	if (PulseDataAsset)
	{
		return PulseDataAsset->PulseConfig;
	}
	return DefaultConfig;
}