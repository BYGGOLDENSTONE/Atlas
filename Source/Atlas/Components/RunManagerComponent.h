#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Atlas/Data/RoomDataAsset.h"  // Need full include for ERoomType
#include "RunManagerComponent.generated.h"

// Forward declarations
class AGameCharacterBase;
class USlotManagerComponent;
class ARoomBase;

/**
 * Current state of the run
 */
UENUM(BlueprintType)
enum class ERunState : uint8
{
	PreRun          UMETA(DisplayName = "Pre-Run (Main Menu)"),
	RoomIntro       UMETA(DisplayName = "Room Introduction"),
	Combat          UMETA(DisplayName = "Active Combat"),
	Victory         UMETA(DisplayName = "Enemy Defeated"),
	RewardSelection UMETA(DisplayName = "Choosing Reward"),
	RoomComplete    UMETA(DisplayName = "Ready to Transition"),
	RunComplete     UMETA(DisplayName = "All 5 Rooms Complete"),
	RunFailed       UMETA(DisplayName = "Death or Station Destroyed")
};

/**
 * Data about the current run progress
 */
USTRUCT(BlueprintType)
struct FRunProgressData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLevel = 1;

	UPROPERTY(BlueprintReadOnly)
	float PlayerHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly)
	float StationIntegrity = 100.0f;

	UPROPERTY(BlueprintReadOnly)
	TArray<FName> CompletedRoomIDs;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalEnemiesDefeated = 0;

	UPROPERTY(BlueprintReadOnly)
	float TotalDamageDealt = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 PerfectParries = 0;

	FRunProgressData() {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunStateChanged, ERunState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomCompleted, URoomDataAsset*, CompletedRoom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomStarted, URoomDataAsset*, NewRoom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRunCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunFailed, FText, FailureReason);

/**
 * Component that manages the room progression and run state for Atlas.
 * Handles the 5-room run structure, enemy spawning, and transitions.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API URunManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URunManagerComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// RUN MANAGEMENT
	// ========================================
	
	/**
	 * Initialize a new run, randomizing room order
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager")
	void InitializeRun();
	
	/**
	 * Start a new run from scratch
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager")
	void StartNewRun();
	
	/**
	 * Resume a run from saved data
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager")
	void ResumeRun(const FRunProgressData& SavedProgress);
	
	/**
	 * Complete the current room and prepare for next
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager")
	void CompleteCurrentRoom();
	
	/**
	 * Transition to the next room
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager")
	void TransitionToNextRoom();
	
	/**
	 * End the current run (success or failure)
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager")
	void EndRun(bool bSuccess, const FText& Reason = FText::GetEmpty());
	
	/**
	 * Abandon the current run and return to menu
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager")
	void AbandonRun();
	
	// ========================================
	// ROOM MANAGEMENT
	// ========================================
	
	/**
	 * Select the next room based on current level
	 * @return The selected room or nullptr if run complete
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|Rooms")
	URoomDataAsset* SelectNextRoom();
	
	/**
	 * Load a specific room
	 * @param Room The room to load
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|Rooms")
	void LoadRoom(URoomDataAsset* Room);
	
	/**
	 * Spawn the enemy for the current room
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|Rooms")
	void SpawnRoomEnemy();
	
	/**
	 * Apply environmental hazards for the current room
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|Rooms")
	void ApplyRoomHazards();
	
	/**
	 * Clear the current room (despawn enemies, effects, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|Rooms")
	void ClearCurrentRoom();
	
	// ========================================
	// STATE MANAGEMENT
	// ========================================
	
	/**
	 * Update the current run state
	 * @param NewState The new state to transition to
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|State")
	void SetRunState(ERunState NewState);
	
	/**
	 * Get the current run state
	 * @return Current run state
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager|State")
	ERunState GetRunState() const { return CurrentRunState; }
	
	/**
	 * Check if currently in a run
	 * @return True if run is active
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager|State")
	bool IsRunActive() const;
	
	/**
	 * Check if the run is complete
	 * @return True if all 5 rooms completed
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager|State")
	bool IsRunComplete() const { return CurrentLevel > 5; }
	
	// ========================================
	// QUERIES
	// ========================================
	
	/**
	 * Get the current room
	 * @return Current room data asset
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager|Queries")
	URoomDataAsset* GetCurrentRoom() const { return CurrentRoom; }
	
	/**
	 * Get the current level (1-5)
	 * @return Current level number
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager|Queries")
	int32 GetCurrentLevel() const { return CurrentLevel; }
	
	/**
	 * Get remaining rooms in the run
	 * @return Array of remaining rooms
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager|Queries")
	TArray<URoomDataAsset*> GetRemainingRooms() const { return RemainingRooms; }
	
	/**
	 * Get completed rooms
	 * @return Array of completed rooms
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager|Queries")
	TArray<URoomDataAsset*> GetCompletedRooms() const { return CompletedRooms; }
	
	/**
	 * Get current run progress data
	 * @return Run progress structure
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager|Queries")
	FRunProgressData GetRunProgress() const;
	
	/**
	 * Get the spawned enemy for the current room
	 * @return Current enemy character
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager|Queries")
	AGameCharacterBase* GetCurrentEnemy() const { return CurrentRoomEnemy; }
	
	// ========================================
	// TEST ARENA SUPPORT
	// ========================================
	
	/**
	 * Teleport to a specific room by name (for testing)
	 * @param RoomName Name of the room (EngineeringBay, CombatArena, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|Testing", Exec)
	void GoToRoom(const FString& RoomName);
	
	/**
	 * Complete current room and spawn next (for testing)
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|Testing", Exec)
	void CompleteRoomTest();
	
	/**
	 * Reset all rooms in test arena
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|Testing", Exec)
	void ResetAllRooms();
	
	/**
	 * Show debug information about rooms
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|Testing", Exec)
	void DebugRooms();
	
	/**
	 * Test complete room sequence
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|Testing", Exec)
	void TestRoomSequence();
	
	/**
	 * Get room actor by type (for test arena)
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager|Testing")
	ARoomBase* GetRoomActorByType(ERoomType RoomType) const;
	
	// ========================================
	// ENEMY SCALING
	// ========================================
	
	/**
	 * Calculate enemy power based on player slots
	 * @return Calculated enemy power level
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager|Scaling")
	int32 CalculateEnemyPower() const;
	
	/**
	 * Apply difficulty scaling to spawned enemy
	 * @param Enemy The enemy to scale
	 */
	UFUNCTION(BlueprintCallable, Category = "Run Manager|Scaling")
	void ApplyEnemyScaling(AGameCharacterBase* Enemy);
	
	// ========================================
	// EVENTS
	// ========================================
	
	UPROPERTY(BlueprintAssignable, Category = "Run Manager|Events")
	FOnRunStateChanged OnRunStateChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Run Manager|Events")
	FOnRoomCompleted OnRoomCompleted;
	
	UPROPERTY(BlueprintAssignable, Category = "Run Manager|Events")
	FOnRoomStarted OnRoomStarted;
	
	UPROPERTY(BlueprintAssignable, Category = "Run Manager|Events")
	FOnRunCompleted OnRunCompleted;
	
	UPROPERTY(BlueprintAssignable, Category = "Run Manager|Events")
	FOnRunFailed OnRunFailed;

