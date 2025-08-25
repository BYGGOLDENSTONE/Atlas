#include "RoomBase.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Atlas/Characters/GameCharacterBase.h"
#include "Atlas/Components/HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ARoomBase::ARoomBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	RoomRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RoomRoot"));
	SetRootComponent(RoomRoot);

	// Create room bounds
	RoomBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomBounds"));
	RoomBounds->SetupAttachment(RoomRoot);
	RoomBounds->SetBoxExtent(FVector(1000.0f, 1000.0f, 500.0f));
	RoomBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create spawn points
	PlayerSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("PlayerSpawnPoint"));
	PlayerSpawnPoint->SetupAttachment(RoomRoot);
	PlayerSpawnPoint->SetRelativeLocation(FVector(0.0f, -800.0f, 100.0f));
	PlayerSpawnPoint->SetArrowColor(FLinearColor::Green);

	EnemySpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("EnemySpawnPoint"));
	EnemySpawnPoint->SetupAttachment(RoomRoot);
	EnemySpawnPoint->SetRelativeLocation(FVector(0.0f, 800.0f, 100.0f));
	EnemySpawnPoint->SetArrowColor(FLinearColor::Red);

	RewardSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("RewardSpawnPoint"));
	RewardSpawnPoint->SetupAttachment(RoomRoot);
	RewardSpawnPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	RewardSpawnPoint->SetArrowColor(FLinearColor::Yellow);

	// Create exit trigger
	ExitTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitTrigger"));
	ExitTrigger->SetupAttachment(RoomRoot);
	ExitTrigger->SetRelativeLocation(FVector(0.0f, 1000.0f, 100.0f));
	ExitTrigger->SetBoxExtent(FVector(200.0f, 200.0f, 300.0f));
	ExitTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExitTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExitTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Create hazard spawn points (5 default points)
	for (int32 i = 0; i < 5; i++)
	{
		FString PointName = FString::Printf(TEXT("HazardSpawnPoint_%d"), i);
		UArrowComponent* HazardPoint = CreateDefaultSubobject<UArrowComponent>(*PointName);
		HazardPoint->SetupAttachment(RoomRoot);
		HazardPoint->SetArrowColor(FLinearColor(1.0f, 0.5f, 0.0f)); // Orange
		HazardSpawnPoints.Add(HazardPoint);
	}

	// Set default hazard spawn locations
	if (HazardSpawnPoints.Num() >= 5)
	{
		HazardSpawnPoints[0]->SetRelativeLocation(FVector(-500.0f, 0.0f, 50.0f));
		HazardSpawnPoints[1]->SetRelativeLocation(FVector(500.0f, 0.0f, 50.0f));
		HazardSpawnPoints[2]->SetRelativeLocation(FVector(0.0f, -500.0f, 50.0f));
		HazardSpawnPoints[3]->SetRelativeLocation(FVector(0.0f, 500.0f, 50.0f));
		HazardSpawnPoints[4]->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
	}

	// Create interactable spawn points (3 default points)
	for (int32 i = 0; i < 3; i++)
	{
		FString PointName = FString::Printf(TEXT("InteractableSpawnPoint_%d"), i);
		UArrowComponent* InteractablePoint = CreateDefaultSubobject<UArrowComponent>(*PointName);
		InteractablePoint->SetupAttachment(RoomRoot);
		InteractablePoint->SetArrowColor(FLinearColor(0.0f, 0.5f, 1.0f)); // Blue
		InteractableSpawnPoints.Add(InteractablePoint);
	}

	// Set default interactable spawn locations
	if (InteractableSpawnPoints.Num() >= 3)
	{
		InteractableSpawnPoints[0]->SetRelativeLocation(FVector(-300.0f, -300.0f, 100.0f));
		InteractableSpawnPoints[1]->SetRelativeLocation(FVector(300.0f, -300.0f, 100.0f));
		InteractableSpawnPoints[2]->SetRelativeLocation(FVector(0.0f, 300.0f, 100.0f));
	}

	// Default configuration
	EnemySpawnDelay = 2.0f;
	bLockExitUntilClear = true;
	bIsRoomActive = false;
}

void ARoomBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Bind exit trigger overlap
	if (ExitTrigger)
	{
		ExitTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARoomBase::OnExitTriggerOverlap);
	}
}

