#include "CoolantHazard.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Components/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ACoolantHazard::ACoolantHazard()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create collision component
	HazardCollision = CreateDefaultSubobject<USphereComponent>(TEXT("HazardCollision"));
	RootComponent = HazardCollision;
	HazardCollision->SetSphereRadius(200.0f);
	HazardCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HazardCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	HazardCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Create decal for visual representation
	HazardDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("HazardDecal"));
	HazardDecal->SetupAttachment(RootComponent);
	HazardDecal->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	HazardDecal->DecalSize = FVector(200.0f, 200.0f, 200.0f);

	// Create particle system
	HazardParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("HazardParticles"));
	HazardParticles->SetupAttachment(RootComponent);

	RemainingDuration = 10.0f;
	TickTimer = 0.0f;
}

void ACoolantHazard::BeginPlay()
{
	Super::BeginPlay();

	HazardCollision->OnComponentBeginOverlap.AddDynamic(this, &ACoolantHazard::OnBeginOverlap);
	HazardCollision->OnComponentEndOverlap.AddDynamic(this, &ACoolantHazard::OnEndOverlap);
}

void ACoolantHazard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update duration
	RemainingDuration -= DeltaTime;
	if (RemainingDuration <= 0.0f)
	{
		Destroy();
		return;
	}

	// Update tick timer for periodic effects
	TickTimer += DeltaTime;
	if (TickTimer >= SprayConfig.TickRate)
	{
		UpdateAffectedActors(TickTimer);
		TickTimer = 0.0f;
	}

	// Visual feedback
	if (GEngine && GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), SprayConfig.SprayRadius, 16, FColor::Cyan, false, 0.0f);
	}
}

void ACoolantHazard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up any affected actors
	for (auto& Pair : AffectedCharacters)
	{
		RemoveSlipEffect(Pair.Key);
	}
	AffectedCharacters.Empty();

	Super::EndPlay(EndPlayReason);
}

void ACoolantHazard::Initialize(const FCoolantSprayConfig& Config, AActor* InInstigator)
{
	SprayConfig = Config;
	Instigator = InInstigator;
	RemainingDuration = Config.HazardDuration;

	// Set collision radius
	if (HazardCollision)
	{
		HazardCollision->SetSphereRadius(Config.SprayRadius);
	}

	// Update decal size
	if (HazardDecal)
	{
		HazardDecal->DecalSize = FVector(Config.SprayRadius, Config.SprayRadius, Config.SprayRadius);
	}
}

void ACoolantHazard::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if should affect owner
	if (!SprayConfig.bAffectsOwner && OtherActor == Instigator)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (Character)
	{
		ApplySlipEffect(Character);
		AffectedCharacters.Add(Character, 0.0f);
	}
}

void ACoolantHazard::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (Character)
	{
		RemoveSlipEffect(Character);
		AffectedCharacters.Remove(Character);
	}
}

void ACoolantHazard::ApplySlipEffect(ACharacter* Character)
{
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp)
	{
		// Reduce friction significantly
		MovementComp->GroundFriction *= SprayConfig.FrictionMultiplier;
		MovementComp->BrakingDecelerationWalking *= SprayConfig.FrictionMultiplier;
		
		// Add random slip force
		FVector RandomSlipDirection = FVector(
			FMath::RandRange(-1.0f, 1.0f),
			FMath::RandRange(-1.0f, 1.0f),
			0.0f
		).GetSafeNormal();

		MovementComp->AddImpulse(RandomSlipDirection * SprayConfig.SlipForce, true);

		UE_LOG(LogTemp, Warning, TEXT("Applied slip effect to %s"), *Character->GetName());
	}
}

void ACoolantHazard::RemoveSlipEffect(ACharacter* Character)
{
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp)
	{
		// Restore friction
		MovementComp->GroundFriction /= SprayConfig.FrictionMultiplier;
		MovementComp->BrakingDecelerationWalking /= SprayConfig.FrictionMultiplier;

		UE_LOG(LogTemp, Warning, TEXT("Removed slip effect from %s"), *Character->GetName());
	}
}

void ACoolantHazard::UpdateAffectedActors(float DeltaTime)
{
	TArray<ACharacter*> CharactersToRemove;

	for (auto& Pair : AffectedCharacters)
	{
		ACharacter* Character = Pair.Key;
		if (!Character)
		{
			CharactersToRemove.Add(Character);
			continue;
		}

		// Apply periodic slip impulse
		UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
		if (MovementComp && MovementComp->IsMovingOnGround())
		{
			FVector Velocity = MovementComp->Velocity;
			if (Velocity.Size() > 100.0f)
			{
				// Add random slip when moving
				FVector SlipDirection = Velocity.GetSafeNormal();
				SlipDirection = FVector(
					SlipDirection.X + FMath::RandRange(-0.5f, 0.5f),
					SlipDirection.Y + FMath::RandRange(-0.5f, 0.5f),
					0.0f
				).GetSafeNormal();

				MovementComp->AddImpulse(SlipDirection * SprayConfig.SlipForce * 0.5f, true);

				// Apply poise damage occasionally
				Pair.Value += DeltaTime;
				if (Pair.Value >= 1.0f)
				{
					UHealthComponent* HealthComp = Character->FindComponentByClass<UHealthComponent>();
					if (HealthComp)
					{
						HealthComp->TakePoiseDamage(SprayConfig.SlipPoiseDamage);
					}
					Pair.Value = 0.0f;
				}
			}
		}
	}

	// Clean up null references
	for (ACharacter* Character : CharactersToRemove)
	{
		AffectedCharacters.Remove(Character);
	}
}