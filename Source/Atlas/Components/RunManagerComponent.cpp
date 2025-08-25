#include "RunManagerComponent.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Atlas/Characters/GameCharacterBase.h"
#include "Atlas/Components/SlotManagerComponent.h"
#include "Atlas/Components/HealthComponent.h"
#include "Atlas/Components/StationIntegrityComponent.h"
#include "Atlas/UI/SRewardSelectionWidget.h"
#include "Atlas/UI/SRunProgressWidget.h"
#include "Atlas/UI/SEnemyHealthWidget.h"
#include "Atlas/UI/SSimpleSlotManagerWidget.h"
#include "Atlas/UI/SInventoryWidget.h"
#include "Atlas/Rooms/RoomBase.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreaming.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "Widgets/SViewport.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/GameViewportClient.h"

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
	
	// Don't create widget in BeginPlay - wait for StartNewRun command
	
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
				UE_LOG(LogTemp, Log, TEXT("  Loaded room: %s, EnemyName: %s"), 
					*RoomData->RoomName.ToString(), 
					RoomData->EnemyName.IsEmpty() ? TEXT("[EMPTY]") : *RoomData->EnemyName.ToString());
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

void URunManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Update health and integrity display periodically
	if (RunProgressWidget.IsValid())
	{
		if (!PlayerCharacter)
		{
			PlayerCharacter = Cast<AGameCharacterBase>(GetWorld()->GetFirstPlayerController()->GetPawn());
		}
		
		if (PlayerCharacter)
		{
			if (UHealthComponent* HealthComp = PlayerCharacter->FindComponentByClass<UHealthComponent>())
			{
				RunProgressWidget->UpdateHealth(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
			}
			
			if (UStationIntegrityComponent* IntegrityComp = PlayerCharacter->FindComponentByClass<UStationIntegrityComponent>())
			{
				RunProgressWidget->UpdateIntegrity(IntegrityComp->GetCurrentIntegrity(), IntegrityComp->GetMaxIntegrity());
			}
		}
	}
}

void URunManagerComponent::StartNewRun()
{
	UE_LOG(LogTemp, Log, TEXT("Starting new run"));
	
	// Create and show the run progress widget when run starts
	if (!RunProgressWidget.IsValid())
	{
		// Get player components for the widget
		AGameCharacterBase* PlayerChar = Cast<AGameCharacterBase>(GetWorld()->GetFirstPlayerController()->GetPawn());
		UHealthComponent* HealthComp = nullptr;
		UStationIntegrityComponent* IntegrityComp = nullptr;
		
		if (PlayerChar)
		{
			HealthComp = PlayerChar->FindComponentByClass<UHealthComponent>();
			IntegrityComp = PlayerChar->FindComponentByClass<UStationIntegrityComponent>();
		}
		
		// Only create widget if we have a player
		if (PlayerChar)
		{
			// Create the widget
			RunProgressWidget = SNew(SRunProgressWidget)
				.RunManager(this)
				.HealthComponent(HealthComp)
				.IntegrityComponent(IntegrityComp);
			
			// Add to viewport with lower Z-order than reward selection
			if (GEngine && GEngine->GameViewport && RunProgressWidget.IsValid())
			{
				GEngine->GameViewport->AddViewportWidgetContent(RunProgressWidget.ToSharedRef(), 10);
				UE_LOG(LogTemp, Log, TEXT("RunProgressWidget created and added to viewport"));
				
				// Initialize widget with current values if components exist
				if (HealthComp)
				{
					RunProgressWidget->UpdateHealth(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
					RunProgressWidget->UpdatePoise(HealthComp->GetCurrentPoise(), HealthComp->GetMaxPoise());
					
					// Subscribe to health and poise changes
					HealthComp->OnHealthChanged.AddDynamic(this, &URunManagerComponent::OnPlayerHealthChanged);
					HealthComp->OnPoiseChanged.AddDynamic(this, &URunManagerComponent::OnPlayerPoiseChanged);
				}
				
				if (IntegrityComp)
				{
					RunProgressWidget->UpdateIntegrity(IntegrityComp->GetCurrentIntegrity(), IntegrityComp->GetMaxIntegrity());
					
					// Subscribe to integrity changes
					IntegrityComp->OnIntegrityChanged.AddDynamic(this, &URunManagerComponent::OnPlayerIntegrityChanged);
				}
			}
			
			// Create the slot manager widget (compact display)
			if (USlotManagerComponent* SlotManager = PlayerChar->GetSlotManagerComponent())
			{
				SlotManagerWidget = SNew(SSimpleSlotManagerWidget)
					.SlotManager(SlotManager);
				
				// Add to viewport
				if (GEngine && GEngine->GameViewport && SlotManagerWidget.IsValid())
				{
					GEngine->GameViewport->AddViewportWidgetContent(SlotManagerWidget.ToSharedRef(), 15);
					UE_LOG(LogTemp, Log, TEXT("SlotManagerWidget created and added to viewport"));
				}
			}
		}
	}
	
	InitializeRun();
	
	// Display the run map showing all 5 rooms
	DisplayRunMap();
	
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
				UE_LOG(LogTemp, Warning, TEXT("Matched room DataAsset: %s, EnemyName: %s"), 
					*DataAsset->RoomName.ToString(),
					DataAsset->EnemyName.IsEmpty() ? TEXT("[EMPTY]") : *DataAsset->EnemyName.ToString());
				
				// Update the RunProgressWidget with the current room
				if (RunProgressWidget.IsValid())
				{
					RunProgressWidget->UpdateCurrentRoomInfo(CurrentRoom);
					RunProgressWidget->UpdateRoomProgress(0, CompletedRooms);
				}
				break;
			}
		}
	}
	
	// If no room data was found, create a temporary one
	if (!CurrentRoom && CurrentRoomActor)
	{
		const UEnum* RoomEnum = StaticEnum<ERoomType>();
		FString RoomName = RoomEnum ? RoomEnum->GetNameStringByValue((int64)CurrentRoomActor->RoomTypeForTesting) : TEXT("Unknown");
		
		URoomDataAsset* TempRoom = NewObject<URoomDataAsset>(this);
		TempRoom->RoomName = FText::FromString(RoomName);
		TempRoom->RoomType = CurrentRoomActor->RoomTypeForTesting;
		TempRoom->EnemyName = FText::FromString(FString::Printf(TEXT("%s Enemy"), *RoomName));
		CurrentRoom = TempRoom;
		
		if (RunProgressWidget.IsValid())
		{
			RunProgressWidget->UpdateCurrentRoomInfo(CurrentRoom);
			RunProgressWidget->UpdateRoomProgress(0, CompletedRooms);
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
	
	// Update run progress widget
	if (RunProgressWidget.IsValid())
	{
		RunProgressWidget->SetRoomCompleted(CurrentLevel - 1);
	}
	
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
		if (CurrentRoomActor && IsValid(CurrentRoomActor))
		{
			// Unbind from previous room's completion event to avoid crashes
			if (CurrentRoomActor->OnRoomCompleted.IsAlreadyBound(this, &URunManagerComponent::OnRoomActorCompleted))
			{
				CurrentRoomActor->OnRoomCompleted.RemoveDynamic(this, &URunManagerComponent::OnRoomActorCompleted);
			}
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
		
		// Update run progress widget with new room
		if (RunProgressWidget.IsValid())
		{
			RunProgressWidget->UpdateRoomProgress(CurrentLevel - 1, CompletedRooms);
			if (CurrentRoomActor && CurrentRoomActor->GetRoomData())
			{
				RunProgressWidget->UpdateCurrentRoomInfo(CurrentRoomActor->GetRoomData());
			}
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
					UE_LOG(LogTemp, Warning, TEXT("Matched room DataAsset: %s, EnemyName: %s"), 
						*DataAsset->RoomName.ToString(),
						DataAsset->EnemyName.IsEmpty() ? TEXT("[EMPTY]") : *DataAsset->EnemyName.ToString());
					
					// Update the widget with the matched room data
					if (RunProgressWidget.IsValid())
					{
						RunProgressWidget->UpdateCurrentRoomInfo(CurrentRoom);
					}
					break;
				}
			}
		}
		
		// Create fallback room data if no matching DataAsset was found
		if (!CurrentRoom && CurrentRoomActor)
		{
			const UEnum* RoomEnum = StaticEnum<ERoomType>();
			FString RoomName = RoomEnum ? RoomEnum->GetNameStringByValue((int64)CurrentRoomActor->RoomTypeForTesting) : TEXT("Unknown");
			
			URoomDataAsset* TempRoomData = NewObject<URoomDataAsset>(this);
			TempRoomData->RoomName = FText::FromString(RoomName);
			TempRoomData->RoomType = CurrentRoomActor->RoomTypeForTesting;
			TempRoomData->EnemyName = FText::FromString(FString::Printf(TEXT("%s Enemy"), *RoomName));
			CurrentRoom = TempRoomData;
			
			// Update the widget with the temporary room data
			if (RunProgressWidget.IsValid())
			{
				RunProgressWidget->UpdateCurrentRoomInfo(CurrentRoom);
			}
		}
		
		// Teleport player to next room
		if (CurrentRoomActor && IsValid(CurrentRoomActor))
		{
			CurrentRoomActor->TeleportPlayerToRoom();
			
			// Activate the room
			if (CurrentRoom)
			{
				CurrentRoomActor->ActivateRoom(CurrentRoom);
			}
			
			// Subscribe to room completion event
			// Check if already bound to avoid duplicate bindings
			if (!CurrentRoomActor->OnRoomCompleted.IsAlreadyBound(this, &URunManagerComponent::OnRoomActorCompleted))
			{
				CurrentRoomActor->OnRoomCompleted.AddDynamic(this, &URunManagerComponent::OnRoomActorCompleted);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CurrentRoomActor is invalid when trying to activate room"));
			EndRun(false, FText::FromString(TEXT("Room actor invalid")));
			return;
		}
		
		SetRunState(ERunState::RoomIntro);
		
		// The room will handle enemy spawning after its delay
		FTimerHandle CombatTimer;
		GetWorld()->GetTimerManager().SetTimer(CombatTimer, [this]()
		{
			SetRunState(ERunState::Combat);
		}, CombatStartDelay, false);
		
		UE_LOG(LogTemp, Warning, TEXT("Transitioned to room %d: %s"), CurrentLevel, *CurrentRoomActor->GetName());
		
		// Display updated map showing progress
		DisplayRunMap();
		
	}, RoomTransitionDuration, false);
}