void ARoomBase::ActivateRoom(URoomDataAsset* RoomData)
{
	if (!RoomData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ARoomBase::ActivateRoom - No RoomData provided"));
		return;
	}

	CurrentRoomData = RoomData;
	bIsRoomActive = true;
	RoomActivationTime = GetWorld()->GetTimeSeconds();

	// Apply environmental effects
	ApplyEnvironmentalEffects();

	// Start combat music
	StartCombatMusic();

	// Spawn enemy after delay
	if (EnemySpawnDelay > 0.0f)
	{
		FTimerHandle SpawnTimer;
		GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &ARoomBase::SpawnRoomEnemy, EnemySpawnDelay, false);
	}
	else
	{
		SpawnRoomEnemy();
	}

	// Spawn hazards
	SpawnRoomHazards();

	// Spawn interactables
	SpawnRoomInteractables();

	// Lock exit if configured
	if (bLockExitUntilClear && ExitTrigger)
	{
		ExitTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Fire events
	OnRoomActivated.Broadcast(this);
	BP_OnRoomActivated();
}

void ARoomBase::DeactivateRoom()
{
	bIsRoomActive = false;

	// Clear all spawned entities
	ClearSpawnedEntities();

	// Remove environmental effects
	RemoveEnvironmentalEffects();

	// Stop combat music
	StopCombatMusic();

	// Fire blueprint event
	BP_OnRoomDeactivated();
}

void ARoomBase::CompleteRoom()
{
	if (!bIsRoomActive)
	{
		return;
	}

	// Unlock exit
	if (ExitTrigger)
	{
		ExitTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// Fire events
	OnRoomCompleted.Broadcast(this);
	BP_OnRoomCompleted();

	// Log completion
	UE_LOG(LogTemp, Log, TEXT("Room Completed: %s"), CurrentRoomData ? *CurrentRoomData->RoomName.ToString() : TEXT("Unknown"));
}

void ARoomBase::SpawnRoomEnemy()
{
	if (!CurrentRoomData || !CurrentRoomData->UniqueEnemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("ARoomBase::SpawnRoomEnemy - No enemy class defined"));
		return;
	}

	// Get spawn transform
	FTransform SpawnTransform = GetEnemySpawnPoint();

	// Spawn enemy
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	SpawnedEnemy = GetWorld()->SpawnActor<AGameCharacterBase>(CurrentRoomData->UniqueEnemy, SpawnTransform, SpawnParams);

	if (SpawnedEnemy)
	{
		// Bind to enemy death
		if (UHealthComponent* HealthComp = SpawnedEnemy->FindComponentByClass<UHealthComponent>())
		{
			HealthComp->OnDeath.AddDynamic(this, &ARoomBase::OnEnemyDefeated);
		}

		// Fire custom spawn event
		BP_CustomEnemySpawn(SpawnedEnemy);

		UE_LOG(LogTemp, Log, TEXT("Spawned Enemy: %s"), *CurrentRoomData->EnemyName.ToString());
	}
}

