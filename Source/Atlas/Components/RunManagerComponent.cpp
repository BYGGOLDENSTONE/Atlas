#include "RunManagerComponent.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Atlas/Characters/GameCharacterBase.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Components/HealthComponent.h"
#include "Atlas/Components/StationIntegrityComponent.h"
#include "Atlas/Rooms/RoomBase.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreaming.h"
#include "TimerManager.h"
#include "EngineUtils.h"

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
	
	// Find all room actors placed in the world
	AllRoomActors.Empty();
	for (TActorIterator<ARoomBase> It(GetWorld()); It; ++It)
	{
		ARoomBase* RoomActor = *It;
		if (RoomActor)
		{
			AllRoomActors.Add(RoomActor);
			UE_LOG(LogTemp, Log, TEXT("Found room actor: %s at location %s"), 
				*RoomActor->GetName(), *RoomActor->GetActorLocation().ToString());
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("RunManagerComponent: Found %d room actors in world"), AllRoomActors.Num());
	
	// Try to load room data assets if none configured
	if (AllRoomDataAssets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RunManagerComponent: No rooms configured in Blueprint, attempting to load from content"));
		
		// Load room data assets from the actual Content/Dataassets/rooms/ folder
		const TArray<FString> RoomAssetPaths = {
			TEXT("/Game/Dataassets/rooms/DA_Room_EngineeringBay"),
			TEXT("/Game/Dataassets/rooms/DA_Room_CombatArena"),
			TEXT("/Game/Dataassets/rooms/DA_Room_MedicalBay"),
			TEXT("/Game/Dataassets/rooms/DA_Room_Cargohold"), // Note: Cargohold not CargoHold
			TEXT("/Game/Dataassets/rooms/DA_Room_Bridge")
		};
		
		for (const FString& AssetPath : RoomAssetPaths)
		{
			if (URoomDataAsset* RoomData = LoadObject<URoomDataAsset>(nullptr, *AssetPath))
			{
				// Fix missing AppearOnLevels configuration
				if (RoomData->AppearOnLevels.Num() == 0)
				{
					// Default: All rooms appear on levels 1-4, Bridge only on level 5
					if (RoomData->RoomName.ToString().Contains("Bridge"))
					{
						RoomData->AppearOnLevels = {5};
					}
					else
					{
						RoomData->AppearOnLevels = {1, 2, 3, 4};
					}
					UE_LOG(LogTemp, Warning, TEXT("  Room %s had no AppearOnLevels, set defaults"), *RoomData->RoomName.ToString());
				}
				
				AllRoomDataAssets.Add(RoomData);
				UE_LOG(LogTemp, Log, TEXT("  Loaded room: %s"), *RoomData->RoomName.ToString());
			}
		}
		
		// If still no rooms, create test rooms
		if (AllRoomDataAssets.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("RunManagerComponent: Creating 5 test rooms for testing"));
			
			for (int32 i = 1; i <= 5; i++)
			{
				URoomDataAsset* TestRoom = NewObject<URoomDataAsset>(this, *FString::Printf(TEXT("TestRoom_%d"), i));
				TestRoom->RoomID = FName(*FString::Printf(TEXT("TestRoom%d"), i));
				TestRoom->RoomName = FText::FromString(FString::Printf(TEXT("Test Room %d"), i));
				TestRoom->Difficulty = ERoomDifficulty::Easy;
				TestRoom->RoomTheme = ERewardCategory::Defense;
				TestRoom->bCanRepeat = (i <= 2); // First 2 rooms can repeat
				TestRoom->RoomSelectionWeight = 1.0f;
				TestRoom->EnemyBasePower = i; // Increasing difficulty
				
				// All test rooms appear on all levels for testing
				TestRoom->AppearOnLevels = {1, 2, 3, 4, 5};
				
				AllRoomDataAssets.Add(TestRoom);
				UE_LOG(LogTemp, Log, TEXT("  Created test room: %s"), *TestRoom->RoomName.ToString());
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("RunManagerComponent: Total rooms available: %d"), AllRoomDataAssets.Num());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("RunManagerComponent: Loaded %d room data assets from Blueprint"), AllRoomDataAssets.Num());
		
		// Fix any rooms that don't have AppearOnLevels configured
		for (URoomDataAsset* Room : AllRoomDataAssets)
		{
			if (Room && Room->AppearOnLevels.Num() == 0)
			{
				// Default: All rooms appear on levels 1-4, Bridge only on level 5
				if (Room->RoomName.ToString().Contains("Bridge"))
				{
					Room->AppearOnLevels = {5};
				}
				else
				{
					Room->AppearOnLevels = {1, 2, 3, 4};
				}
				UE_LOG(LogTemp, Warning, TEXT("  Fixed room %s - added default AppearOnLevels"), *Room->RoomName.ToString());
			}
		}
	}
	
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
	CurrentRoomActor = nullptr;
	
	// Randomize room actors order (all rooms have equal importance)
	RandomizedRoomOrder = AllRoomActors;
	
	// Shuffle the rooms randomly
	for (int32 i = RandomizedRoomOrder.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		RandomizedRoomOrder.Swap(i, j);
	}
	
	UE_LOG(LogTemp, Log, TEXT("Randomized %d rooms for run"), RandomizedRoomOrder.Num());
	
	// Also randomize data assets for compatibility
	RandomizeRoomOrder();
	
	// Set initial state
	SetRunState(ERunState::PreRun);
}