void URunManagerComponent::EndRun(bool bSuccess, const FText& Reason)
{
	UE_LOG(LogTemp, Log, TEXT("Ending run - Success: %s, Reason: %s"), 
		bSuccess ? TEXT("True") : TEXT("False"), *Reason.ToString());
	
	// Remove run progress widget when run ends
	if (RunProgressWidget.IsValid())
	{
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(RunProgressWidget.ToSharedRef());
		}
		RunProgressWidget.Reset();
		UE_LOG(LogTemp, Log, TEXT("RunProgressWidget removed from viewport"));
	}
	
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
	
	// Update the RunProgressWidget with current room info
	if (RunProgressWidget.IsValid())
	{
		RunProgressWidget->UpdateCurrentRoomInfo(CurrentRoom);
	}
	
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
		
		// Show enemy health widget
		ShowEnemyHealthWidget(CurrentRoomEnemy);
		
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
	
	// Hide enemy health widget
	HideEnemyHealthWidget();
	
	CurrentRoomEnemy = nullptr;
	SetRunState(ERunState::Victory);
	
	// Start reward selection after a short delay
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		// Change state to reward selection and start the process
		SetRunState(ERunState::RewardSelection);
		StartRewardSelection();
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
	
	// Start reward selection after a short delay
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		// Check if this is the final room
		if (CurrentLevel >= 5)
		{
			UE_LOG(LogTemp, Log, TEXT("All 5 rooms complete! Run successful!"));
			SetRunState(ERunState::RunComplete);
			EndRun(true, FText::FromString(TEXT("Run completed successfully!")));
		}
		else
		{
			// Start reward selection for non-final rooms
			SetRunState(ERunState::RewardSelection);
			StartRewardSelection();
			// DO NOT transition to next room here - wait for reward selection to complete!
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
	
	if (!PlayerCharacter)
	{
		// Try to find player character if not cached
		PlayerCharacter = Cast<AGameCharacterBase>(GetWorld()->GetFirstPlayerController()->GetPawn());
	}
	
	if (PlayerCharacter)
	{
		// Update health and integrity in the widget
		if (RunProgressWidget.IsValid())
		{
			if (UHealthComponent* HealthComp = PlayerCharacter->FindComponentByClass<UHealthComponent>())
			{
				RunProgressWidget->UpdateHealth(HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth());
			}
			
			if (UStationIntegrityComponent* IntegrityComp = PlayerCharacter->FindComponentByClass<UStationIntegrityComponent>())
			{
				RunProgressWidget->UpdateIntegrity(IntegrityComp->GetCurrentIntegrity(), IntegrityComp->GetMaxIntegrity());
			}
		}
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
	
	// First ensure RunProgressWidget exists
	if (!RunProgressWidget.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("RunProgressWidget not created - creating now"));
		// Get player components for the widget
		AGameCharacterBase* PlayerChar = Cast<AGameCharacterBase>(GetWorld()->GetFirstPlayerController()->GetPawn());
		if (PlayerChar)
		{
			UHealthComponent* HealthComp = PlayerChar->FindComponentByClass<UHealthComponent>();
			UStationIntegrityComponent* IntegrityComp = PlayerChar->FindComponentByClass<UStationIntegrityComponent>();
			
			// Create the widget
			RunProgressWidget = SNew(SRunProgressWidget)
				.RunManager(this)
				.HealthComponent(HealthComp)
				.IntegrityComponent(IntegrityComp);
			
			// Add to viewport
			if (GEngine && GEngine->GameViewport && RunProgressWidget.IsValid())
			{
				GEngine->GameViewport->AddViewportWidgetContent(RunProgressWidget.ToSharedRef(), 10);
				UE_LOG(LogTemp, Log, TEXT("RunProgressWidget created in GoToRoom"));
			}
			
			// Create the slot manager widget if not exists (compact display)
			if (!SlotManagerWidget.IsValid())
			{
				if (USlotManagerComponent* SlotManager = PlayerChar->GetSlotManagerComponent())
				{
					SlotManagerWidget = SNew(SSimpleSlotManagerWidget)
						.SlotManager(SlotManager);
					
					// Add to viewport
					if (GEngine && GEngine->GameViewport && SlotManagerWidget.IsValid())
					{
						GEngine->GameViewport->AddViewportWidgetContent(SlotManagerWidget.ToSharedRef(), 15);
						UE_LOG(LogTemp, Log, TEXT("SlotManagerWidget created in GoToRoom"));
					}
				}
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
		
		// Debug logging for room data assets
		UE_LOG(LogTemp, Warning, TEXT("=== ROOM DATA DEBUG ==="));
		UE_LOG(LogTemp, Warning, TEXT("AllRoomDataAssets count: %d"), AllRoomDataAssets.Num());
		
		// Get the room data asset for this room type
		bool bFoundRoomData = false;
		if (AllRoomDataAssets.Num() > 0)
		{
			for (URoomDataAsset* RoomData : AllRoomDataAssets)
			{
				if (RoomData)
				{
					UE_LOG(LogTemp, Warning, TEXT("Checking RoomData: %s, Type: %d"), 
						*RoomData->RoomName.ToString(), (int32)RoomData->RoomType);
					
					if (RoomData->RoomType == TargetType)
					{
						CurrentRoom = RoomData;
						bFoundRoomData = true;
						UE_LOG(LogTemp, Warning, TEXT("FOUND matching room data: %s"), *CurrentRoom->RoomName.ToString());
						break;
					}
				}
			}
		}
		
		if (!bFoundRoomData)
		{
			// Create a temporary room data asset for testing if none exist
			UE_LOG(LogTemp, Warning, TEXT("No room data assets found, creating temporary data for: %s"), *RoomName);
			URoomDataAsset* TempRoomData = NewObject<URoomDataAsset>(this);
			TempRoomData->RoomName = FText::FromString(RoomName);
			TempRoomData->RoomType = TargetType;
			TempRoomData->EnemyName = FText::FromString(FString::Printf(TEXT("%s Enemy"), *RoomName));
			CurrentRoom = TempRoomData;
		}
		
		// Update the RunProgressWidget with current room info
		if (CurrentRoom)
		{
			UE_LOG(LogTemp, Warning, TEXT("CurrentRoom set to: %s"), *CurrentRoom->RoomName.ToString());
			
			if (RunProgressWidget.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("Updating RunProgressWidget with room: %s"), *CurrentRoom->RoomName.ToString());
				RunProgressWidget->UpdateCurrentRoomInfo(CurrentRoom);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("RunProgressWidget is not valid!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CurrentRoom is null!"));
		}
		
		// Teleport player to room
		TargetRoom->TeleportPlayerToRoom();
		
		// Update state
		SetRunState(ERunState::RoomIntro);
		
		UE_LOG(LogTemp, Log, TEXT("Teleported to room: %s"), *RoomName);
		
		// Start combat and spawn enemy after delay
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, TargetRoom]()
		{
			SetRunState(ERunState::Combat);
			
			// Spawn enemy for this room
			if (CurrentRoom)
			{
				if (CurrentRoom->UniqueEnemy)
				{
					UE_LOG(LogTemp, Warning, TEXT("Spawning enemy class: %s"), *CurrentRoom->UniqueEnemy->GetName());
					
					FTransform SpawnTransform = TargetRoom->GetEnemySpawnPoint();
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
						UE_LOG(LogTemp, Warning, TEXT("=== ENEMY SPAWNED ==="));
						UE_LOG(LogTemp, Warning, TEXT("Enemy: %s at %s"), 
							*CurrentRoom->EnemyName.ToString(), 
							*CurrentRoomEnemy->GetActorLocation().ToString());
						
						// Apply difficulty scaling
						ApplyEnemyScaling(CurrentRoomEnemy);
						
						// Show enemy health widget
						UE_LOG(LogTemp, Warning, TEXT("Calling ShowEnemyHealthWidget..."));
						ShowEnemyHealthWidget(CurrentRoomEnemy);
						
						// Subscribe to enemy death event
						if (UHealthComponent* EnemyHealth = CurrentRoomEnemy->GetHealthComponent())
						{
							UE_LOG(LogTemp, Warning, TEXT("Enemy health component found, subscribing to death event"));
							EnemyHealth->OnDeath.AddDynamic(this, &URunManagerComponent::OnEnemyDefeated);
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("Enemy has no health component!"));
						}
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to spawn enemy!"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("CurrentRoom->UniqueEnemy is null - using fallback enemy spawn"));
					
					// Try to spawn a default enemy if no unique enemy is set
					// This would need to be configured in Blueprint or we create a test enemy
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("CurrentRoom is null when trying to spawn enemy!"));
			}
		}), 2.0f, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find room actor for: %s"), *RoomName);
		UE_LOG(LogTemp, Warning, TEXT("Make sure room actors are placed in the test arena level"));
	}
}

