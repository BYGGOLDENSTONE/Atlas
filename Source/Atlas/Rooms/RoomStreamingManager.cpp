#include "RoomStreamingManager.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "Atlas/Rooms/RoomBase.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

URoomStreamingManager::URoomStreamingManager()
{
	bIsStreaming = false;
	CurrentTransitionDuration = 2.0f;
}

void URoomStreamingManager::Initialize(UWorld* InWorld)
{
	if (!InWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("RoomStreamingManager: Invalid world provided to Initialize"));
		return;
	}

	WorldContext = InWorld;
	LoadedStreamingLevels.Empty();
	PreloadedLevels.Empty();
	bIsStreaming = false;
	
	UE_LOG(LogTemp, Log, TEXT("RoomStreamingManager initialized for world: %s"), *WorldContext->GetName());
}

void URoomStreamingManager::Shutdown()
{
	// Clear all streaming levels
	for (auto& Pair : LoadedStreamingLevels)
	{
		if (Pair.Value)
		{
			Pair.Value->SetShouldBeLoaded(false);
			Pair.Value->SetShouldBeVisible(false);
		}
	}
	
	LoadedStreamingLevels.Empty();
	PreloadedLevels.Empty();
	
	// Destroy room actors
	if (CurrentRoomActor)
	{
		CurrentRoomActor->Destroy();
		CurrentRoomActor = nullptr;
	}
	
	if (PreviousRoomActor)
	{
		PreviousRoomActor->Destroy();
		PreviousRoomActor = nullptr;
	}
	
	// Clear timers
	if (WorldContext)
	{
		WorldContext->GetTimerManager().ClearTimer(TransitionTimerHandle);
	}
	
	WorldContext = nullptr;
	bIsStreaming = false;
	
	UE_LOG(LogTemp, Log, TEXT("RoomStreamingManager shutdown complete"));
}

bool URoomStreamingManager::LoadRoomLevel(URoomDataAsset* RoomData, bool bMakeVisible)
{
	if (!RoomData || !WorldContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadRoomLevel: Invalid room data or world context"));
		return false;
	}

	// Get the level path from room data
	FString LevelPath = GetLevelNameFromRoom(RoomData);
	if (LevelPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadRoomLevel: No level path specified for room %s"), 
			*RoomData->RoomName.ToString());
		return false;
	}

	// Check if already loaded
	if (LoadedStreamingLevels.Contains(*LevelPath))
	{
		UE_LOG(LogTemp, Log, TEXT("LoadRoomLevel: Level %s already loaded"), *LevelPath);
		if (bMakeVisible)
		{
			return ShowPreloadedRoom(RoomData);
		}
		return true;
	}

	// Create streaming level
	ULevelStreaming* StreamingLevel = GetOrCreateStreamingLevel(LevelPath);
	if (!StreamingLevel)
	{
		UE_LOG(LogTemp, Error, TEXT("LoadRoomLevel: Failed to create streaming level for %s"), *LevelPath);
		OnLevelStreamingFailed.Broadcast(*LevelPath);
		return false;
	}

	// Store the streaming level
	LoadedStreamingLevels.Add(*LevelPath, StreamingLevel);
	CurrentLoadingRoom = RoomData;
	bIsStreaming = true;

	// Set loading parameters
	StreamingLevel->SetShouldBeLoaded(true);
	StreamingLevel->SetShouldBeVisible(bMakeVisible);
	
	if (!bMakeVisible)
	{
		PreloadedLevels.AddUnique(*LevelPath);
	}

	// Bind completion delegates
	StreamingLevel->OnLevelShown.AddDynamic(this, &URoomStreamingManager::OnLevelShown);
	StreamingLevel->OnLevelHidden.AddDynamic(this, &URoomStreamingManager::OnLevelHidden);

	UE_LOG(LogTemp, Log, TEXT("LoadRoomLevel: Started loading %s (Visible: %s)"), 
		*LevelPath, bMakeVisible ? TEXT("Yes") : TEXT("No"));

	return true;
}