void URunManagerComponent::StartNewRun()
{
	UE_LOG(LogTemp, Log, TEXT("Starting new run"));
	
	InitializeRun();
	
	// Check if we have room actors
	if (RandomizedRoomOrder.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No room actors found in world"));
		EndRun(false, FText::FromString(TEXT("No rooms available")));
		return;
	}
	
	// Get first room actor from randomized order
	CurrentRoomActor = RandomizedRoomOrder[0];
	if (!CurrentRoomActor)
	{
		UE_LOG(LogTemp, Error, TEXT("First room actor is null"));
		EndRun(false, FText::FromString(TEXT("Invalid room configuration")));
		return;
	}
	
	// Match with a data asset based on room type
	CurrentRoom = nullptr;
	if (AllRoomDataAssets.Num() > 0)
	{
		for (URoomDataAsset* DataAsset : AllRoomDataAssets)
		{
			if (DataAsset && DataAsset->RoomType == CurrentRoomActor->RoomTypeForTesting)
			{
				CurrentRoom = DataAsset;
				UE_LOG(LogTemp, Log, TEXT("Matched room actor with data asset: %s"), *DataAsset->RoomName.ToString());
				break;
			}
		}
	}
	
	// Teleport player to first room
	CurrentRoomActor->TeleportPlayerToRoom();
	
	// Activate the room
	if (CurrentRoom)
	{
		CurrentRoomActor->ActivateRoom(CurrentRoom);
	}
	
	// Subscribe to room completion event
	CurrentRoomActor->OnRoomCompleted.AddDynamic(this, &URunManagerComponent::OnRoomActorCompleted);
	
	SetRunState(ERunState::RoomIntro);
	
	// The room will handle enemy spawning after its delay
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		SetRunState(ERunState::Combat);
	}, CombatStartDelay, false);
	
	UE_LOG(LogTemp, Warning, TEXT("Started run in room: %s"), *CurrentRoomActor->GetName());
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
		
		// Deactivate current room
		if (CurrentRoomActor)
		{
			CurrentRoomActor->DeactivateRoom();
		}
		
		// Check if we have a next room (CurrentLevel gets incremented after room completion)
		if (CurrentLevel > RandomizedRoomOrder.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("No more rooms available"));
			EndRun(false, FText::FromString(TEXT("Room progression error")));
			return;
		}
		
		// Get next room actor from randomized order (CurrentLevel is 1-based)
		CurrentRoomActor = RandomizedRoomOrder[CurrentLevel - 1];
		if (!CurrentRoomActor)
		{
			UE_LOG(LogTemp, Error, TEXT("Next room actor is null"));
			EndRun(false, FText::FromString(TEXT("Invalid room configuration")));
			return;
		}
		
		// Match with a data asset if available based on room type
		CurrentRoom = nullptr;
		if (AllRoomDataAssets.Num() > 0)
		{
			for (URoomDataAsset* DataAsset : AllRoomDataAssets)
			{
				if (DataAsset && DataAsset->RoomType == CurrentRoomActor->RoomTypeForTesting)
				{
					CurrentRoom = DataAsset;
					break;
				}
			}
		}
		
		// Teleport player to next room
		CurrentRoomActor->TeleportPlayerToRoom();
		
		// Activate the room
		if (CurrentRoom)
		{
			CurrentRoomActor->ActivateRoom(CurrentRoom);
		}
		
		// Subscribe to room completion event
		CurrentRoomActor->OnRoomCompleted.AddDynamic(this, &URunManagerComponent::OnRoomActorCompleted);
		
		SetRunState(ERunState::RoomIntro);
		
		// The room will handle enemy spawning after its delay
		FTimerHandle CombatTimer;
		GetWorld()->GetTimerManager().SetTimer(CombatTimer, [this]()
		{
			SetRunState(ERunState::Combat);
		}, CombatStartDelay, false);
		
		UE_LOG(LogTemp, Warning, TEXT("Transitioned to room %d: %s"), CurrentLevel, *CurrentRoomActor->GetName());
		
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
	
	// Clean up previous room if it exists
	if (CurrentRoomInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("Destroying previous room instance"));
		CurrentRoomInstance->DeactivateRoom();
		CurrentRoomInstance->Destroy();
		CurrentRoomInstance = nullptr;
	}
	
	CurrentRoom = Room;
	RoomStartTime = GetWorld()->GetTimeSeconds();
	
	// Remove from remaining rooms if not repeatable
	if (!Room->bCanRepeat)
	{
		RemainingRooms.Remove(Room);
	}
	
	// Direct room spawning - no complex streaming needed
	TSubclassOf<ARoomBase> RoomClass = ARoomBase::StaticClass();
	
	UE_LOG(LogTemp, Log, TEXT("Spawning room directly: %s"), *Room->RoomName.ToString());
	
	// Spawn the room actor with appropriate class
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Name = FName(*FString::Printf(TEXT("Room_%s"), *Room->RoomName.ToString()));
	
	CurrentRoomInstance = GetWorld()->SpawnActor<ARoomBase>(RoomClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	
	if (!CurrentRoomInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn room actor for: %s"), *Room->RoomName.ToString());
		EndRun(false, FText::FromString(TEXT("Room spawn failed")));
		return;
	}
	
	// Apply environmental hazards through room instance
	if (CurrentRoomInstance)
	{
		// Room instance handles its own hazards
		ApplyRoomHazards();
	}
	
	// Play ambient sound
	if (Room->AmbientSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Room->AmbientSound);
	}
	
	// Fire event
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
			// Subscribe to the OnDeath event to track when enemy is defeated
			EnemyHealth->OnDeath.AddDynamic(this, &URunManagerComponent::OnEnemyDefeated);
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
	
	const UEnum* StateEnum = StaticEnum<ERunState>();
	FString OldStateName = StateEnum ? StateEnum->GetNameStringByValue((int64)OldState) : TEXT("Unknown");
	FString NewStateName = StateEnum ? StateEnum->GetNameStringByValue((int64)NewState) : TEXT("Unknown");
	UE_LOG(LogTemp, Log, TEXT("Run state changed: %s -> %s"), *OldStateName, *NewStateName);
	
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