void URunManagerComponent::CompleteRoomTest()
{
	UE_LOG(LogTemp, Warning, TEXT("CompleteRoomTest called"));
	UE_LOG(LogTemp, Warning, TEXT("CurrentRoomInstance: %s"), CurrentRoomInstance ? *CurrentRoomInstance->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("CurrentRoomActor: %s"), CurrentRoomActor ? *CurrentRoomActor->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("CurrentRoom (DataAsset): %s"), CurrentRoom ? *CurrentRoom->RoomName.ToString() : TEXT("NULL"));
	
	// Use CurrentRoomActor if CurrentRoomInstance is null
	ARoomBase* ActiveRoom = CurrentRoomInstance ? CurrentRoomInstance : CurrentRoomActor;
	
	if (!ActiveRoom)
	{
		UE_LOG(LogTemp, Warning, TEXT("No current room found - will still trigger reward selection"));
		// Don't return - we can still simulate enemy death and trigger rewards
	}
	
	// Clear any enemies (simulating enemy death)
	if (CurrentRoomEnemy)
	{
		CurrentRoomEnemy->Destroy();
		CurrentRoomEnemy = nullptr;
	}
	
	FString RoomName = TEXT("Unknown Room");
	if (ActiveRoom)
	{
		const UEnum* RoomEnum = StaticEnum<ERoomType>();
		RoomName = RoomEnum ? RoomEnum->GetNameStringByValue((int64)ActiveRoom->RoomTypeForTesting) : TEXT("Unknown");
	}
	else if (CurrentRoom)
	{
		RoomName = CurrentRoom->RoomName.ToString();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Simulating enemy defeat in room: %s"), *RoomName);
	
	// Set victory state
	SetRunState(ERunState::Victory);
	
	// Start reward selection after a short delay (simulating enemy death flow)
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		// Check if we're at the last room
		if (CurrentLevel >= 5)
		{
			UE_LOG(LogTemp, Log, TEXT("All 5 rooms complete! Run successful!"));
			SetRunState(ERunState::RunComplete);
			EndRun(true, FText::FromString(TEXT("Run completed successfully!")));
		}
		else
		{
			// Start reward selection for non-final rooms
			SetRunState(ERunState::RewardSelection);
			StartRewardSelection();
			// DO NOT transition to next room here - wait for reward selection to complete!
		}
	}, 2.0f, false);
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

// ========================================
// REWARD SELECTION IMPLEMENTATION
// ========================================