bool URoomStreamingManager::UnloadRoomLevel(URoomDataAsset* RoomData)
{
	if (!RoomData || !WorldContext)
	{
		return false;
	}

	FString LevelPath = GetLevelNameFromRoom(RoomData);
	ULevelStreaming** StreamingLevelPtr = LoadedStreamingLevels.Find(*LevelPath);
	
	if (!StreamingLevelPtr || !(*StreamingLevelPtr))
	{
		UE_LOG(LogTemp, Warning, TEXT("UnloadRoomLevel: Level %s not found"), *LevelPath);
		return false;
	}

	ULevelStreaming* StreamingLevel = *StreamingLevelPtr;
	bIsStreaming = true;

	// Hide and unload the level
	StreamingLevel->SetShouldBeVisible(false);
	StreamingLevel->SetShouldBeLoaded(false);
	
	// Remove from loaded levels
	LoadedStreamingLevels.Remove(*LevelPath);
	PreloadedLevels.Remove(*LevelPath);

	UE_LOG(LogTemp, Log, TEXT("UnloadRoomLevel: Started unloading %s"), *LevelPath);
	
	// Fire unload complete event after a delay (async operation)
	FTimerHandle UnloadTimer;
	WorldContext->GetTimerManager().SetTimer(UnloadTimer, [this, LevelPath]()
	{
		bIsStreaming = false;
		OnLevelUnloadComplete.Broadcast(*LevelPath);
		UE_LOG(LogTemp, Log, TEXT("UnloadRoomLevel: Completed unloading %s"), *LevelPath);
	}, 0.5f, false);

	return true;
}

void URoomStreamingManager::TransitionRooms(URoomDataAsset* FromRoom, URoomDataAsset* ToRoom, float TransitionDuration)
{
	if (!ToRoom || !WorldContext)
	{
		UE_LOG(LogTemp, Error, TEXT("TransitionRooms: Invalid parameters"));
		return;
	}

	CurrentTransitionDuration = TransitionDuration;
	PendingTransitionRoom = ToRoom;
	
	// Store previous room actor
	if (CurrentRoomActor)
	{
		PreviousRoomActor = CurrentRoomActor;
		CurrentRoomActor = nullptr;
		
		// Deactivate previous room
		if (PreviousRoomActor)
		{
			PreviousRoomActor->DeactivateRoom();
		}
	}

	// Start transition effect (fade out)
	APlayerController* PC = WorldContext->GetFirstPlayerController();
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, TransitionDuration * 0.4f, FLinearColor::Black, false, true);
	}

	// Unload previous room after fade
	if (FromRoom)
	{
		FTimerHandle UnloadTimer;
		WorldContext->GetTimerManager().SetTimer(UnloadTimer, [this, FromRoom]()
		{
			UnloadRoomLevel(FromRoom);
			
			// Destroy previous room actor
			if (PreviousRoomActor)
			{
				PreviousRoomActor->Destroy();
				PreviousRoomActor = nullptr;
			}
		}, TransitionDuration * 0.4f, false);
	}

	// Load new room at midpoint
	FTimerHandle LoadTimer;
	WorldContext->GetTimerManager().SetTimer(LoadTimer, [this, ToRoom]()
	{
		LoadRoomLevel(ToRoom, true);
	}, TransitionDuration * 0.5f, false);

	// Complete transition (fade in)
	WorldContext->GetTimerManager().SetTimer(TransitionTimerHandle, [this]()
	{
		CompleteRoomTransition();
	}, TransitionDuration, false);
}

bool URoomStreamingManager::PreloadRoomLevel(URoomDataAsset* RoomData)
{
	return LoadRoomLevel(RoomData, false);
}

