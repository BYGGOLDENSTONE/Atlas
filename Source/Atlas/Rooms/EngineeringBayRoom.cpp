#include "EngineeringBayRoom.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

AEngineeringBayRoom::AEngineeringBayRoom()
{
	// Set default values
	SteamVentDamage = 10.0f;
	ElectricalSurgeDamage = 15.0f;
	HazardInterval = 5.0f;
	ExplosiveBarrelSpawnChance = 0.3f;
	bEmergencyLightingActive = false;
	CurrentHazardPattern = 0;
}

void AEngineeringBayRoom::BeginPlay()
{
	Super::BeginPlay();
	
	// Start industrial ambient sound
	if (IndustrialAmbientSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), IndustrialAmbientSound, GetActorLocation());
	}
}

void AEngineeringBayRoom::ActivateRoom(URoomDataAsset* RoomData)
{
	Super::ActivateRoom(RoomData);
	
	UE_LOG(LogTemp, Log, TEXT("EngineeringBayRoom: Activating with industrial theme"));
	
	// Start hazard sequence
	StartHazardSequence();
	
	// Enable emergency lighting for dramatic effect
	ToggleEmergencyLighting(true);
	
	// Start flickering lights
	GetWorld()->GetTimerManager().SetTimer(FlickerTimer, this, 
		&AEngineeringBayRoom::UpdateFlickeringLights, 0.1f + FMath::RandRange(0.0f, 0.5f), true);
	
	// Spawn initial steam vents
	for (int32 i = 0; i < 2; i++)
	{
		TriggerSteamVentBurst();
	}
	
	// Play metal creaking sound periodically
	if (MetalCreakingSound)
	{
		FTimerHandle CreakTimer;
		GetWorld()->GetTimerManager().SetTimer(CreakTimer, [this]()
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), MetalCreakingSound, 
				GetActorLocation() + FVector(FMath::RandRange(-500, 500), FMath::RandRange(-500, 500), 0));
		}, 10.0f, true);
	}
}

void AEngineeringBayRoom::DeactivateRoom()
{
	Super::DeactivateRoom();
	
	// Stop all hazard sequences
	StopHazardSequence();
	
	// Clear timers
	GetWorld()->GetTimerManager().ClearTimer(HazardSequenceTimer);
	GetWorld()->GetTimerManager().ClearTimer(FlickerTimer);
	
	// Disable emergency lighting
	ToggleEmergencyLighting(false);
	
	// Clear active hazard zones
	ActiveSteamVents.Empty();
	ActiveElectricalZones.Empty();
}

void AEngineeringBayRoom::TriggerSteamVentBurst()
{
	if (HazardSpawnPoints.Num() == 0)
	{
		return;
	}
	
	// Select a random hazard point
	int32 RandomIndex = FMath::RandRange(0, HazardSpawnPoints.Num() - 1);
	UArrowComponent* SpawnPoint = HazardSpawnPoints[RandomIndex];
	
	if (!SpawnPoint)
	{
		return;
	}
	
	FVector VentLocation = SpawnPoint->GetComponentLocation();
	
	// Spawn steam particle effect
	if (SteamVentEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SteamVentEffect, VentLocation, 
			FRotator(-90.0f, 0.0f, 0.0f));
	}
	
	// Play steam sound
	if (SteamHissSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SteamHissSound, VentLocation);
	}
	
	// Add to active vents
	ActiveSteamVents.Add(VentLocation);
	
	// Apply damage in area after a short delay
	FTimerHandle DamageTimer;
	GetWorld()->GetTimerManager().SetTimer(DamageTimer, [this, VentLocation]()
	{
		// Apply radial damage
		UGameplayStatics::ApplyRadialDamage(
			GetWorld(),
			SteamVentDamage,
			VentLocation,
			200.0f, // Radius
			nullptr,
			TArray<AActor*>(),
			this,
			nullptr,
			false
		);
		
		// Remove from active vents after 3 seconds
		FTimerHandle RemoveTimer;
		GetWorld()->GetTimerManager().SetTimer(RemoveTimer, [this, VentLocation]()
		{
			ActiveSteamVents.Remove(VentLocation);
		}, 3.0f, false);
		
	}, 0.5f, false);
	
	UE_LOG(LogTemp, Log, TEXT("EngineeringBayRoom: Triggered steam vent at %s"), *VentLocation.ToString());
}