void URunManagerComponent::StartRewardSelection()
{
	if (bRewardSelectionActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reward selection already active"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT(""));
	UE_LOG(LogTemp, Warning, TEXT("================================================="));
	UE_LOG(LogTemp, Warning, TEXT("STARTING REWARD SELECTION"));
	UE_LOG(LogTemp, Warning, TEXT("================================================="));
	UE_LOG(LogTemp, Warning, TEXT("Current Room: %s"), CurrentRoom ? *CurrentRoom->RoomName.ToString() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Current Room Instance: %s"), CurrentRoomInstance ? *CurrentRoomInstance->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Current Room Actor: %s"), CurrentRoomActor ? *CurrentRoomActor->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Current Level: %d"), CurrentLevel);
	UE_LOG(LogTemp, Warning, TEXT("Current State: %s"), *UEnum::GetValueAsString(CurrentRunState));
	
	bRewardSelectionActive = true;
	
	// Get rewards from current room
	TArray<URewardDataAsset*> RewardChoices = GetRandomRewardsFromRoom(2);
	CurrentRewardChoices = RewardChoices;
	
	UE_LOG(LogTemp, Warning, TEXT("========== REWARD SELECTION =========="));
	UE_LOG(LogTemp, Warning, TEXT("Found %d reward choices:"), RewardChoices.Num());
	for (int32 i = 0; i < RewardChoices.Num(); i++)
	{
		if (RewardChoices[i])
		{
			UE_LOG(LogTemp, Warning, TEXT("  [%d] %s: %s"), i, *RewardChoices[i]->RewardName.ToString(), *RewardChoices[i]->Description.ToString());
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Use 'Atlas.SelectReward 0' or 'Atlas.SelectReward 1' to choose"));
	UE_LOG(LogTemp, Warning, TEXT("Use 'Atlas.CancelRewardSelection' to skip"));
	UE_LOG(LogTemp, Warning, TEXT("======================================"));
	
	// Also display on screen with actual reward data
	if (GEngine && RewardChoices.Num() >= 2)
	{
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(99, 30.0f, FColor::Green, TEXT("=== CHOOSE YOUR REWARD ==="));
		if (RewardChoices[0])
		{
			GEngine->AddOnScreenDebugMessage(100, 30.0f, FColor::Cyan, FString::Printf(TEXT("[0] %s"), *RewardChoices[0]->RewardName.ToString()));
		}
		if (RewardChoices[1])
		{
			GEngine->AddOnScreenDebugMessage(101, 30.0f, FColor::Yellow, FString::Printf(TEXT("[1] %s"), *RewardChoices[1]->RewardName.ToString()));
		}
		GEngine->AddOnScreenDebugMessage(102, 30.0f, FColor::Red, TEXT("Atlas.SelectReward 0/1 or Atlas.CancelRewardSelection"));
	}
	
	// Create proper Slate UI with actual rewards
	CreateRewardSelectionUI();
	
	UE_LOG(LogTemp, Warning, TEXT("Reward selection is now ACTIVE - waiting for player choice..."));
}

TArray<URewardDataAsset*> URunManagerComponent::GetRandomRewardsFromRoom(int32 Count)
{
	TArray<URewardDataAsset*> SelectedRewards;
	
	UE_LOG(LogTemp, Warning, TEXT("GetRandomRewardsFromRoom called with Count=%d"), Count);
	
	// If we have a current room with a reward pool, use it
	if (CurrentRoom && CurrentRoom->RewardPool.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Using room reward pool with %d rewards"), CurrentRoom->RewardPool.Num());
		SelectedRewards = CurrentRoom->SelectRandomRewards(Count, 1); // Player level 1 for now
	}
	else
	{
		// Create test rewards based on room type
		UE_LOG(LogTemp, Warning, TEXT("Creating test rewards for room testing"));
		SelectedRewards = CreateTestRewardsForRoom(Count);
	}
	
	// Store current choices for console commands
	CurrentRewardChoices = SelectedRewards;
	
	return SelectedRewards;
	
}

TArray<URewardDataAsset*> URunManagerComponent::CreateTestRewardsForRoom(int32 Count)
{
	TArray<URewardDataAsset*> TestRewards;
	
	// Determine room type for themed rewards
	ERoomType RoomType = ERoomType::EngineeringBay;  // Default
	ERewardCategory RoomTheme = ERewardCategory::PassiveStats; // Default
	
	if (CurrentRoom)
	{
		RoomType = CurrentRoom->RoomType;
		RoomTheme = CurrentRoom->RoomTheme;
	}
	else if (CurrentRoomInstance)
	{
		RoomType = CurrentRoomInstance->RoomTypeForTesting;
		// Set theme based on room type
		switch (RoomType)
		{
			case ERoomType::EngineeringBay: RoomTheme = ERewardCategory::PassiveStats; break;
			case ERoomType::CombatArena: RoomTheme = ERewardCategory::Offense; break;
			case ERoomType::MedicalBay: RoomTheme = ERewardCategory::PassiveAbility; break;
			case ERoomType::CargoHold: RoomTheme = ERewardCategory::Defense; break;
			case ERoomType::Bridge: RoomTheme = ERewardCategory::Interactable; break;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Creating test rewards for room type: %s"), *UEnum::GetValueAsString(RoomType));
	
	// Create room-specific rewards
	switch (RoomType)
	{
		case ERoomType::EngineeringBay:
		{
			if (URewardDataAsset* Reward1 = CreateTestReward("Speed Boost", "Increases movement speed by 25%", ERewardCategory::PassiveStats))
				TestRewards.Add(Reward1);
			if (URewardDataAsset* Reward2 = CreateTestReward("Efficient Systems", "Reduces ability cooldowns by 15%", ERewardCategory::PassiveStats))
				TestRewards.Add(Reward2);
			if (URewardDataAsset* Reward3 = CreateTestReward("Enhanced Servos", "Increases attack speed by 20%", ERewardCategory::PassiveStats))
				TestRewards.Add(Reward3);
			break;
		}
			
		case ERoomType::CombatArena:
		{
			if (URewardDataAsset* Reward1 = CreateTestReward("Power Strike", "Heavy attacks deal 50% more damage", ERewardCategory::Offense))
				TestRewards.Add(Reward1);
			if (URewardDataAsset* Reward2 = CreateTestReward("Combo Master", "Basic attacks can chain up to 4 hits", ERewardCategory::Offense))
				TestRewards.Add(Reward2);
			if (URewardDataAsset* Reward3 = CreateTestReward("Berserker Mode", "Low health increases damage by 100%", ERewardCategory::Offense))
				TestRewards.Add(Reward3);
			break;
		}
			
		case ERoomType::MedicalBay:
		{
			if (URewardDataAsset* Reward1 = CreateTestReward("Second Life", "Revive once per run at 50% health", ERewardCategory::PassiveAbility))
				TestRewards.Add(Reward1);
			if (URewardDataAsset* Reward2 = CreateTestReward("Regeneration", "Slowly heal 2 HP per second", ERewardCategory::PassiveAbility))
				TestRewards.Add(Reward2);
			if (URewardDataAsset* Reward3 = CreateTestReward("Stimpack", "Start each room with 25% extra health", ERewardCategory::PassiveAbility))
				TestRewards.Add(Reward3);
			break;
		}
			
		case ERoomType::CargoHold:
		{
			if (URewardDataAsset* Reward1 = CreateTestReward("Armor Plating", "Reduce all damage taken by 25%", ERewardCategory::Defense))
				TestRewards.Add(Reward1);
			if (URewardDataAsset* Reward2 = CreateTestReward("Perfect Block", "Blocking reduces damage by 75%", ERewardCategory::Defense))
				TestRewards.Add(Reward2);
			if (URewardDataAsset* Reward3 = CreateTestReward("Shield Generator", "Gain 50 shield points", ERewardCategory::Defense))
				TestRewards.Add(Reward3);
			break;
		}
			
		case ERoomType::Bridge:
		{
			if (URewardDataAsset* Reward1 = CreateTestReward("Tactical Override", "Hack terminals from double range", ERewardCategory::Interactable))
				TestRewards.Add(Reward1);
			if (URewardDataAsset* Reward2 = CreateTestReward("Emergency Systems", "Use vents every 3 seconds", ERewardCategory::Interactable))
				TestRewards.Add(Reward2);
			if (URewardDataAsset* Reward3 = CreateTestReward("System Admin", "Interact with all room systems", ERewardCategory::Interactable))
				TestRewards.Add(Reward3);
			break;
		}
	}
	
	// Check if we have any rewards
	if (TestRewards.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create any test rewards for room type: %s"), *UEnum::GetValueAsString(RoomType));
		return TestRewards;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Created %d test rewards for room"), TestRewards.Num());
	
	// Shuffle and return requested count
	if (TestRewards.Num() > Count)
	{
		// Randomly select Count rewards
		TArray<URewardDataAsset*> SelectedRewards;
		TArray<int32> Indices;
		for (int32 i = 0; i < TestRewards.Num(); i++)
		{
			Indices.Add(i);
		}
		
		for (int32 i = 0; i < Count && Indices.Num() > 0; i++)
		{
			int32 RandomIndex = FMath::RandRange(0, Indices.Num() - 1);
			SelectedRewards.Add(TestRewards[Indices[RandomIndex]]);
			Indices.RemoveAt(RandomIndex);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Selected %d rewards from pool of %d"), SelectedRewards.Num(), TestRewards.Num());
		return SelectedRewards;
	}
	
	return TestRewards;
}

URewardDataAsset* URunManagerComponent::CreateTestReward(const FString& Name, const FString& Description, ERewardCategory Category)
{
	URewardDataAsset* TestReward = NewObject<URewardDataAsset>(this);
	
	if (!TestReward)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create test reward object!"));
		return nullptr;
	}
	
	TestReward->RewardName = FText::FromString(Name);
	TestReward->Description = FText::FromString(Description);
	TestReward->Category = Category;
	
	// Don't try to request gameplay tags that don't exist - just leave it empty
	// TestReward->RewardTag will remain as an empty tag which is safe
	
	TestReward->SlotCost = 1;
	TestReward->MaxStackLevel = 3;
	TestReward->bUnlockedByDefault = true;
	
	// Set basic stat modifiers based on category
	switch (Category)
	{
		case ERewardCategory::PassiveStats:
			if (Name.Contains("Speed"))
			{
				TestReward->StatModifiers.Add("MoveSpeed", 1.25f);
			}
			else if (Name.Contains("Attack"))
			{
				TestReward->StatModifiers.Add("AttackSpeed", 1.2f);
			}
			break;
			
		case ERewardCategory::Offense:
			if (Name.Contains("Power") || Name.Contains("Damage"))
			{
				TestReward->StatModifiers.Add("DamageMultiplier", 1.5f);
			}
			break;
			
		case ERewardCategory::Defense:
			if (Name.Contains("Armor") || Name.Contains("Reduce"))
			{
				TestReward->StatModifiers.Add("DamageReduction", 0.25f);
			}
			else if (Name.Contains("Block"))
			{
				TestReward->StatModifiers.Add("BlockEffectiveness", 0.75f);
			}
			break;
			
		case ERewardCategory::PassiveAbility:
			// Don't request non-existent tags - just set parameters
			if (Name.Contains("Second Life"))
			{
				TestReward->PassiveParameters.Add("ReviveHealth", 0.5f);
			}
			else if (Name.Contains("Regeneration"))
			{
				TestReward->PassiveParameters.Add("RegenPerSecond", 2.0f);
			}
			break;
			
		case ERewardCategory::Interactable:
			// Don't request non-existent tags - just set the range and cooldown
			TestReward->InteractionRange = 1000.0f;
			TestReward->InteractableCooldown = 3.0f;
			break;
	}
	
	return TestReward;
}

void URunManagerComponent::SelectReward(int32 RewardIndex)
{
	if (!bRewardSelectionActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("No active reward selection"));
		return;
	}
	
	if (RewardIndex < 0 || RewardIndex > 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid reward index: %d (must be 0 or 1)"), RewardIndex);
		return;
	}
	
	// Get the actual selected reward
	URewardDataAsset* SelectedReward = nullptr;
	if (RewardIndex >= 0 && RewardIndex < CurrentRewardChoices.Num())
	{
		SelectedReward = CurrentRewardChoices[RewardIndex];
	}
	
	FString SelectedRewardName = SelectedReward ? SelectedReward->RewardName.ToString() : TEXT("Unknown Reward");
	FString SelectedRewardEffect = SelectedReward ? SelectedReward->Description.ToString() : TEXT("No effect");
	
	// Log the selection
	UE_LOG(LogTemp, Warning, TEXT(""));
	UE_LOG(LogTemp, Warning, TEXT(">>> REWARD SELECTED: %s <<<"), *SelectedRewardName);
	UE_LOG(LogTemp, Warning, TEXT("    Effect: %s"), *SelectedRewardEffect);
	UE_LOG(LogTemp, Warning, TEXT(""));
	
	// Clear on-screen messages
	if (GEngine)
	{
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
			FString::Printf(TEXT("Reward Selected: %s"), *SelectedRewardName));
	}
	
	// Close ONLY the reward selection widget, not the slot manager
	if (RewardSelectionWidget.IsValid())
	{
		// Remove from viewport
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(RewardSelectionWidget.ToSharedRef());
		}
		
		// Reset widget reference
		RewardSelectionWidget.Reset();
		
		UE_LOG(LogTemp, Log, TEXT("Closed reward selection widget"));
	}
	
	// Now create and show the inventory widget for slot selection
	if (SelectedReward)
	{
		AGameCharacterBase* PlayerChar = Cast<AGameCharacterBase>(GetWorld()->GetFirstPlayerController()->GetPawn());
		if (PlayerChar)
		{
			if (USlotManagerComponent* SlotManager = PlayerChar->GetSlotManagerComponent())
			{
				// Create the inventory widget
				InventoryWidget = SNew(SInventoryWidget)
					.SlotManager(SlotManager)
					.SelectedReward(SelectedReward)
					.OnRewardEquipped_Lambda([this]()
					{
						// When a reward is equipped, close inventory and complete
						UE_LOG(LogTemp, Log, TEXT("Reward equipped to slot"));
						CloseInventoryWidget();
						
						// Refresh the compact slot manager display
						if (SlotManagerWidget.IsValid())
						{
							SlotManagerWidget->RefreshSlots();
						}
						
						CompleteRewardSelection();
					})
					.OnBackToRewardSelection_Lambda([this]()
					{
						// Go back to reward selection
						UE_LOG(LogTemp, Log, TEXT("Back to reward selection"));
						CloseInventoryWidget();
						BackToRewardSelection();
					})
					.OnCancelEquip_Lambda([this]()
					{
						// Cancel equipment and continue
						UE_LOG(LogTemp, Log, TEXT("Equipment cancelled"));
						CloseInventoryWidget();
						CompleteRewardSelection();
					});
				
				// Add to viewport
				if (GEngine && GEngine->GameViewport && InventoryWidget.IsValid())
				{
					GEngine->GameViewport->AddViewportWidgetContent(InventoryWidget.ToSharedRef(), 100);
					UE_LOG(LogTemp, Log, TEXT("Opening inventory widget with reward: %s"), *SelectedRewardName);
				}
				
				// Set input mode to UI for slot selection
				if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
				{
					FInputModeUIOnly InputMode;
					PC->SetInputMode(InputMode);
					PC->bShowMouseCursor = true;
				}
			}
		}
	}
	else
	{
		// If no reward selected, just complete
		UE_LOG(LogTemp, Warning, TEXT("No reward selected, completing reward selection"));
		CompleteRewardSelection();
	}
	
	// Clear timers
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	
	// Complete the room and transition after a short delay
	FTimerHandle TransitionTimer;
	GetWorld()->GetTimerManager().SetTimer(TransitionTimer, [this]()
	{
		CompleteCurrentRoom();
		if (!IsRunComplete())
		{
			TransitionToNextRoom();
		}
	}, 1.0f, false);
}

void URunManagerComponent::CancelRewardSelection()
{
	if (!bRewardSelectionActive)
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Reward selection cancelled"));
	
	// Clear on-screen messages
	if (GEngine)
	{
		GEngine->ClearOnScreenDebugMessages();
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Reward Selection Cancelled"));
	}
	
	// Complete the reward selection process
	CompleteRewardSelection();
}

void URunManagerComponent::CompleteRewardSelection()
{
	// Close any UI
	CloseRewardSelectionUI();
	CloseInventoryWidget();
	
	// Restore input mode
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
	
	// Unpause the game
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	
	// Clean up reward selection state
	bRewardSelectionActive = false;
	CurrentRewardChoices.Empty();
	
	// Clear timers
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	
	// Complete the room and transition to next
	FTimerHandle TransitionTimer;
	GetWorld()->GetTimerManager().SetTimer(TransitionTimer, [this]()
	{
		CompleteCurrentRoom();
		if (!IsRunComplete())
		{
			TransitionToNextRoom();
		}
	}, 1.0f, false);
}

void URunManagerComponent::BackToRewardSelection()
{
	// Close inventory widget if open
	CloseInventoryWidget();
	
	// Restore game input
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
	
	// Unpause temporarily
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	
	// Re-show reward selection
	if (CurrentRewardChoices.Num() > 0)
	{
		CreateRewardSelectionUI();
	}
	else
	{
		// If no rewards available, just complete
		CompleteRewardSelection();
	}
}

// ========================================
// RUN MAP DISPLAY
// ========================================

void URunManagerComponent::DisplayRunMap()
{
	UE_LOG(LogTemp, Warning, TEXT(""));
	UE_LOG(LogTemp, Warning, TEXT("╔════════════════════════════════════════════════════════════╗"));
	UE_LOG(LogTemp, Warning, TEXT("║                    ATLAS RUN MAP                           ║"));
	UE_LOG(LogTemp, Warning, TEXT("╚════════════════════════════════════════════════════════════╝"));
	UE_LOG(LogTemp, Warning, TEXT(""));
	
	// Run progression now shown in Slate UI widget
	
	// Show the complete path of 5 rooms
	if (RandomizedRoomOrder.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Your journey through the station:"));
		UE_LOG(LogTemp, Warning, TEXT(""));
		
		for (int32 i = 0; i < RandomizedRoomOrder.Num() && i < 5; i++)
		{
			if (RandomizedRoomOrder[i])
			{
				FString RoomName = GetRoomDisplayName(RandomizedRoomOrder[i]->RoomTypeForTesting);
				FString RoomInfo;
				FColor DisplayColor;
				
				// Determine room status
				if (i < CurrentLevel - 1)
				{
					// Completed room
					RoomInfo = FString::Printf(TEXT("  [✓] Room %d: %s (COMPLETED)"), i + 1, *RoomName);
					DisplayColor = FColor::Green;
				}
				else if (i == CurrentLevel - 1)
				{
					// Current room
					RoomInfo = FString::Printf(TEXT("  [►] Room %d: %s (CURRENT)"), i + 1, *RoomName);
					DisplayColor = FColor::Yellow;
				}
				else
				{
					// Upcoming room
					RoomInfo = FString::Printf(TEXT("  [ ] Room %d: %s"), i + 1, *RoomName);
					DisplayColor = FColor::Silver;
				}
				
				UE_LOG(LogTemp, Warning, TEXT("%s"), *RoomInfo);
				
				// Room info now shown in Slate UI widget
				
				// Add arrow between rooms (except after last room)
				if (i < RandomizedRoomOrder.Num() - 1 && i < 4)
				{
					UE_LOG(LogTemp, Warning, TEXT("           ↓"));
				}
			}
		}
		
		UE_LOG(LogTemp, Warning, TEXT(""));
		UE_LOG(LogTemp, Warning, TEXT("Defeat all enemies to earn rewards and progress!"));
		UE_LOG(LogTemp, Warning, TEXT("════════════════════════════════════════════════════════"));
		
		// Add legend on screen
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(20, 15.0f, FColor::White, TEXT(""));
			GEngine->AddOnScreenDebugMessage(21, 15.0f, FColor::White, TEXT("[✓] Completed  [►] Current  [ ] Upcoming"));
		}
	}
	else if (RemainingRooms.Num() > 0)
	{
		// Use data assets if room actors aren't available
		UE_LOG(LogTemp, Warning, TEXT("Your journey through the station:"));
		UE_LOG(LogTemp, Warning, TEXT(""));
		
		for (int32 i = 0; i < RemainingRooms.Num() && i < 5; i++)
		{
			if (RemainingRooms[i])
			{
				FString RoomName = RemainingRooms[i]->RoomName.ToString();
				FString RoomInfo;
				FColor DisplayColor;
				
				// Determine room status
				if (i < CurrentLevel - 1)
				{
					// Completed room
					RoomInfo = FString::Printf(TEXT("  [✓] Room %d: %s (COMPLETED)"), i + 1, *RoomName);
					DisplayColor = FColor::Green;
				}
				else if (i == CurrentLevel - 1)
				{
					// Current room
					RoomInfo = FString::Printf(TEXT("  [►] Room %d: %s (CURRENT)"), i + 1, *RoomName);
					DisplayColor = FColor::Yellow;
				}
				else
				{
					// Upcoming room
					RoomInfo = FString::Printf(TEXT("  [ ] Room %d: %s"), i + 1, *RoomName);
					DisplayColor = FColor::Silver;
				}
				
				UE_LOG(LogTemp, Warning, TEXT("%s"), *RoomInfo);
				
				// Room info now shown in Slate UI widget
				
				// Add arrow between rooms
				if (i < RemainingRooms.Num() - 1 && i < 4)
				{
					UE_LOG(LogTemp, Warning, TEXT("           ↓"));
				}
			}
		}
		
		UE_LOG(LogTemp, Warning, TEXT(""));
		UE_LOG(LogTemp, Warning, TEXT("Defeat all enemies to earn rewards and progress!"));
		UE_LOG(LogTemp, Warning, TEXT("════════════════════════════════════════════════════════"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No rooms available for this run"));
	}
}

FString URunManagerComponent::GetRoomDisplayName(ERoomType RoomType) const
{
	switch (RoomType)
	{
		case ERoomType::EngineeringBay:
			return TEXT("Engineering Bay");
		case ERoomType::CombatArena:
			return TEXT("Combat Arena");
		case ERoomType::MedicalBay:
			return TEXT("Medical Bay");
		case ERoomType::CargoHold:
			return TEXT("Cargo Hold");
		case ERoomType::Bridge:
			return TEXT("Bridge");
		default:
			return TEXT("Unknown Room");
	}
}

void URunManagerComponent::ShowMap()
{
	DisplayRunMap();
}

void URunManagerComponent::CreateRewardSelectionUI()
{
	UE_LOG(LogTemp, Warning, TEXT(""));
	UE_LOG(LogTemp, Warning, TEXT("=== CreateRewardSelectionUI called ==="));
	
	// Make sure we have rewards to display
	if (CurrentRewardChoices.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: No reward choices available for UI!"));
		UE_LOG(LogTemp, Error, TEXT("Creating test rewards as fallback..."));
		
		// Create fallback test rewards
		CurrentRewardChoices = CreateTestRewardsForRoom(2);
		
		if (CurrentRewardChoices.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create even test rewards!"));
			return;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Creating Slate UI with %d rewards:"), CurrentRewardChoices.Num());
	for (int32 i = 0; i < CurrentRewardChoices.Num(); i++)
	{
		if (CurrentRewardChoices[i])
		{
			UE_LOG(LogTemp, Warning, TEXT("  [%d] %s"), i, *CurrentRewardChoices[i]->RewardName.ToString());
		}
	}
	
	// Close existing widget if any
	CloseRewardSelectionUI();
	
	// Don't transition to inventory mode here - that happens after selecting a reward
	// Just log that rewards are available
	UE_LOG(LogTemp, Warning, TEXT("Reward selection UI ready with %d rewards"), CurrentRewardChoices.Num());
	
	// For now, let's add a simple fallback that lets you select via console commands
	UE_LOG(LogTemp, Warning, TEXT("=== REWARD SELECTION ACTIVE ==="));
	UE_LOG(LogTemp, Warning, TEXT("Drag rewards to equipment slots or use number keys 1-5"));
	UE_LOG(LogTemp, Warning, TEXT("Console commands also available:"));
	UE_LOG(LogTemp, Warning, TEXT("  Atlas.SelectReward 0  (for first reward)"));
	UE_LOG(LogTemp, Warning, TEXT("  Atlas.SelectReward 1  (for second reward)"));
	UE_LOG(LogTemp, Warning, TEXT("  Atlas.CancelReward   (to skip reward)"));
	UE_LOG(LogTemp, Warning, TEXT("================================="));
	
	// Create the Slate widget
	UE_LOG(LogTemp, Warning, TEXT("Creating SRewardSelectionWidget..."));
	RewardSelectionWidget = SNew(SRewardSelectionWidget)
		.RewardChoices(CurrentRewardChoices)
		.RunManager(this)
		.OnRewardSelected_Lambda([this]()
		{
			UE_LOG(LogTemp, Warning, TEXT("Slate UI: Reward selected callback"));
			// Reward was selected, close the UI
			CloseRewardSelectionUI();
		})
		.OnSelectionCancelled_Lambda([this]()
		{
			UE_LOG(LogTemp, Warning, TEXT("Slate UI: Selection cancelled callback"));
			// Selection was cancelled, close the UI
			CloseRewardSelectionUI();
		});
	
	// Check if widget was created successfully
	if (!RewardSelectionWidget.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create SRewardSelectionWidget!"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("SRewardSelectionWidget created successfully"));
	
	// Add to viewport
	if (GEngine)
	{
		UE_LOG(LogTemp, Warning, TEXT("GEngine exists"));
		
		if (GEngine->GameViewport)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameViewport exists - adding widget"));
			GEngine->GameViewport->AddViewportWidgetContent(RewardSelectionWidget.ToSharedRef(), 1000);
			
			// Set input mode to UI only
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				UE_LOG(LogTemp, Warning, TEXT("Setting input mode to UI only"));
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(RewardSelectionWidget);
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("No player controller found!"));
			}
			
			// Pause the game
			UGameplayStatics::SetGamePaused(GetWorld(), true);
			
			UE_LOG(LogTemp, Warning, TEXT("✓ Successfully created Slate reward selection UI"));
			UE_LOG(LogTemp, Warning, TEXT("✓ Game paused - waiting for player input"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GameViewport is NULL!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GEngine is NULL!"));
		UE_LOG(LogTemp, Error, TEXT("Failed to create reward selection UI - no game engine"));
		UE_LOG(LogTemp, Error, TEXT("Use console commands instead to select rewards"));
	}
}

void URunManagerComponent::CreateSimpleRewardSelectionUI()
{
	UE_LOG(LogTemp, Warning, TEXT("Creating simple pure Slate reward UI"));
	
	// Close any existing widget
	CloseRewardSelectionUI();
	
	// Create a simple Slate widget with hardcoded rewards
	RewardSelectionWidget = 
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			// Semi-transparent background overlay
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0, 0, 0, 0.8f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				// Main reward panel
				SNew(SBox)
				.WidthOverride(600.0f)
				.HeightOverride(300.0f)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.95f))
					[
						SNew(SVerticalBox)
						
						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(20.0f, 20.0f, 20.0f, 10.0f)
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString("Choose Your Reward"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
							.ColorAndOpacity(FLinearColor::White)
						]
						
						// Rewards
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.Padding(20.0f, 10.0f)
						[
							SNew(SHorizontalBox)
							
							// Reward 1
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(10.0f, 0.0f)
							[
								SNew(SButton)
								.OnClicked_Lambda([this]() -> FReply
								{
									UE_LOG(LogTemp, Warning, TEXT("Player selected Sharp Blade via Slate UI"));
									SelectReward(0);
									return FReply::Handled();
								})
								.ContentPadding(FMargin(15.0f, 10.0f))
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.HAlign(HAlign_Center)
									[
										SNew(STextBlock)
										.Text(FText::FromString("Sharp Blade"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
										.ColorAndOpacity(FLinearColor::White)
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.HAlign(HAlign_Center)
									.Padding(0.0f, 5.0f, 0.0f, 0.0f)
									[
										SNew(STextBlock)
										.Text(FText::FromString("+25%% Attack Damage"))
										.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
										.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
										.AutoWrapText(true)
									]
								]
							]
							
							// Reward 2
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(10.0f, 0.0f)
							[
								SNew(SButton)
								.OnClicked_Lambda([this]() -> FReply
								{
									UE_LOG(LogTemp, Warning, TEXT("Player selected Iron Skin via Slate UI"));
									SelectReward(1);
									return FReply::Handled();
								})
								.ContentPadding(FMargin(15.0f, 10.0f))
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.HAlign(HAlign_Center)
									[
										SNew(STextBlock)
										.Text(FText::FromString("Iron Skin"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
										.ColorAndOpacity(FLinearColor::White)
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									.HAlign(HAlign_Center)
									.Padding(0.0f, 5.0f, 0.0f, 0.0f)
									[
										SNew(STextBlock)
										.Text(FText::FromString("-15%% Damage Taken"))
										.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
										.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
										.AutoWrapText(true)
									]
								]
							]
						]
						
						// Cancel button
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(20.0f, 10.0f, 20.0f, 20.0f)
						.HAlign(HAlign_Center)
						[
							SNew(SButton)
							.Text(FText::FromString("Skip Reward"))
							.OnClicked_Lambda([this]() -> FReply
							{
								UE_LOG(LogTemp, Warning, TEXT("Player cancelled reward selection via Slate UI"));
								CancelRewardSelection();
								return FReply::Handled();
							})
						]
					]
				]
			]
		];
	
	// Add to viewport
	if (GEngine && GEngine->GameViewport && RewardSelectionWidget.IsValid())
	{
		GEngine->GameViewport->AddViewportWidgetContent(RewardSelectionWidget.ToSharedRef(), 1000);
		
		// Set input mode to UI
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(RewardSelectionWidget);
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
		
		// Pause the game
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		
		UE_LOG(LogTemp, Warning, TEXT("Successfully created simple Slate reward UI"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create simple Slate UI"));
	}
}

void URunManagerComponent::CloseRewardSelectionUI()
{
	if (RewardSelectionWidget.IsValid())
	{
		// Remove from viewport
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(RewardSelectionWidget.ToSharedRef());
		}
		
		// Reset widget reference
		RewardSelectionWidget.Reset();
		
		// Restore input mode
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
		
		// Unpause the game
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		
		UE_LOG(LogTemp, Log, TEXT("Closed Slate reward selection UI"));
	}
}

void URunManagerComponent::CloseInventoryWidget()
{
	if (InventoryWidget.IsValid())
	{
		// Remove from viewport
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(InventoryWidget.ToSharedRef());
		}
		
		// Reset widget reference
		InventoryWidget.Reset();
		
		UE_LOG(LogTemp, Log, TEXT("Closed inventory widget"));
	}
}

void URunManagerComponent::UpdateEnemyHealthDisplay(float CurrentHealth, float MaxHealth, float CurrentPoise, float MaxPoise)
{
	if (EnemyHealthWidget.IsValid())
	{
		EnemyHealthWidget->UpdateEnemyHealth(CurrentHealth, MaxHealth);
		EnemyHealthWidget->UpdateEnemyPoise(CurrentPoise, MaxPoise);
		
		// Log updates periodically
		static int32 UpdateCounter = 0;
		if (++UpdateCounter % 30 == 0)  // Log every 30 updates
		{
			UE_LOG(LogTemp, Warning, TEXT("UpdateEnemyHealthDisplay: Health %.0f/%.0f, Poise %.0f/%.0f"),
				CurrentHealth, MaxHealth, CurrentPoise, MaxPoise);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UpdateEnemyHealthDisplay: EnemyHealthWidget is not valid!"));
	}
}

void URunManagerComponent::ShowEnemyHealthWidget(AGameCharacterBase* Enemy)
{
	if (!Enemy)
	{
		HideEnemyHealthWidget();
		return;
	}
	
	// Store the current enemy reference
	CurrentRoomEnemy = Enemy;
	
	// Create enemy health widget if it doesn't exist
	if (!EnemyHealthWidget.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Creating new enemy health widget"));
		
		// Determine enemy name with better fallback logic
		FText EnemyName;
		if (CurrentRoom)
		{
			UE_LOG(LogTemp, Warning, TEXT("CurrentRoom exists: %s"), *CurrentRoom->RoomName.ToString());
			
			// Check if EnemyName field is populated
			if (!CurrentRoom->EnemyName.IsEmpty())
			{
				EnemyName = CurrentRoom->EnemyName;
				UE_LOG(LogTemp, Warning, TEXT("Using DataAsset EnemyName: %s"), *EnemyName.ToString());
			}
			else
			{
				// EnemyName is empty, generate based on room type
				const UEnum* RoomEnum = StaticEnum<ERoomType>();
				FString RoomTypeName = RoomEnum ? RoomEnum->GetNameStringByValue((int64)CurrentRoom->RoomType) : TEXT("Unknown");
				EnemyName = FText::FromString(FString::Printf(TEXT("%s Enemy"), *RoomTypeName));
				UE_LOG(LogTemp, Warning, TEXT("DataAsset EnemyName is empty, generated: %s"), *EnemyName.ToString());
			}
		}
		else
		{
			// No CurrentRoom, use generic fallback
			EnemyName = FText::FromString("Enemy");
			UE_LOG(LogTemp, Warning, TEXT("No CurrentRoom, using generic enemy name"));
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Final enemy name: %s"), *EnemyName.ToString());
		
		UHealthComponent* EnemyHealthComp = Enemy->GetHealthComponent();
		if (!EnemyHealthComp)
		{
			UE_LOG(LogTemp, Error, TEXT("Enemy has no health component! Cannot create widget."));
			return;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Creating SEnemyHealthWidget..."));
		EnemyHealthWidget = SNew(SEnemyHealthWidget)
			.EnemyHealthComponent(EnemyHealthComp)
			.EnemyName(EnemyName);
		
		// Add to viewport at top-center
		if (GEngine && GEngine->GameViewport && EnemyHealthWidget.IsValid())
		{
			GEngine->GameViewport->AddViewportWidgetContent(EnemyHealthWidget.ToSharedRef(), 15);
			
			// Subscribe to health changes
			if (!EnemyHealthComp->OnHealthChanged.IsAlreadyBound(this, &URunManagerComponent::OnEnemyHealthChanged))
			{
				EnemyHealthComp->OnHealthChanged.AddDynamic(this, &URunManagerComponent::OnEnemyHealthChanged);
			}
			if (!EnemyHealthComp->OnPoiseChanged.IsAlreadyBound(this, &URunManagerComponent::OnEnemyPoiseChanged))
			{
				EnemyHealthComp->OnPoiseChanged.AddDynamic(this, &URunManagerComponent::OnEnemyPoiseChanged);
			}
			// Subscribe to death event
			if (!EnemyHealthComp->OnDeath.IsAlreadyBound(this, &URunManagerComponent::OnEnemyDefeated))
			{
				EnemyHealthComp->OnDeath.AddDynamic(this, &URunManagerComponent::OnEnemyDefeated);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to add enemy health widget to viewport!"));
			if (!GEngine) UE_LOG(LogTemp, Error, TEXT("GEngine is null"));
			if (!GEngine->GameViewport) UE_LOG(LogTemp, Error, TEXT("GameViewport is null"));
			if (!EnemyHealthWidget.IsValid()) UE_LOG(LogTemp, Error, TEXT("EnemyHealthWidget is invalid after creation"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy health widget already exists, updating it"));
		
		// Update existing widget with new enemy
		if (UHealthComponent* HealthComp = Enemy->GetHealthComponent())
		{
			FText EnemyName = CurrentRoom ? CurrentRoom->EnemyName : FText::FromString("Enemy");
			EnemyHealthWidget->SetEnemyName(EnemyName);
			EnemyHealthWidget->SetEnemyHealthComponent(HealthComp);
			
			// Subscribe to health changes
			if (!HealthComp->OnHealthChanged.IsAlreadyBound(this, &URunManagerComponent::OnEnemyHealthChanged))
			{
				HealthComp->OnHealthChanged.AddDynamic(this, &URunManagerComponent::OnEnemyHealthChanged);
			}
			if (!HealthComp->OnPoiseChanged.IsAlreadyBound(this, &URunManagerComponent::OnEnemyPoiseChanged))
			{
				HealthComp->OnPoiseChanged.AddDynamic(this, &URunManagerComponent::OnEnemyPoiseChanged);
			}
			// Subscribe to death event
			if (!HealthComp->OnDeath.IsAlreadyBound(this, &URunManagerComponent::OnEnemyDefeated))
			{
				HealthComp->OnDeath.AddDynamic(this, &URunManagerComponent::OnEnemyDefeated);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Enemy has no health component for update!"));
		}
	}
	
	// Show the widget
	if (EnemyHealthWidget.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Calling ShowWidget on EnemyHealthWidget"));
		EnemyHealthWidget->ShowWidget();
		
		// Force update with current values
		if (UHealthComponent* HealthComp = Enemy->GetHealthComponent())
		{
			UpdateEnemyHealthDisplay(
				HealthComp->GetCurrentHealth(),
				HealthComp->GetMaxHealth(),
				HealthComp->GetCurrentPoise(),
				HealthComp->GetMaxPoise());
			UE_LOG(LogTemp, Warning, TEXT("Updated enemy display - Health: %.0f/%.0f, Poise: %.0f/%.0f"),
				HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth(),
				HealthComp->GetCurrentPoise(), HealthComp->GetMaxPoise());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyHealthWidget is not valid after all attempts!"));
	}
}

void URunManagerComponent::HideEnemyHealthWidget()
{
	if (EnemyHealthWidget.IsValid())
	{
		EnemyHealthWidget->HideWidget();
		
		// Optionally remove from viewport completely
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(EnemyHealthWidget.ToSharedRef());
		}
		EnemyHealthWidget.Reset();
	}
}

void URunManagerComponent::OnPlayerHealthChanged(float CurrentHealth, float MaxHealth, float HealthDelta)
{
	if (RunProgressWidget.IsValid())
	{
		RunProgressWidget->UpdateHealth(CurrentHealth, MaxHealth);
	}
}

void URunManagerComponent::OnPlayerPoiseChanged(float CurrentPoise, float MaxPoise, float PoiseDelta)
{
	if (RunProgressWidget.IsValid())
	{
		RunProgressWidget->UpdatePoise(CurrentPoise, MaxPoise);
	}
}

void URunManagerComponent::OnPlayerIntegrityChanged(float CurrentIntegrity, float MaxIntegrity, float IntegrityDelta)
{
	if (RunProgressWidget.IsValid())
	{
		RunProgressWidget->UpdateIntegrity(CurrentIntegrity, MaxIntegrity);
	}
}

void URunManagerComponent::OnEnemyHealthChanged(float CurrentHealth, float MaxHealth, float HealthDelta)
{
	// Update the widget directly with the values from the event
	if (EnemyHealthWidget.IsValid())
	{
		EnemyHealthWidget->UpdateEnemyHealth(CurrentHealth, MaxHealth);
	}
}

void URunManagerComponent::OnEnemyPoiseChanged(float CurrentPoise, float MaxPoise, float PoiseDelta)
{
	// Update the widget directly with the values from the event
	if (EnemyHealthWidget.IsValid())
	{
		EnemyHealthWidget->UpdateEnemyPoise(CurrentPoise, MaxPoise);
	}
}