void URunManagerComponent::OnEnemyDefeated(AActor* KilledBy)
{
	// This is called when the current room enemy dies
	// KilledBy parameter indicates who killed the enemy (usually the player)
	UE_LOG(LogTemp, Log, TEXT("Enemy defeated by %s!"), KilledBy ? *KilledBy->GetName() : TEXT("Unknown"));
	
	CurrentRoomEnemy = nullptr;
	SetRunState(ERunState::Victory);
	
	// Auto-complete the room and move to next after delay
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		CompleteCurrentRoom();
		
		// If run is not complete, transition to next room
		if (!IsRunComplete())
		{
			TransitionToNextRoom();
		}
	}, 2.0f, false);
}

void URunManagerComponent::OnRoomActorCompleted(ARoomBase* CompletedRoom)
{
	// This is called when a room completes (enemy defeated)
	if (CompletedRoom != CurrentRoomActor)
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Room completed: %s"), *CompletedRoom->GetName());
	
	SetRunState(ERunState::Victory);
	
	// Auto-complete the room and move to next after delay
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		CompleteCurrentRoom();
		
		// If run is not complete, transition to next room
		if (!IsRunComplete())
		{
			TransitionToNextRoom();
		}
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

// ========================================
// TEST ARENA SUPPORT IMPLEMENTATION
// ========================================

void URunManagerComponent::GoToRoom(const FString& RoomName)
{
	// Find all room actors in the world if not already cached
	if (TestArenaRooms.Num() == 0)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomBase::StaticClass(), FoundActors);
		
		for (AActor* Actor : FoundActors)
		{
			if (ARoomBase* Room = Cast<ARoomBase>(Actor))
			{
				TestArenaRooms.Add(Room);
			}
		}
	}
	
	// Map room names to types
	ERoomType TargetType = ERoomType::EngineeringBay;
	if (RoomName.Equals("EngineeringBay", ESearchCase::IgnoreCase))
		TargetType = ERoomType::EngineeringBay;
	else if (RoomName.Equals("CombatArena", ESearchCase::IgnoreCase))
		TargetType = ERoomType::CombatArena;
	else if (RoomName.Equals("MedicalBay", ESearchCase::IgnoreCase))
		TargetType = ERoomType::MedicalBay;
	else if (RoomName.Equals("CargoHold", ESearchCase::IgnoreCase))
		TargetType = ERoomType::CargoHold;
	else if (RoomName.Equals("Bridge", ESearchCase::IgnoreCase))
		TargetType = ERoomType::Bridge;
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unknown room name: %s"), *RoomName);
		UE_LOG(LogTemp, Warning, TEXT("Valid names: EngineeringBay, CombatArena, MedicalBay, CargoHold, Bridge"));
		return;
	}
	
	// Find the room actor
	ARoomBase* TargetRoom = GetRoomActorByType(TargetType);
	if (TargetRoom)
	{
		// Reset previous room if exists
		if (CurrentRoomInstance && CurrentRoomInstance != TargetRoom)
		{
			CurrentRoomInstance->ResetRoom();
		}
		
		// Set as current room
		CurrentRoomInstance = TargetRoom;
		bTestArenaMode = true;
		
		// Teleport player to room
		TargetRoom->TeleportPlayerToRoom();
		
		// Update state
		SetRunState(ERunState::RoomIntro);
		
		UE_LOG(LogTemp, Log, TEXT("Teleported to room: %s"), *RoomName);
		
		// Start combat after delay
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			SetRunState(ERunState::Combat);
		}, 2.0f, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find room actor for: %s"), *RoomName);
		UE_LOG(LogTemp, Warning, TEXT("Make sure room actors are placed in the test arena level"));
	}
}