void AEngineeringBayRoom::BurstPipe(FVector Location)
{
	// Spawn pipe burst effect
	if (PipeBurstEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PipeBurstEffect, Location);
	}
	
	// Create a hazard zone that persists
	FTimerHandle* HazardTimer = new FTimerHandle();
	GetWorld()->GetTimerManager().SetTimer(*HazardTimer, [this, Location]()
	{
		// Apply continuous damage in the area
		UGameplayStatics::ApplyRadialDamage(
			GetWorld(),
			SteamVentDamage * 0.5f,
			Location,
			150.0f,
			nullptr,
			TArray<AActor*>(),
			this,
			nullptr,
			false
		);
	}, 1.0f, true);
	
	// Stop after 10 seconds
	FTimerHandle StopTimer;
	GetWorld()->GetTimerManager().SetTimer(StopTimer, [this, HazardTimer]()
	{
		GetWorld()->GetTimerManager().ClearTimer(*HazardTimer);
		delete HazardTimer;
	}, 10.0f, false);
	
	UE_LOG(LogTemp, Log, TEXT("EngineeringBayRoom: Pipe burst at %s"), *Location.ToString());
}

void AEngineeringBayRoom::ActivateMachinery(int32 MachineIndex)
{
	// This would activate specific machinery in the room
	// For now, create a rotating hazard zone
	
	FVector MachineLocation = GetActorLocation() + FVector(
		FMath::RandRange(-500, 500),
		FMath::RandRange(-500, 500),
		0
	);
	
	// Visual feedback
	DrawDebugSphere(GetWorld(), MachineLocation, 300.0f, 32, FColor::Orange, false, 5.0f);
	
	// Apply damage in rotating pattern
	float RotationSpeed = 90.0f; // Degrees per second
	float* CurrentAngle = new float(0.0f);
	
	FTimerHandle* RotationTimer = new FTimerHandle();
	GetWorld()->GetTimerManager().SetTimer(*RotationTimer, [this, MachineLocation, CurrentAngle, RotationSpeed]()
	{
		*CurrentAngle += RotationSpeed * 0.1f; // Update every 0.1 seconds
		
		// Calculate damage position
		FVector DamagePos = MachineLocation + FVector(
			FMath::Cos(FMath::DegreesToRadians(*CurrentAngle)) * 200.0f,
			FMath::Sin(FMath::DegreesToRadians(*CurrentAngle)) * 200.0f,
			0.0f
		);
		
		// Apply damage at position
		UGameplayStatics::ApplyRadialDamage(
			GetWorld(),
			5.0f,
			DamagePos,
			100.0f,
			nullptr,
			TArray<AActor*>(),
			this,
			nullptr,
			false
		);
		
	}, 0.1f, true);
	
	// Stop after 8 seconds
	FTimerHandle StopTimer;
	GetWorld()->GetTimerManager().SetTimer(StopTimer, [this, RotationTimer, CurrentAngle]()
	{
		GetWorld()->GetTimerManager().ClearTimer(*RotationTimer);
		delete RotationTimer;
		delete CurrentAngle;
	}, 8.0f, false);
	
	UE_LOG(LogTemp, Log, TEXT("EngineeringBayRoom: Activated machinery %d"), MachineIndex);
}

void AEngineeringBayRoom::CreateElectricalSurge()
{
	if (InteractableSpawnPoints.Num() == 0)
	{
		return;
	}
	
	// Select random location
	int32 RandomIndex = FMath::RandRange(0, InteractableSpawnPoints.Num() - 1);
	UArrowComponent* SpawnPoint = InteractableSpawnPoints[RandomIndex];
	
	if (!SpawnPoint)
	{
		return;
	}
	
	FVector SurgeLocation = SpawnPoint->GetComponentLocation();
	
	// Warning phase (sparks before damage)
	if (ElectricalSurgeEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElectricalSurgeEffect, SurgeLocation);
	}
	
	// Play electrical sound
	if (ElectricalSurgeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ElectricalSurgeSound, SurgeLocation);
	}
	
	// Add to active zones
	ActiveElectricalZones.Add(SurgeLocation);
	
	// Apply damage after warning period
	FTimerHandle DamageTimer;
	GetWorld()->GetTimerManager().SetTimer(DamageTimer, [this, SurgeLocation]()
	{
		// Apply electrical damage in area
		UGameplayStatics::ApplyRadialDamage(
			GetWorld(),
			ElectricalSurgeDamage,
			SurgeLocation,
			250.0f,
			nullptr,
			TArray<AActor*>(),
			this,
			nullptr,
			false
		);
		
		// Chain to nearby electrical zones
		for (const FVector& Zone : ActiveElectricalZones)
		{
			if (Zone != SurgeLocation && FVector::Dist(Zone, SurgeLocation) < 500.0f)
			{
				// Create electrical arc effect between zones
				DrawDebugLine(GetWorld(), SurgeLocation, Zone, FColor::Cyan, false, 0.5f, 0, 5.0f);
				
				// Damage along the line
				FVector Direction = (Zone - SurgeLocation).GetSafeNormal();
				float Distance = FVector::Dist(Zone, SurgeLocation);
				
				for (float D = 0; D < Distance; D += 50.0f)
				{
					FVector PointOnLine = SurgeLocation + Direction * D;
					UGameplayStatics::ApplyRadialDamage(
						GetWorld(),
						ElectricalSurgeDamage * 0.5f,
						PointOnLine,
						50.0f,
						nullptr,
						TArray<AActor*>(),
						this,
						nullptr,
						false
					);
				}
			}
		}
		
		// Remove from active zones
		FTimerHandle RemoveTimer;
		GetWorld()->GetTimerManager().SetTimer(RemoveTimer, [this, SurgeLocation]()
		{
			ActiveElectricalZones.Remove(SurgeLocation);
		}, 2.0f, false);
		
	}, 1.0f, false);
	
	UE_LOG(LogTemp, Log, TEXT("EngineeringBayRoom: Created electrical surge at %s"), *SurgeLocation.ToString());
}

