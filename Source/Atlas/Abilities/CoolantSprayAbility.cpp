#include "CoolantSprayAbility.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "../Data/CoolantSprayDataAsset.h"
#include "../Actors/CoolantHazard.h"
#include "Kismet/GameplayStatics.h"

UCoolantSprayAbility::UCoolantSprayAbility()
{
	AbilityName = "Coolant Spray";
	AbilityDescription = "Sprays coolant on the ground creating a slippery hazard zone";
	Cooldown = 8.0f;
	ExecutionDuration = 0.5f;
	RiskTier = EAbilityRiskTier::LowRisk;
	IntegrityCost = 0.0f;
}

void UCoolantSprayAbility::ExecuteAbility()
{
	Super::ExecuteAbility();

	ACharacter* Owner = GetOwnerCharacter();
	if (!Owner)
	{
		return;
	}

	FVector SprayLocation = CalculateSprayLocation();
	SpawnHazardZone(SprayLocation);

	UE_LOG(LogTemp, Warning, TEXT("Coolant Spray: Created hazard at %s"), *SprayLocation.ToString());
}

bool UCoolantSprayAbility::CheckAbilitySpecificConditions() const
{
	return true;
}

FVector UCoolantSprayAbility::CalculateSprayLocation() const
{
	ACharacter* Owner = GetOwnerCharacter();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}

	FCoolantSprayConfig Config = GetSprayConfig();

	// Get spray direction from character's forward vector
	FVector ForwardVector = Owner->GetActorForwardVector();
	FVector StartLocation = Owner->GetActorLocation();

	// Trace to find ground location
	FVector TraceStart = StartLocation + (ForwardVector * Config.SprayRange);
	FVector TraceEnd = TraceStart - FVector(0, 0, 500.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	if (GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams))
	{
		return HitResult.Location;
	}

	// If no ground found, place at forward position
	return StartLocation + (ForwardVector * Config.SprayRange);
}

void UCoolantSprayAbility::SpawnHazardZone(const FVector& Location)
{
	if (!GetWorld())
	{
		return;
	}

	FCoolantSprayConfig Config = GetSprayConfig();

	// Spawn the hazard actor
	if (HazardActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		ACoolantHazard* Hazard = GetWorld()->SpawnActor<ACoolantHazard>(
			HazardActorClass,
			Location,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Hazard)
		{
			Hazard->Initialize(Config, GetOwner());
		}
	}
	else
	{
		// Fallback: Create a simple hazard without spawning actor
		// This would need a hazard manager system to track zones
		UE_LOG(LogTemp, Warning, TEXT("Coolant Spray: No HazardActorClass set, using debug visualization"));
		
		if (GEngine && GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer)
		{
			DrawDebugCylinder(
				GetWorld(),
				Location,
				Location + FVector(0, 0, 10),
				Config.SprayRadius,
				32,
				FColor::Cyan,
				false,
				Config.HazardDuration
			);
		}
	}
}

FCoolantSprayConfig UCoolantSprayAbility::GetSprayConfig() const
{
	if (SprayDataAsset)
	{
		return SprayDataAsset->SprayConfig;
	}
	return DefaultConfig;
}