void URunManagerComponent::CompleteRoomTest()
{
	if (!CurrentRoomInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("No current room to complete"));
		return;
	}
	
	// Mark room as completed
	CurrentRoomInstance->CompleteRoom();
	CurrentRoomInstance->bTestRoomCompleted = true;
	
	// Clear any enemies
	if (CurrentRoomEnemy)
	{
		CurrentRoomEnemy->Destroy();
		CurrentRoomEnemy = nullptr;
	}
	
	const UEnum* RoomEnum = StaticEnum<ERoomType>();
	FString RoomName = RoomEnum ? RoomEnum->GetNameStringByValue((int64)CurrentRoomInstance->RoomTypeForTesting) : TEXT("Unknown");
	UE_LOG(LogTemp, Log, TEXT("Room completed: %s"), *RoomName);
	
	// Update progress
	CurrentLevel++;
	if (CurrentLevel > 5)
	{
		UE_LOG(LogTemp, Log, TEXT("All 5 rooms complete! Run successful!"));
		SetRunState(ERunState::RunComplete);
		CurrentLevel = 5;
	}
	else
	{
		SetRunState(ERunState::RoomComplete);
		UE_LOG(LogTemp, Log, TEXT("Ready for next room. Current level: %d/5"), CurrentLevel);
	}
}

void URunManagerComponent::ResetAllRooms()
{
	// Find all room actors
	if (TestArenaRooms.Num() == 0)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomBase::StaticClass(), FoundActors);
		
		for (AActor* Actor : FoundActors)
		{
			if (ARoomBase* Room = Cast<ARoomBase>(Actor))
			{
				TestArenaRooms.Add(Room);
			}
		}
	}
	
	// Reset each room
	for (ARoomBase* Room : TestArenaRooms)
	{
		if (Room)
		{
			Room->ResetRoom();
		}
	}
	
	// Reset run state
	CurrentLevel = 1;
	TestSequenceIndex = 0;
	CurrentRoomInstance = nullptr;
	CurrentRoomEnemy = nullptr;
	SetRunState(ERunState::PreRun);
	
	UE_LOG(LogTemp, Log, TEXT("All rooms reset. Ready for new test run."));
}

