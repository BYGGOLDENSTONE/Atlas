#include "RunManagerComponent.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "Atlas/Characters/GameCharacterBase.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Components/HealthComponent.h"
#include "Atlas/Components/StationIntegrityComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreaming.h"
#include "TimerManager.h"

URunManagerComponent::URunManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Set default spawn point
	DefaultEnemySpawnPoint.SetLocation(FVector(0.0f, 500.0f, 100.0f));
	DefaultEnemySpawnPoint.SetRotation(FQuat::MakeFromEuler(FVector(0.0f, 0.0f, 180.0f)));
}

void URunManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Get player references
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PlayerCharacter = Cast<AGameCharacterBase>(PC->GetPawn());
		if (PlayerCharacter)
		{
			PlayerSlotManager = PlayerCharacter->FindComponentByClass<USlotManagerComponent>();
			
			// Subscribe to player death event
			if (UHealthComponent* HealthComp = PlayerCharacter->GetHealthComponent())
			{
				// TODO: Subscribe to death event when available
			}
		}
	}
	
	// Subscribe to station integrity events
	// TODO: Find station integrity manager and subscribe to destruction event
}

void URunManagerComponent::InitializeRun()
{
	UE_LOG(LogTemp, Log, TEXT("Initializing new run"));
	
	// Reset progress
	RunProgress = FRunProgressData();
	CurrentLevel = 1;
	CompletedRooms.Empty();
	
	// Randomize room order
	RandomizeRoomOrder();
	
	// Set initial state
	SetRunState(ERunState::PreRun);
}

void URunManagerComponent::StartNewRun()
{
	UE_LOG(LogTemp, Log, TEXT("Starting new run"));
	
	InitializeRun();
	
	// Select and load first room
	if (URoomDataAsset* FirstRoom = SelectNextRoom())
	{
		LoadRoom(FirstRoom);
		SetRunState(ERunState::RoomIntro);
		
		// Start combat after delay
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			SetRunState(ERunState::Combat);
			SpawnRoomEnemy();
		}, CombatStartDelay, false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to select first room"));
		EndRun(false, FText::FromString(TEXT("Failed to initialize rooms")));
	}
}

void URunManagerComponent::ResumeRun(const FRunProgressData& SavedProgress)
{
	UE_LOG(LogTemp, Log, TEXT("Resuming run from level %d"), SavedProgress.CurrentLevel);
	
	// Restore progress
	RunProgress = SavedProgress;
	CurrentLevel = SavedProgress.CurrentLevel;
	
	// Rebuild completed rooms list
	CompletedRooms.Empty();
	for (const FName& RoomID : SavedProgress.CompletedRoomIDs)
	{
		for (URoomDataAsset* Room : AllRoomDataAssets)
		{
			if (Room && Room->RoomID == RoomID)
			{
				CompletedRooms.Add(Room);
				break;
			}
		}
	}
	
	// Rebuild remaining rooms
	RandomizeRoomOrder();
	for (URoomDataAsset* CompletedRoom : CompletedRooms)
	{
		RemainingRooms.Remove(CompletedRoom);
	}
	
	// Load current room
	if (URoomDataAsset* NextRoom = SelectNextRoom())
	{
		LoadRoom(NextRoom);
		SetRunState(ERunState::RoomIntro);
	}
}

void URunManagerComponent::CompleteCurrentRoom()
{
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("No current room to complete"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Completing room: %s"), *CurrentRoom->RoomName.ToString());
	
	// Add to completed list
	CompletedRooms.Add(CurrentRoom);
	RunProgress.CompletedRoomIDs.Add(CurrentRoom->RoomID);
	
	// Update statistics
	UpdateRunStats();
	
	// Broadcast completion event
	OnRoomCompleted.Broadcast(CurrentRoom);
	
	// Clear the room
	ClearCurrentRoom();
	
	// Move to next level
	CurrentLevel++;
	
	// Check if run is complete
	if (IsRunComplete())
	{
		EndRun(true, FText::FromString(TEXT("All rooms completed!")));
	}
	else
	{
		SetRunState(ERunState::RoomComplete);
	}
}

void URunManagerComponent::TransitionToNextRoom()
{
	if (IsRunComplete())
	{
		EndRun(true, FText::FromString(TEXT("Run completed successfully!")));
		return;
	}
	
	// Begin transition
	BeginRoomTransition();
	
	// After transition duration, load next room
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		CompleteRoomTransition();
		
		// Select and load next room
		if (URoomDataAsset* NextRoom = SelectNextRoom())
		{
			LoadRoom(NextRoom);
			SetRunState(ERunState::RoomIntro);
			
			// Start combat after intro
			FTimerHandle CombatTimer;
			GetWorld()->GetTimerManager().SetTimer(CombatTimer, [this]()
			{
				SetRunState(ERunState::Combat);
				SpawnRoomEnemy();
			}, CombatStartDelay, false);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to select next room"));
			EndRun(false, FText::FromString(TEXT("Room selection failed")));
		}
		
	}, RoomTransitionDuration, false);
}