bool URoomStreamingManager::ShowPreloadedRoom(URoomDataAsset* RoomData)
{
	if (!RoomData || !WorldContext)
	{
		return false;
	}

	FString LevelPath = GetLevelNameFromRoom(RoomData);
	
	// Check if it's preloaded
	if (!PreloadedLevels.Contains(*LevelPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowPreloadedRoom: Level %s not preloaded"), *LevelPath);
		return false;
	}

	ULevelStreaming** StreamingLevelPtr = LoadedStreamingLevels.Find(*LevelPath);
	if (!StreamingLevelPtr || !(*StreamingLevelPtr))
	{
		return false;
	}

	ULevelStreaming* StreamingLevel = *StreamingLevelPtr;
	StreamingLevel->SetShouldBeVisible(true);
	
	PreloadedLevels.Remove(*LevelPath);
	
	UE_LOG(LogTemp, Log, TEXT("ShowPreloadedRoom: Made %s visible"), *LevelPath);
	return true;
}

ARoomBase* URoomStreamingManager::SpawnRoomActor(URoomDataAsset* RoomData, TSubclassOf<ARoomBase> RoomClass)
{
	if (!RoomData || !RoomClass || !WorldContext)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnRoomActor: Invalid parameters"));
		return nullptr;
	}

	// Clear any existing room actor
	if (CurrentRoomActor)
	{
		CurrentRoomActor->Destroy();
		CurrentRoomActor = nullptr;
	}

	// Spawn the new room actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	CurrentRoomActor = WorldContext->SpawnActor<ARoomBase>(RoomClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	
	if (CurrentRoomActor)
	{
		// Activate the room with its data
		CurrentRoomActor->ActivateRoom(RoomData);
		UE_LOG(LogTemp, Log, TEXT("SpawnRoomActor: Spawned and activated room %s"), *RoomData->RoomName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnRoomActor: Failed to spawn room actor"));
	}

	return CurrentRoomActor;
}

void URoomStreamingManager::ClearCurrentRoom()
{
	if (CurrentRoomActor)
	{
		CurrentRoomActor->DeactivateRoom();
		CurrentRoomActor->Destroy();
		CurrentRoomActor = nullptr;
	}
}

bool URoomStreamingManager::IsRoomLoaded(URoomDataAsset* RoomData) const
{
	if (!RoomData)
	{
		return false;
	}

	FString LevelPath = GetLevelNameFromRoom(RoomData);
	return LoadedStreamingLevels.Contains(*LevelPath);
}

bool URoomStreamingManager::IsRoomVisible(URoomDataAsset* RoomData) const
{
	if (!RoomData)
	{
		return false;
	}

	FString LevelPath = GetLevelNameFromRoom(RoomData);
	ULevelStreaming* const* StreamingLevelPtr = LoadedStreamingLevels.Find(*LevelPath);
	
	if (!StreamingLevelPtr || !(*StreamingLevelPtr))
	{
		return false;
	}

	return (*StreamingLevelPtr)->IsLevelVisible();
}

TArray<FName> URoomStreamingManager::GetLoadedLevels() const
{
	TArray<FName> LevelNames;
	LoadedStreamingLevels.GetKeys(LevelNames);
	return LevelNames;
}

ULevelStreaming* URoomStreamingManager::GetOrCreateStreamingLevel(const FString& LevelPath)
{
	if (!WorldContext)
	{
		return nullptr;
	}

	// Try to find existing streaming level
	const TArray<ULevelStreaming*>& StreamingLevels = WorldContext->GetStreamingLevels();
	for (ULevelStreaming* Level : StreamingLevels)
	{
		if (Level && Level->GetWorldAssetPackageFName().ToString().Contains(LevelPath))
		{
			return Level;
		}
	}

	// Create new streaming level
	bool bSuccess = false;
	ULevelStreamingDynamic* NewStreamingLevel = ULevelStreamingDynamic::LoadLevelInstance(
		WorldContext, 
		LevelPath, 
		FVector::ZeroVector, 
		FRotator::ZeroRotator, 
		bSuccess
	);

	return NewStreamingLevel;
}

void URoomStreamingManager::OnLevelShown()
{
	bIsStreaming = false;
	
	if (CurrentLoadingRoom)
	{
		FString LevelPath = GetLevelNameFromRoom(CurrentLoadingRoom);
		OnLevelLoadComplete.Broadcast(*LevelPath);
		UE_LOG(LogTemp, Log, TEXT("OnLevelShown: Level %s is now visible"), *LevelPath);
	}
}

void URoomStreamingManager::OnLevelHidden()
{
	UE_LOG(LogTemp, Log, TEXT("OnLevelHidden: A level was hidden"));
}

void URoomStreamingManager::CompleteRoomTransition()
{
	// Fade back in
	APlayerController* PC = WorldContext->GetFirstPlayerController();
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, CurrentTransitionDuration * 0.3f, FLinearColor::Black, false, false);
	}

	PendingTransitionRoom = nullptr;
	bIsStreaming = false;
	
	UE_LOG(LogTemp, Log, TEXT("CompleteRoomTransition: Transition complete"));
}

ULevelStreaming* URoomStreamingManager::FindStreamingLevel(URoomDataAsset* RoomData) const
{
	if (!RoomData)
	{
		return nullptr;
	}

	FString LevelPath = GetLevelNameFromRoom(RoomData);
	ULevelStreaming* const* StreamingLevelPtr = LoadedStreamingLevels.Find(*LevelPath);
	
	return StreamingLevelPtr ? *StreamingLevelPtr : nullptr;
}

FString URoomStreamingManager::GetLevelNameFromRoom(URoomDataAsset* RoomData) const
{
	if (!RoomData || !RoomData->RoomLevel.IsValid())
	{
		return FString();
	}

	return RoomData->RoomLevel.GetLongPackageName();
}