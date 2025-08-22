#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/LevelStreaming.h"
#include "RoomStreamingManager.generated.h"

// Forward declarations
class URoomDataAsset;
class ARoomBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelStreamingComplete, FName, LevelName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelStreamingFailed, FName, LevelName);

/**
 * Manages level streaming for room transitions in Atlas.
 * Handles async loading/unloading of room levels.
 */
UCLASS(BlueprintType)
class ATLAS_API URoomStreamingManager : public UObject
{
	GENERATED_BODY()

public:
	URoomStreamingManager();

	// ========================================
	// INITIALIZATION
	// ========================================
	
	/**
	 * Initialize the streaming manager with world context
	 * @param InWorld The world to manage streaming for
	 */
	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	void Initialize(UWorld* InWorld);
	
	/**
	 * Cleanup and reset the streaming manager
	 */
	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	void Shutdown();

	// ========================================
	// LEVEL STREAMING
	// ========================================
	
	/**
	 * Load a room level asynchronously
	 * @param RoomData The room data containing level reference
	 * @param bMakeVisible Whether to make the level visible immediately
	 * @return True if streaming was initiated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	bool LoadRoomLevel(URoomDataAsset* RoomData, bool bMakeVisible = true);
	
	/**
	 * Unload a room level asynchronously
	 * @param RoomData The room data containing level reference
	 * @return True if unloading was initiated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	bool UnloadRoomLevel(URoomDataAsset* RoomData);
	
	/**
	 * Transition between two room levels
	 * @param FromRoom The room to unload (can be null)
	 * @param ToRoom The room to load
	 * @param TransitionDuration Time for the transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	void TransitionRooms(URoomDataAsset* FromRoom, URoomDataAsset* ToRoom, float TransitionDuration = 2.0f);
	
	/**
	 * Preload a room level without making it visible
	 * @param RoomData The room to preload
	 * @return True if preloading was initiated
	 */
	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	bool PreloadRoomLevel(URoomDataAsset* RoomData);
	
	/**
	 * Make a preloaded room visible
	 * @param RoomData The room to show
	 * @return True if the room was made visible
	 */
	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	bool ShowPreloadedRoom(URoomDataAsset* RoomData);

	// ========================================
	// ROOM MANAGEMENT
	// ========================================
	
	/**
	 * Spawn a room actor in the loaded level
	 * @param RoomData The room configuration
	 * @param RoomClass The room actor class to spawn
	 * @return The spawned room actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	ARoomBase* SpawnRoomActor(URoomDataAsset* RoomData, TSubclassOf<ARoomBase> RoomClass);
	
	/**
	 * Get the currently loaded room actor
	 * @return Current room actor or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Room Streaming")
	ARoomBase* GetCurrentRoomActor() const { return CurrentRoomActor; }
	
	/**
	 * Clear the current room (destroy actors, reset state)
	 */
	UFUNCTION(BlueprintCallable, Category = "Room Streaming")
	void ClearCurrentRoom();

	// ========================================
	// QUERIES
	// ========================================
	
	/**
	 * Check if a room level is currently loaded
	 * @param RoomData The room to check
	 * @return True if the level is loaded
	 */
	UFUNCTION(BlueprintPure, Category = "Room Streaming|Queries")
	bool IsRoomLoaded(URoomDataAsset* RoomData) const;
	
	/**
	 * Check if a room level is currently visible
	 * @param RoomData The room to check
	 * @return True if the level is visible
	 */
	UFUNCTION(BlueprintPure, Category = "Room Streaming|Queries")
	bool IsRoomVisible(URoomDataAsset* RoomData) const;
	
	/**
	 * Check if any streaming operation is in progress
	 * @return True if streaming is active
	 */
	UFUNCTION(BlueprintPure, Category = "Room Streaming|Queries")
	bool IsStreamingInProgress() const { return bIsStreaming; }
	
	/**
	 * Get all currently loaded streaming levels
	 * @return Array of loaded level names
	 */
	UFUNCTION(BlueprintPure, Category = "Room Streaming|Queries")
	TArray<FName> GetLoadedLevels() const;

	// ========================================
	// EVENTS
	// ========================================
	
	/** Called when a level finishes streaming in */
	UPROPERTY(BlueprintAssignable, Category = "Room Streaming|Events")
	FOnLevelStreamingComplete OnLevelLoadComplete;
	
	/** Called when a level finishes streaming out */
	UPROPERTY(BlueprintAssignable, Category = "Room Streaming|Events")
	FOnLevelStreamingComplete OnLevelUnloadComplete;
	
	/** Called when level streaming fails */
	UPROPERTY(BlueprintAssignable, Category = "Room Streaming|Events")
	FOnLevelStreamingFailed OnLevelStreamingFailed;

protected:
	// ========================================
	// INTERNAL FUNCTIONS
	// ========================================
	
	/**
	 * Get or create a streaming level instance
	 * @param LevelPath The path to the level
	 * @return The streaming level instance
	 */
	ULevelStreaming* GetOrCreateStreamingLevel(const FString& LevelPath);
	
	/**
	 * Handle level shown callback
	 */
	UFUNCTION()
	void OnLevelShown();
	
	/**
	 * Handle level hidden callback
	 */
	UFUNCTION()
	void OnLevelHidden();
	
	/**
	 * Internal transition completion
	 */
	void CompleteRoomTransition();
	
	/**
	 * Find streaming level by room data
	 */
	ULevelStreaming* FindStreamingLevel(URoomDataAsset* RoomData) const;
	
	/**
	 * Get level name from room data
	 */
	FString GetLevelNameFromRoom(URoomDataAsset* RoomData) const;

protected:
	// ========================================
	// PROPERTIES
	// ========================================
	
	/** World context for streaming operations */
	UPROPERTY()
	UWorld* WorldContext;
	
	/** Currently loaded streaming levels mapped by level name */
	UPROPERTY()
	TMap<FName, ULevelStreaming*> LoadedStreamingLevels;
	
	/** Currently active room actor */
	UPROPERTY()
	ARoomBase* CurrentRoomActor;
	
	/** Previous room actor (during transitions) */
	UPROPERTY()
	ARoomBase* PreviousRoomActor;
	
	/** Whether a streaming operation is in progress */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsStreaming;
	
	/** Current room being loaded */
	UPROPERTY()
	URoomDataAsset* CurrentLoadingRoom;
	
	/** Room to transition to after current unload */
	UPROPERTY()
	URoomDataAsset* PendingTransitionRoom;
	
	/** Timer handle for transition effects */
	FTimerHandle TransitionTimerHandle;
	
	/** Transition fade duration */
	float CurrentTransitionDuration;
	
	/** Preloaded levels that are not yet visible */
	UPROPERTY()
	TArray<FName> PreloadedLevels;
};