#include "DebrisPullAbility.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
// DebrisPullDataAsset removed - use ActionDataAsset
#include "Kismet/GameplayStatics.h"

UDebrisPullAbility::UDebrisPullAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	AbilityName = "Debris Pull";
	AbilityDescription = "Magnetically pulls nearby debris and objects toward you";
	Cooldown = 5.0f;
	ExecutionDuration = 2.0f;
	RiskTier = EAbilityRiskTier::LowRisk;
	IntegrityCost = 0.0f;

	PullTimer = 0.0f;
	bIsPulling = false;
}

void UDebrisPullAbility::BeginPlay()
{
	Super::BeginPlay();
}

void UDebrisPullAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsPulling)
	{
		UpdatePull(DeltaTime);
	}
}

void UDebrisPullAbility::ExecuteAbility()
{
	Super::ExecuteAbility();

	FDebrisPullConfig Config = GetPullConfig();
	
	FindDebrisObjects();
	
	if (PulledObjects.Num() > 0)
	{
		bIsPulling = true;
		PullTimer = Config.PullDuration;
		ExecutionDuration = Config.PullDuration;
		
		UE_LOG(LogTemp, Warning, TEXT("Debris Pull: Started pulling %d objects"), PulledObjects.Num());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Debris Pull: No valid objects found to pull"));
		OnAbilityEnd();
	}
}

void UDebrisPullAbility::OnAbilityEnd()
{
	ReleaseObjects();
	bIsPulling = false;
	PullTimer = 0.0f;
	
	Super::OnAbilityEnd();
}

bool UDebrisPullAbility::CheckAbilitySpecificConditions() const
{
	return true;
}

void UDebrisPullAbility::FindDebrisObjects()
{
	PulledObjects.Empty();

	ACharacter* Owner = GetOwnerCharacter();
	if (!Owner)
	{
		return;
	}

	FDebrisPullConfig Config = GetPullConfig();
	FVector PlayerLocation = Owner->GetActorLocation();

	// Find all physics objects in radius
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Config.PullRadius);

	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		PlayerLocation,
		FQuat::Identity,
		ECC_PhysicsBody,
		SphereShape,
		QueryParams
	);

	// Also check WorldDynamic channel for interactables
	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		PlayerLocation,
		FQuat::Identity,
		ECC_WorldDynamic,
		SphereShape,
		QueryParams
	);

	int32 ObjectsAdded = 0;
	for (const FOverlapResult& Result : OverlapResults)
	{
		if (ObjectsAdded >= Config.MaxObjectsToPull)
		{
			break;
		}

		AActor* Actor = Result.GetActor();
		UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Result.Component.Get());

		if (IsValidDebrisObject(Actor, Component))
		{
			FPulledObject PulledObj;
			PulledObj.Actor = Actor;
			PulledObj.Component = Component;
			PulledObj.InitialDistance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
			
			PulledObjects.Add(PulledObj);
			ObjectsAdded++;

			// Visual feedback
			if (GEngine && GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer)
			{
				DrawDebugLine(GetWorld(), PlayerLocation, Actor->GetActorLocation(), FColor::Blue, false, Config.PullDuration);
			}
		}
	}
}

void UDebrisPullAbility::UpdatePull(float DeltaTime)
{
	FDebrisPullConfig Config = GetPullConfig();
	
	PullTimer -= DeltaTime;
	if (PullTimer <= 0.0f)
	{
		OnAbilityEnd();
		return;
	}

	for (const FPulledObject& PulledObj : PulledObjects)
	{
		if (PulledObj.Actor && PulledObj.Component)
		{
			ApplyPullForce(PulledObj, DeltaTime);
		}
	}
}

void UDebrisPullAbility::ApplyPullForce(const FPulledObject& PulledObj, float DeltaTime)
{
	ACharacter* Owner = GetOwnerCharacter();
	if (!Owner || !PulledObj.Actor || !PulledObj.Component)
	{
		return;
	}

	FDebrisPullConfig Config = GetPullConfig();
	
	FVector PlayerLocation = Owner->GetActorLocation();
	FVector ObjectLocation = PulledObj.Actor->GetActorLocation();
	float CurrentDistance = FVector::Dist(ObjectLocation, PlayerLocation);

	// Don't pull if too close
	if (CurrentDistance < Config.MinDistanceFromPlayer)
	{
		return;
	}

	// Calculate pull direction
	FVector PullDirection = (PlayerLocation - ObjectLocation).GetSafeNormal();
	
	// Add lift force if enabled
	if (Config.bLiftObjects)
	{
		PullDirection.Z += Config.LiftForce / Config.PullForce;
		PullDirection.Normalize();
	}

	// Apply force based on distance (stronger when farther)
	float DistanceMultiplier = FMath::Clamp(CurrentDistance / PulledObj.InitialDistance, 0.3f, 1.0f);
	FVector Force = PullDirection * Config.PullForce * DistanceMultiplier;

	// Apply the force
	PulledObj.Component->AddForce(Force);

	// Visual feedback
	if (GEngine && GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer)
	{
		DrawDebugLine(GetWorld(), PlayerLocation, ObjectLocation, FColor::Cyan, false, 0.0f);
		DrawDebugSphere(GetWorld(), ObjectLocation, 30.0f, 8, FColor::Blue, false, 0.0f);
	}
}

void UDebrisPullAbility::ReleaseObjects()
{
	// Could add release impulse here if desired
	for (const FPulledObject& PulledObj : PulledObjects)
	{
		if (PulledObj.Component && PulledObj.Component->IsSimulatingPhysics())
		{
			// Optional: Add a small release force
			FVector ReleaseForce = FVector(0, 0, 100.0f);
			PulledObj.Component->AddImpulse(ReleaseForce);
		}
	}

	PulledObjects.Empty();
}

bool UDebrisPullAbility::IsValidDebrisObject(AActor* Actor, UPrimitiveComponent* Component) const
{
	if (!Actor || !Component)
	{
		return false;
	}

	// Must be simulating physics
	if (!Component->IsSimulatingPhysics())
	{
		return false;
	}

	FDebrisPullConfig Config = GetPullConfig();

	// Check mass limit
	float Mass = Component->GetMass();
	if (Mass > Config.MaxObjectMass)
	{
		return false;
	}

	// Don't pull characters or pawns
	if (Actor->IsA<APawn>())
	{
		return false;
	}

	return true;
}

FDebrisPullConfig UDebrisPullAbility::GetPullConfig() const
{
	if (PullDataAsset)
	{
		return PullDataAsset->PullConfig;
	}
	return DefaultConfig;
}