void ARoomBase::SpawnRoomHazards()
{
	if (!CurrentRoomData || CurrentRoomData->EnvironmentalHazard == ERoomHazard::None)
	{
		return;
	}

	// Get hazard class for this room's hazard type
	TSubclassOf<AActor>* HazardClass = HazardClasses.Find(CurrentRoomData->EnvironmentalHazard);
	if (!HazardClass || !(*HazardClass))
	{
		// Fire blueprint event for custom hazard spawning
		BP_CustomHazardSpawn();
		return;
	}

	// Spawn hazards at designated points
	for (UArrowComponent* SpawnPoint : HazardSpawnPoints)
	{
		if (!SpawnPoint)
			continue;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* Hazard = GetWorld()->SpawnActor<AActor>(*HazardClass, SpawnPoint->GetComponentTransform(), SpawnParams);
		if (Hazard)
		{
			SpawnedHazards.Add(Hazard);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned %d hazards for room"), SpawnedHazards.Num());
}

void ARoomBase::SpawnRoomInteractables()
{
	// Spawn interactables at designated points
	int32 InteractableIndex = 0;
	for (UArrowComponent* SpawnPoint : InteractableSpawnPoints)
	{
		if (!SpawnPoint || InteractableIndex >= InteractableClasses.Num())
			break;

		TSubclassOf<AActor> InteractableClass = InteractableClasses[InteractableIndex];
		if (!InteractableClass)
		{
			InteractableIndex++;
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* Interactable = GetWorld()->SpawnActor<AActor>(InteractableClass, SpawnPoint->GetComponentTransform(), SpawnParams);
		if (Interactable)
		{
			SpawnedInteractables.Add(Interactable);
		}

		InteractableIndex++;
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned %d interactables for room"), SpawnedInteractables.Num());
}

void ARoomBase::ClearSpawnedEntities()
{
	// Destroy enemy
	if (SpawnedEnemy)
	{
		SpawnedEnemy->Destroy();
		SpawnedEnemy = nullptr;
	}

	// Destroy hazards
	for (AActor* Hazard : SpawnedHazards)
	{
		if (Hazard)
		{
			Hazard->Destroy();
		}
	}
	SpawnedHazards.Empty();

	// Destroy interactables
	for (AActor* Interactable : SpawnedInteractables)
	{
		if (Interactable)
		{
			Interactable->Destroy();
		}
	}
	SpawnedInteractables.Empty();
}

FTransform ARoomBase::GetPlayerSpawnPoint() const
{
	return PlayerSpawnPoint ? PlayerSpawnPoint->GetComponentTransform() : FTransform();
}

FTransform ARoomBase::GetEnemySpawnPoint() const
{
	return EnemySpawnPoint ? EnemySpawnPoint->GetComponentTransform() : FTransform();
}

FTransform ARoomBase::GetRewardSpawnPoint() const
{
	return RewardSpawnPoint ? RewardSpawnPoint->GetComponentTransform() : FTransform();
}

TArray<FTransform> ARoomBase::GetHazardSpawnPoints() const
{
	TArray<FTransform> Transforms;
	for (UArrowComponent* Point : HazardSpawnPoints)
	{
		if (Point)
		{
			Transforms.Add(Point->GetComponentTransform());
		}
	}
	return Transforms;
}

TArray<FTransform> ARoomBase::GetInteractableSpawnPoints() const
{
	TArray<FTransform> Transforms;
	for (UArrowComponent* Point : InteractableSpawnPoints)
	{
		if (Point)
		{
			Transforms.Add(Point->GetComponentTransform());
		}
	}
	return Transforms;
}

void ARoomBase::OnEnemyDefeated(AActor* KilledBy)
{
	// OnDeath passes who killed the enemy, not the enemy itself
	// We just need to check if we had a spawned enemy
	if (!SpawnedEnemy)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Enemy defeated in room: %s by %s"), 
		CurrentRoomData ? *CurrentRoomData->RoomName.ToString() : TEXT("Unknown"),
		KilledBy ? *KilledBy->GetName() : TEXT("Unknown"));

	// Clear the enemy reference
	SpawnedEnemy = nullptr;
	
	// Complete the room
	CompleteRoom();
}

void ARoomBase::OnExitTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if player overlapped and room is complete
	if (!bIsRoomActive || SpawnedEnemy)
	{
		return;
	}

	// Check if it's the player
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (OtherActor == PlayerPawn)
	{
		// Notify that player wants to exit
		UE_LOG(LogTemp, Log, TEXT("Player entered exit trigger for room: %s"), 
			CurrentRoomData ? *CurrentRoomData->RoomName.ToString() : TEXT("Unknown"));
		
		// Room manager will handle the transition
	}
}

void ARoomBase::ApplyEnvironmentalEffects()
{
	if (!CurrentRoomData)
	{
		return;
	}

	// Apply based on hazard type
	switch (CurrentRoomData->EnvironmentalHazard)
	{
		case ERoomHazard::LowGravity:
			// Reduce gravity
			if (ACharacter* PlayerChar = GetWorld()->GetFirstPlayerController()->GetPawn<ACharacter>())
			{
				PlayerChar->GetCharacterMovement()->GravityScale = 0.3f;
			}
			break;

		case ERoomHazard::ElectricalSurges:
			// Periodic damage will be handled by hazard actors
			break;

		case ERoomHazard::HullBreach:
			// Apply integrity drain
			break;

		case ERoomHazard::ToxicLeak:
			// Apply poison effect
			break;

		case ERoomHazard::SystemMalfunction:
			// Random effects
			break;

		default:
			break;
	}
}

void ARoomBase::RemoveEnvironmentalEffects()
{
	// Reset any modified values
	if (ACharacter* PlayerChar = GetWorld()->GetFirstPlayerController()->GetPawn<ACharacter>())
	{
		PlayerChar->GetCharacterMovement()->GravityScale = 1.0f;
	}
}

void ARoomBase::StartCombatMusic()
{
	USoundBase* MusicToPlay = OverrideCombatMusic ? OverrideCombatMusic : 
		(CurrentRoomData ? CurrentRoomData->CombatMusic : nullptr);

	if (MusicToPlay)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), MusicToPlay);
	}
}