void URunManagerComponent::EndRun(bool bSuccess, const FText& Reason)
{
	UE_LOG(LogTemp, Log, TEXT("Ending run - Success: %s, Reason: %s"), 
		bSuccess ? TEXT("True") : TEXT("False"), *Reason.ToString());
	
	if (bSuccess)
	{
		SetRunState(ERunState::RunComplete);
		OnRunCompleted.Broadcast();
		
		// Save equipped rewards for next run
		if (PlayerSlotManager)
		{
			PlayerSlotManager->SaveRewardsForNextRun();
		}
	}
	else
	{
		SetRunState(ERunState::RunFailed);
		OnRunFailed.Broadcast(Reason);
	}
	
	// Clear current room
	ClearCurrentRoom();
	
	// Return to main menu or show results screen
	// TODO: Implement transition to results/menu
}

void URunManagerComponent::AbandonRun()
{
	UE_LOG(LogTemp, Log, TEXT("Abandoning current run"));
	
	EndRun(false, FText::FromString(TEXT("Run abandoned by player")));
}

URoomDataAsset* URunManagerComponent::SelectNextRoom()
{
	// Filter rooms valid for current level
	TArray<URoomDataAsset*> ValidRooms;
	float TotalWeight = 0.0f;
	
	for (URoomDataAsset* Room : RemainingRooms)
	{
		if (Room && Room->IsValidForLevel(CurrentLevel))
		{
			// Check if room can repeat or hasn't been used
			if (Room->bCanRepeat || !CompletedRooms.Contains(Room))
			{
				ValidRooms.Add(Room);
				TotalWeight += Room->RoomSelectionWeight;
			}
		}
	}
	
	if (ValidRooms.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid rooms for level %d"), CurrentLevel);
		return nullptr;
	}
	
	// Select room based on weights
	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;
	
	for (URoomDataAsset* Room : ValidRooms)
	{
		CurrentWeight += Room->RoomSelectionWeight;
		if (RandomValue <= CurrentWeight)
		{
			return Room;
		}
	}
	
	// Fallback to first valid room
	return ValidRooms[0];
}

void URunManagerComponent::LoadRoom(URoomDataAsset* Room)
{
	if (!Room)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot load null room"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Loading room: %s"), *Room->RoomName.ToString());
	
	CurrentRoom = Room;
	RoomStartTime = GetWorld()->GetTimeSeconds();
	
	// Remove from remaining rooms if not repeatable
	if (!Room->bCanRepeat)
	{
		RemainingRooms.Remove(Room);
	}
	
	// Load room level if specified
	LoadRoomLevel(Room);
	
	// Apply environmental hazards
	ApplyRoomHazards();
	
	// Play ambient sound
	if (Room->AmbientSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Room->AmbientSound);
	}
	
	// Broadcast room started event
	OnRoomStarted.Broadcast(Room);
}

void URunManagerComponent::SpawnRoomEnemy()
{
	if (!CurrentRoom || !CurrentRoom->UniqueEnemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("No enemy to spawn for current room"));
		return;
	}
	
	// Determine spawn point
	FTransform SpawnTransform = DefaultEnemySpawnPoint;
	if (CurrentRoom->EnemySpawnPoints.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, CurrentRoom->EnemySpawnPoints.Num() - 1);
		SpawnTransform = CurrentRoom->EnemySpawnPoints[RandomIndex];
	}
	
	// Spawn the enemy
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	CurrentRoomEnemy = GetWorld()->SpawnActor<AGameCharacterBase>(
		CurrentRoom->UniqueEnemy,
		SpawnTransform.GetLocation(),
		SpawnTransform.GetRotation().Rotator(),
		SpawnParams
	);
	
	if (CurrentRoomEnemy)
	{
		UE_LOG(LogTemp, Log, TEXT("Spawned enemy: %s"), *CurrentRoom->EnemyName.ToString());
		
		// Apply difficulty scaling
		ApplyEnemyScaling(CurrentRoomEnemy);
		
		// Subscribe to enemy death event
		if (UHealthComponent* EnemyHealth = CurrentRoomEnemy->GetHealthComponent())
		{
			// TODO: Subscribe to death event
			// For now, we'll check manually in tick or use a different approach
		}
		
		// Start combat music
		if (CurrentRoom->CombatMusic)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), CurrentRoom->CombatMusic);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn enemy"));
		EndRun(false, FText::FromString(TEXT("Enemy spawn failed")));
	}
}