void AEngineeringBayRoom::ToggleEmergencyLighting(bool bEnabled)
{
	bEmergencyLightingActive = bEnabled;
	
	// This would control actual lights in the level
	// For now, log the state change
	UE_LOG(LogTemp, Log, TEXT("EngineeringBayRoom: Emergency lighting %s"), 
		bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
	
	// Could broadcast an event that Blueprint lights respond to
	// OnEmergencyLightingChanged.Broadcast(bEnabled);
}

void AEngineeringBayRoom::StartHazardSequence()
{
	// Start the hazard pattern timer
	GetWorld()->GetTimerManager().SetTimer(HazardSequenceTimer, [this]()
	{
		// Cycle through different hazard patterns
		switch (CurrentHazardPattern)
		{
			case 0:
				TriggerSteamVentBurst();
				break;
			case 1:
				CreateElectricalSurge();
				break;
			case 2:
				ActivateMachinery(FMath::RandRange(0, 2));
				break;
			case 3:
				// Multiple hazards at once
				TriggerSteamVentBurst();
				CreateElectricalSurge();
				break;
			default:
				CurrentHazardPattern = -1; // Reset
				break;
		}
		
		CurrentHazardPattern = (CurrentHazardPattern + 1) % 4;
		
	}, HazardInterval, true);
	
	UE_LOG(LogTemp, Log, TEXT("EngineeringBayRoom: Started hazard sequence (Interval: %.1f)"), HazardInterval);
}

void AEngineeringBayRoom::StopHazardSequence()
{
	GetWorld()->GetTimerManager().ClearTimer(HazardSequenceTimer);
	UE_LOG(LogTemp, Log, TEXT("EngineeringBayRoom: Stopped hazard sequence"));
}

void AEngineeringBayRoom::UpdateFlickeringLights()
{
	// This would control actual light actors in the level
	// For demonstration, we'll use debug visualization
	
	bool bLightOn = FMath::RandBool();
	
	if (bLightOn)
	{
		// Draw debug light representation
		DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0, 0, 400), 50, 16, 
			FColor::Yellow, false, 0.15f);
	}
	
	// Vary the next flicker time
	GetWorld()->GetTimerManager().SetTimer(FlickerTimer, this, 
		&AEngineeringBayRoom::UpdateFlickeringLights, 
		bLightOn ? FMath::RandRange(0.5f, 2.0f) : FMath::RandRange(0.05f, 0.2f), 
		false);
}

void AEngineeringBayRoom::ApplyEnvironmentalEffects()
{
	Super::ApplyEnvironmentalEffects();
	
	// Engineering bay specific effects
	// - Reduced visibility from steam
	// - Electrical interference
	// - Metal fatigue sounds
	
	UE_LOG(LogTemp, Log, TEXT("EngineeringBayRoom: Applied industrial environmental effects"));
}

void AEngineeringBayRoom::RemoveEnvironmentalEffects()
{
	Super::RemoveEnvironmentalEffects();
	
	// Clear all active hazard zones
	ActiveSteamVents.Empty();
	ActiveElectricalZones.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("EngineeringBayRoom: Removed industrial environmental effects"));
}