protected:
	// ========================================
	// INTERNAL FUNCTIONS
	// ========================================
	
	/** Randomize room order for the run */
	void RandomizeRoomOrder();
	
	/** Handle room transition effects */
	void BeginRoomTransition();
	void CompleteRoomTransition();
	
	/** Load room level using level streaming */
	void LoadRoomLevel(URoomDataAsset* Room);
	void UnloadRoomLevel(URoomDataAsset* Room);
	
	/** Handle enemy defeat */
	UFUNCTION()
	void OnEnemyDefeated(AActor* DefeatedEnemy);
	
	/** Handle player death */
	UFUNCTION()
	void OnPlayerDeath();
	
	/** Handle station destruction */
	UFUNCTION()
	void OnStationDestroyed();
	
	/** Update run statistics */
	void UpdateRunStats();

protected:
	// ========================================
	// PROPERTIES
	// ========================================
	
	/** Current state of the run */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ERunState CurrentRunState = ERunState::PreRun;
	
	/** Current level in the run (1-5) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 CurrentLevel = 1;
	
	/** Currently active room */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	URoomDataAsset* CurrentRoom;
	
	/** Rooms remaining in this run */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TArray<URoomDataAsset*> RemainingRooms;
	
	/** Rooms completed in this run */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TArray<URoomDataAsset*> CompletedRooms;
	
	/** All available room data assets */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	TArray<URoomDataAsset*> AllRoomDataAssets;
	
	/** Currently spawned enemy */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	AGameCharacterBase* CurrentRoomEnemy;
	
	/** Reference to player's slot manager */
	UPROPERTY()
	USlotManagerComponent* PlayerSlotManager;
	
	/** Reference to player character */
	UPROPERTY()
	AGameCharacterBase* PlayerCharacter;
	
	/** Time when current room started */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float RoomStartTime;
	
	/** Run statistics */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
	FRunProgressData RunProgress;
	
	/** Transition duration between rooms */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	float RoomTransitionDuration = 2.0f;
	
	/** Delay before combat starts in new room */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	float CombatStartDelay = 3.0f;
	
	/** Default spawn point for enemies if room doesn't specify */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	FTransform DefaultEnemySpawnPoint;
	
	/** Whether to auto-save after each room */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	bool bAutoSaveProgress = true;
	
	
	/** Current room actor instance */
	UPROPERTY()
	ARoomBase* CurrentRoomInstance;
	
	// ========================================
	// TEST ARENA PROPERTIES
	// ========================================
	
	/** All room actors in test arena (populated at runtime) */
	UPROPERTY()
	TArray<ARoomBase*> TestArenaRooms;
	
	/** Current room index in test sequence */
	int32 TestSequenceIndex = 0;
	
	/** Whether we're in test arena mode */
	bool bTestArenaMode = false;
};