void URunManagerComponent::ApplyRoomHazards()
{
	if (!CurrentRoom)
		return;
		
	switch (CurrentRoom->EnvironmentalHazard)
	{
		case ERoomHazard::LowGravity:
			// TODO: Reduce gravity for all actors
			UE_LOG(LogTemp, Log, TEXT("Applying Low Gravity hazard"));
			break;
			
		case ERoomHazard::ElectricalSurges:
			// TODO: Spawn periodic electrical damage zones
			UE_LOG(LogTemp, Log, TEXT("Applying Electrical Surges hazard"));
			break;
			
		case ERoomHazard::HullBreach:
			// TODO: Apply constant station integrity drain
			if (CurrentRoom->IntegrityDrainRate > 0.0f)
			{
				// TODO: Start integrity drain timer
				UE_LOG(LogTemp, Log, TEXT("Applying Hull Breach with %f damage/sec"), CurrentRoom->IntegrityDrainRate);
			}
			break;
			
		case ERoomHazard::ToxicLeak:
			// TODO: Apply DoT to player in certain areas
			UE_LOG(LogTemp, Log, TEXT("Applying Toxic Leak hazard"));
			break;
			
		case ERoomHazard::SystemMalfunction:
			// TODO: Randomly disable player abilities
			UE_LOG(LogTemp, Log, TEXT("Applying System Malfunction hazard"));
			break;
			
		default:
			break;
	}
}

void URunManagerComponent::ClearCurrentRoom()
{
	UE_LOG(LogTemp, Log, TEXT("Clearing current room"));
	
	// Destroy enemy if still alive
	if (CurrentRoomEnemy && IsValid(CurrentRoomEnemy))
	{
		CurrentRoomEnemy->Destroy();
		CurrentRoomEnemy = nullptr;
	}
	
	// Unload room level if streaming
	if (CurrentRoom)
	{
		UnloadRoomLevel(CurrentRoom);
	}
	
	// Stop any active hazards
	// TODO: Clean up hazard effects
	
	CurrentRoom = nullptr;
}

void URunManagerComponent::SetRunState(ERunState NewState)
{
	if (CurrentRunState == NewState)
		return;
		
	ERunState OldState = CurrentRunState;
	CurrentRunState = NewState;
	
	UE_LOG(LogTemp, Log, TEXT("Run state changed: %s -> %s"), 
		*UEnum::GetValueAsString(OldState), 
		*UEnum::GetValueAsString(NewState));
	
	// Handle state-specific logic
	switch (NewState)
	{
		case ERunState::RoomIntro:
			// TODO: Show room intro UI
			break;
			
		case ERunState::Victory:
			// Enemy defeated, prepare for reward selection
			RunProgress.TotalEnemiesDefeated++;
			// TODO: Show victory UI
			break;
			
		case ERunState::RewardSelection:
			// TODO: Present reward choices
			break;
			
		case ERunState::RunComplete:
			// Save final statistics
			UpdateRunStats();
			break;
			
		default:
			break;
	}
	
	// Broadcast state change
	OnRunStateChanged.Broadcast(NewState);
}

bool URunManagerComponent::IsRunActive() const
{
	return CurrentRunState != ERunState::PreRun && 
	       CurrentRunState != ERunState::RunComplete && 
	       CurrentRunState != ERunState::RunFailed;
}

FRunProgressData URunManagerComponent::GetRunProgress() const
{
	FRunProgressData Progress = RunProgress;
	Progress.CurrentLevel = CurrentLevel;
	
	// Update current health values
	if (PlayerCharacter)
	{
		if (UHealthComponent* Health = PlayerCharacter->GetHealthComponent())
		{
			Progress.PlayerHealth = Health->GetCurrentHealth();
		}
		
		if (UStationIntegrityComponent* Station = PlayerCharacter->FindComponentByClass<UStationIntegrityComponent>())
		{
			Progress.StationIntegrity = Station->GetCurrentIntegrity();
		}
	}
	
	return Progress;
}

