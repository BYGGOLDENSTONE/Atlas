#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "RoomBase.generated.h"

// Forward declarations
class UBoxComponent;
class UArrowComponent;
class AGameCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomBaseCompleted, ARoomBase*, CompletedRoom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoomBaseActivated, ARoomBase*, ActivatedRoom);

/**
 * Base class for all room blueprints in Atlas.
 * Handles room activation, enemy spawning, hazard placement, and completion.
 */
UCLASS(Abstract, Blueprintable)
class ATLAS_API ARoomBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ARoomBase();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// ROOM MANAGEMENT
	// ========================================
	
	/**
	 * Activate this room with the given configuration
	 * @param RoomData The room data asset to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Room")
	virtual void ActivateRoom(URoomDataAsset* RoomData);
	
	/**
	 * Deactivate and clean up this room
	 */
	UFUNCTION(BlueprintCallable, Category = "Room")
	virtual void DeactivateRoom();
	
	/**
	 * Mark this room as completed
	 */
	UFUNCTION(BlueprintCallable, Category = "Room")
	virtual void CompleteRoom();
	
	/**
	 * Check if this room is currently active
	 */
	UFUNCTION(BlueprintPure, Category = "Room")
	bool IsRoomActive() const { return bIsRoomActive; }
	
	// ========================================
	// SPAWNING
	// ========================================
	
	/**
	 * Spawn the enemy for this room
	 */
	UFUNCTION(BlueprintCallable, Category = "Room|Spawning")
	virtual void SpawnRoomEnemy();
	
	/**
	 * Spawn hazards at designated points
	 */
	UFUNCTION(BlueprintCallable, Category = "Room|Spawning")
	virtual void SpawnRoomHazards();
	
	/**
	 * Spawn interactables for this room
	 */
	UFUNCTION(BlueprintCallable, Category = "Room|Spawning")
	virtual void SpawnRoomInteractables();
	
	/**
	 * Clear all spawned entities
	 */
	UFUNCTION(BlueprintCallable, Category = "Room|Spawning")
	virtual void ClearSpawnedEntities();
	
	// ========================================
	// SPAWN POINTS
	// ========================================
	
	/**
	 * Get the player spawn point for this room
	 */
	UFUNCTION(BlueprintPure, Category = "Room|Spawn Points")
	FTransform GetPlayerSpawnPoint() const;
	
	/**
	 * Get the enemy spawn point for this room
	 */
	UFUNCTION(BlueprintPure, Category = "Room|Spawn Points")
	FTransform GetEnemySpawnPoint() const;
	
	/**
	 * Get the reward spawn point for this room
	 */
	UFUNCTION(BlueprintPure, Category = "Room|Spawn Points")
	FTransform GetRewardSpawnPoint() const;
	
	/**
	 * Get all hazard spawn points
	 */
	UFUNCTION(BlueprintPure, Category = "Room|Spawn Points")
	TArray<FTransform> GetHazardSpawnPoints() const;
	
	/**
	 * Get all interactable spawn points
	 */
	UFUNCTION(BlueprintPure, Category = "Room|Spawn Points")
	TArray<FTransform> GetInteractableSpawnPoints() const;
	
	// ========================================
	// QUERIES
	// ========================================
	
	/**
	 * Get the current room data
	 */
	UFUNCTION(BlueprintPure, Category = "Room|Queries")
	URoomDataAsset* GetRoomData() const { return CurrentRoomData; }
	
	/**
	 * Get the spawned enemy
	 */
	UFUNCTION(BlueprintPure, Category = "Room|Queries")
	AGameCharacterBase* GetSpawnedEnemy() const { return SpawnedEnemy; }
	
	/**
	 * Get all spawned hazards
	 */
	UFUNCTION(BlueprintPure, Category = "Room|Queries")
	TArray<AActor*> GetSpawnedHazards() const { return SpawnedHazards; }
	
	// ========================================
	// EVENTS
	// ========================================
	
	/** Called when room is activated */
	UPROPERTY(BlueprintAssignable, Category = "Room|Events")
	FOnRoomBaseActivated OnRoomActivated;
	
	/** Called when room is completed */
	UPROPERTY(BlueprintAssignable, Category = "Room|Events")
	FOnRoomBaseCompleted OnRoomCompleted;
	
	// ========================================
	// BLUEPRINT EVENTS
	// ========================================
	
	/**
	 * Blueprint event called when room is activated
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Room|Events")
	void BP_OnRoomActivated();
	
	/**
	 * Blueprint event called when room is deactivated
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Room|Events")
	void BP_OnRoomDeactivated();
	
	/**
	 * Blueprint event called when room is completed
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Room|Events")
	void BP_OnRoomCompleted();
	
	/**
	 * Blueprint event for custom enemy spawn logic
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Room|Events")
	void BP_CustomEnemySpawn(AGameCharacterBase* Enemy);
	
	/**
	 * Blueprint event for custom hazard spawn logic
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Room|Events")
	void BP_CustomHazardSpawn();

protected:
	// ========================================
	// INTERNAL FUNCTIONS
	// ========================================
	
	/** Handle enemy death */
	UFUNCTION()
	virtual void OnEnemyDefeated(AActor* DefeatedActor);
	
	/** Handle exit trigger overlap */
	UFUNCTION()
	virtual void OnExitTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/** Apply room-specific environmental effects */
	virtual void ApplyEnvironmentalEffects();
	
	/** Remove room-specific environmental effects */
	virtual void RemoveEnvironmentalEffects();
	
	/** Start combat music for this room */
	virtual void StartCombatMusic();
	
	/** Stop combat music */
	virtual void StopCombatMusic();

protected:
	// ========================================
	// COMPONENTS
	// ========================================
	
	/** Root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RoomRoot;
	
	/** Bounds of the room */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* RoomBounds;
	
	/** Player spawn point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* PlayerSpawnPoint;
	
	/** Enemy spawn point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* EnemySpawnPoint;
	
	/** Reward spawn point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* RewardSpawnPoint;
	
	/** Exit trigger volume */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* ExitTrigger;
	
	/** Hazard spawn points */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<UArrowComponent*> HazardSpawnPoints;
	
	/** Interactable spawn points */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<UArrowComponent*> InteractableSpawnPoints;
	
	// ========================================
	// ROOM STATE
	// ========================================
	
	/** Current room data configuration */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	URoomDataAsset* CurrentRoomData;
	
	/** Whether this room is currently active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsRoomActive;
	
	/** The spawned enemy for this room */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	AGameCharacterBase* SpawnedEnemy;
	
	/** All spawned hazards */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TArray<AActor*> SpawnedHazards;
	
	/** All spawned interactables */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TArray<AActor*> SpawnedInteractables;
	
	/** Time when room was activated */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float RoomActivationTime;
	
	// ========================================
	// CONFIGURATION
	// ========================================
	
	/** Default hazard classes to spawn based on room hazard type */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	TMap<ERoomHazard, TSubclassOf<AActor>> HazardClasses;
	
	/** Default interactable classes for this room type */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	TArray<TSubclassOf<AActor>> InteractableClasses;
	
	/** Delay before enemy spawns after room activation */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	float EnemySpawnDelay;
	
	/** Whether exit is locked until enemy is defeated */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	bool bLockExitUntilClear;
	
	/** Override combat music for this specific room */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	class USoundBase* OverrideCombatMusic;
	
	/** Override ambient sound for this specific room */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	class USoundBase* OverrideAmbientSound;
};