void URunManagerComponent::DebugRooms()
{
	UE_LOG(LogTemp, Warning, TEXT("=== ROOM DEBUG INFO ==="));
	UE_LOG(LogTemp, Warning, TEXT("Current Level: %d/5"), CurrentLevel);
	const UEnum* StateEnum2 = StaticEnum<ERunState>();
	FString StateName = StateEnum2 ? StateEnum2->GetNameStringByValue((int64)CurrentRunState) : TEXT("Unknown");
	UE_LOG(LogTemp, Warning, TEXT("Current State: %s"), *StateName);
	UE_LOG(LogTemp, Warning, TEXT("Test Arena Mode: %s"), bTestArenaMode ? TEXT("YES") : TEXT("NO"));
	
	if (CurrentRoomInstance)
	{
		const UEnum* RoomEnum2 = StaticEnum<ERoomType>();
		FString CurrentRoomName = RoomEnum2 ? RoomEnum2->GetNameStringByValue((int64)CurrentRoomInstance->RoomTypeForTesting) : TEXT("Unknown");
		UE_LOG(LogTemp, Warning, TEXT("Current Room: %s"), *CurrentRoomName);
		UE_LOG(LogTemp, Warning, TEXT("  - Active: %s"), CurrentRoomInstance->IsRoomActive() ? TEXT("YES") : TEXT("NO"));
		UE_LOG(LogTemp, Warning, TEXT("  - Completed: %s"), CurrentRoomInstance->bTestRoomCompleted ? TEXT("YES") : TEXT("NO"));
		UE_LOG(LogTemp, Warning, TEXT("  - Player In Room: %s"), CurrentRoomInstance->IsPlayerInRoom() ? TEXT("YES") : TEXT("NO"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No current room instance"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Test Arena Rooms Found: %d"), TestArenaRooms.Num());
	for (ARoomBase* Room : TestArenaRooms)
	{
		if (Room)
		{
			const UEnum* RoomEnum3 = StaticEnum<ERoomType>();
			FString RoomTypeName = RoomEnum3 ? RoomEnum3->GetNameStringByValue((int64)Room->RoomTypeForTesting) : TEXT("Unknown");
			UE_LOG(LogTemp, Warning, TEXT("  - %s (Completed: %s)"), 
				*RoomTypeName,
				Room->bTestRoomCompleted ? TEXT("YES") : TEXT("NO"));
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("======================"));
}

void URunManagerComponent::TestRoomSequence()
{
	UE_LOG(LogTemp, Log, TEXT("Starting test room sequence..."));
	
	// Reset everything first
	ResetAllRooms();
	
	// Start with first room
	TestSequenceIndex = 0;
	TArray<FString> RoomNames = {
		TEXT("EngineeringBay"),
		TEXT("CombatArena"),
		TEXT("MedicalBay"),
		TEXT("CargoHold"),
		TEXT("Bridge")
	};
	
	if (RoomNames.IsValidIndex(TestSequenceIndex))
	{
		GoToRoom(RoomNames[TestSequenceIndex]);
		
		// Set up timer to auto-complete rooms
		FTimerHandle SequenceTimer;
		GetWorld()->GetTimerManager().SetTimer(SequenceTimer, [this, RoomNames]()
		{
			// Complete current room
			CompleteRoomTest();
			
			// Move to next room
			TestSequenceIndex++;
			if (TestSequenceIndex < RoomNames.Num())
			{
				FTimerHandle NextRoomTimer;
				GetWorld()->GetTimerManager().SetTimer(NextRoomTimer, [this, RoomNames]()
				{
					GoToRoom(RoomNames[TestSequenceIndex]);
				}, 2.0f, false);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Test sequence complete!"));
			}
		}, 5.0f, true); // Auto-complete every 5 seconds
	}
}

ARoomBase* URunManagerComponent::GetRoomActorByType(ERoomType RoomType) const
{
	for (ARoomBase* Room : TestArenaRooms)
	{
		if (Room && Room->RoomTypeForTesting == RoomType)
		{
			return Room;
		}
	}
	
	// If not in cache, try to find in world
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomBase::StaticClass(), FoundActors);
	
	for (AActor* Actor : FoundActors)
	{
		if (ARoomBase* Room = Cast<ARoomBase>(Actor))
		{
			if (Room->RoomTypeForTesting == RoomType)
			{
				return Room;
			}
		}
	}
	
	return nullptr;
}