int32 URunManagerComponent::CalculateEnemyPower() const
{
	if (!PlayerSlotManager)
		return 1;
		
	// As per GDD: Enemy power = Player equipped slots + 1
	return PlayerSlotManager->GetUsedSlotCount() + 1;
}

void URunManagerComponent::ApplyEnemyScaling(AGameCharacterBase* Enemy)
{
	if (!Enemy)
		return;
		
	int32 EnemyPower = CalculateEnemyPower();
	
	UE_LOG(LogTemp, Log, TEXT("Applying enemy scaling - Power Level: %d"), EnemyPower);
	
	// TODO: Apply scaling to enemy stats
	// This would interact with the AIDifficultyComponent when implemented
	
	// Apply special abilities from room configuration
	if (CurrentRoom)
	{
		for (const FGameplayTag& AbilityTag : CurrentRoom->EnemyAbilities)
		{
			// TODO: Grant ability to enemy
			UE_LOG(LogTemp, Log, TEXT("Granting enemy ability: %s"), *AbilityTag.ToString());
		}
	}
}

void URunManagerComponent::RandomizeRoomOrder()
{
	RemainingRooms = AllRoomDataAssets;
	
	// Shuffle the array using Fisher-Yates algorithm
	for (int32 i = RemainingRooms.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		RemainingRooms.Swap(i, j);
	}
	
	UE_LOG(LogTemp, Log, TEXT("Randomized %d rooms for run"), RemainingRooms.Num());
}

void URunManagerComponent::BeginRoomTransition()
{
	UE_LOG(LogTemp, Log, TEXT("Beginning room transition"));
	
	// TODO: Fade out screen
	// TODO: Show loading/transition UI
}

void URunManagerComponent::CompleteRoomTransition()
{
	UE_LOG(LogTemp, Log, TEXT("Completing room transition"));
	
	// TODO: Fade in screen
	// TODO: Hide loading/transition UI
}

void URunManagerComponent::LoadRoomLevel(URoomDataAsset* Room)
{
	if (!Room || !Room->RoomLevel.IsValid())
		return;
		
	// Use level streaming to load the room
	FLatentActionInfo LatentInfo;
	UGameplayStatics::LoadStreamLevel(
		GetWorld(),
		FName(*Room->RoomLevel.GetAssetName()),
		true,  // Make visible
		true,  // Should block on load
		LatentInfo
	);
	
	UE_LOG(LogTemp, Log, TEXT("Loading room level: %s"), *Room->RoomLevel.GetAssetName());
}

void URunManagerComponent::UnloadRoomLevel(URoomDataAsset* Room)
{
	if (!Room || !Room->RoomLevel.IsValid())
		return;
		
	// Unload the streamed level
	FLatentActionInfo LatentInfo;
	UGameplayStatics::UnloadStreamLevel(
		GetWorld(),
		FName(*Room->RoomLevel.GetAssetName()),
		LatentInfo,
		true  // Should block on unload
	);
	
	UE_LOG(LogTemp, Log, TEXT("Unloading room level: %s"), *Room->RoomLevel.GetAssetName());
}

void URunManagerComponent::OnEnemyDefeated(AActor* DefeatedEnemy)
{
	if (DefeatedEnemy != CurrentRoomEnemy)
		return;
		
	UE_LOG(LogTemp, Log, TEXT("Enemy defeated!"));
	
	CurrentRoomEnemy = nullptr;
	SetRunState(ERunState::Victory);
	
	// Move to reward selection after a brief delay
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		SetRunState(ERunState::RewardSelection);
		// TODO: Present reward choices
	}, 2.0f, false);
}

void URunManagerComponent::OnPlayerDeath()
{
	UE_LOG(LogTemp, Log, TEXT("Player died!"));
	
	EndRun(false, FText::FromString(TEXT("Player health depleted")));
}

void URunManagerComponent::OnStationDestroyed()
{
	UE_LOG(LogTemp, Log, TEXT("Station destroyed!"));
	
	EndRun(false, FText::FromString(TEXT("Station integrity reached zero")));
}

void URunManagerComponent::UpdateRunStats()
{
	// Update damage dealt, parries, etc.
	// This would be called throughout combat to track statistics
	
	if (PlayerCharacter)
	{
		// TODO: Pull stats from combat components
	}
	
	UE_LOG(LogTemp, Log, TEXT("Run stats updated - Enemies defeated: %d"), RunProgress.TotalEnemiesDefeated);
}