void ARoomBase::StopCombatMusic()
{
	// Stop all sounds (simplified - in production use audio component)
	// This would be better handled with an audio component that can be stopped
}

// ========================================
// TEST ARENA SUPPORT IMPLEMENTATION
// ========================================

void ARoomBase::TeleportPlayerToRoom()
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("TeleportPlayerToRoom: World is null"));
		return;
	}
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("TeleportPlayerToRoom: No player controller found"));
		return;
	}
	
	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("TeleportPlayerToRoom: No player pawn found"));
		return;
	}
	
	// Calculate spawn position
	FVector SpawnLocation;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	
	if (PlayerSpawnPoint)
	{
		// Use spawn point if available
		SpawnLocation = GetActorLocation() + PlayerSpawnPoint->GetRelativeLocation();
		SpawnRotation = PlayerSpawnPoint->GetComponentRotation();
	}
	else
	{
		// Fallback: Use room location + offset
		SpawnLocation = GetActorLocation() + FVector(0, 0, 100);
		UE_LOG(LogTemp, Warning, TEXT("PlayerSpawnPoint is null for room, using default offset"));
	}
	
	// Teleport player
	PlayerPawn->SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
	PlayerPawn->SetActorRotation(SpawnRotation);
	
	// Reset velocity if it's a character
	if (ACharacter* PlayerChar = Cast<ACharacter>(PlayerPawn))
	{
		if (PlayerChar->GetCharacterMovement())
		{
			PlayerChar->GetCharacterMovement()->Velocity = FVector::ZeroVector;
			PlayerChar->GetCharacterMovement()->StopMovementImmediately();
		}
	}
	
	const UEnum* EnumPtr = StaticEnum<ERoomType>();
	FString RoomName = EnumPtr ? EnumPtr->GetNameStringByValue((int64)RoomTypeForTesting) : TEXT("Unknown");
	UE_LOG(LogTemp, Log, TEXT("Teleported player to room: %s at position %s"), 
		*RoomName,
		*SpawnLocation.ToString());
	
	// Activate this room if not already active
	if (!bIsRoomActive && CurrentRoomData)
	{
		ActivateRoom(CurrentRoomData);
	}
}

void ARoomBase::ResetRoom()
{
	// Clear all spawned entities
	ClearSpawnedEntities();
	
	// Reset room state
	bIsRoomActive = false;
	bTestRoomCompleted = false;
	SpawnedEnemy = nullptr;
	
	// Remove environmental effects
	RemoveEnvironmentalEffects();
	
	// Stop combat music
	StopCombatMusic();
	
	// Re-enable exit trigger if it was locked
	if (ExitTrigger)
	{
		ExitTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	
	const UEnum* EnumPtr2 = StaticEnum<ERoomType>();
	FString RoomName2 = EnumPtr2 ? EnumPtr2->GetNameStringByValue((int64)RoomTypeForTesting) : TEXT("Unknown");
	UE_LOG(LogTemp, Log, TEXT("Room reset: %s"), *RoomName2);
}

bool ARoomBase::IsPlayerInRoom() const
{
	if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
	{
		FVector PlayerLocation = PlayerPawn->GetActorLocation();
		FVector RoomCenter = TestArenaPosition;
		
		// Check if player is within room radius (2D check)
		float Distance2D = FVector::Dist2D(PlayerLocation, RoomCenter);
		
		return Distance2D <= RoomRadius;
	}
	
	return false;
}