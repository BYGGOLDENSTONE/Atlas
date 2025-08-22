#pragma once

#include "CoreMinimal.h"
#include "RoomBase.h"
#include "EngineeringBayRoom.generated.h"

// Forward declarations
class USoundBase;
class UParticleSystem;

/**
 * Engineering Bay room with industrial hazards and machinery
 */
UCLASS()
class ATLAS_API AEngineeringBayRoom : public ARoomBase
{
	GENERATED_BODY()

public:
	AEngineeringBayRoom();

protected:
	virtual void BeginPlay() override;
	virtual void ActivateRoom(URoomDataAsset* RoomData) override;
	virtual void DeactivateRoom() override;

	// ========================================
	// UNIQUE MECHANICS
	// ========================================

	/**
	 * Trigger a steam vent burst at a random hazard point
	 */
	UFUNCTION(BlueprintCallable, Category = "Engineering Bay")
	void TriggerSteamVentBurst();

	/**
	 * Cause a pipe to burst, creating a hazard zone
	 */
	UFUNCTION(BlueprintCallable, Category = "Engineering Bay")
	void BurstPipe(FVector Location);

	/**
	 * Activate machinery that can damage players/enemies
	 */
	UFUNCTION(BlueprintCallable, Category = "Engineering Bay")
	void ActivateMachinery(int32 MachineIndex);

	/**
	 * Create an electrical surge hazard
	 */
	UFUNCTION(BlueprintCallable, Category = "Engineering Bay")
	void CreateElectricalSurge();

	/**
	 * Toggle emergency lighting (red flashing lights)
	 */
	UFUNCTION(BlueprintCallable, Category = "Engineering Bay")
	void ToggleEmergencyLighting(bool bEnabled);

	// ========================================
	// ENVIRONMENTAL PATTERNS
	// ========================================

	/**
	 * Start the room's hazard pattern sequence
	 */
	void StartHazardSequence();

	/**
	 * Stop all active hazard sequences
	 */
	void StopHazardSequence();

	/**
	 * Update flickering lights effect
	 */
	void UpdateFlickeringLights();

	/**
	 * Apply industrial atmosphere effects
	 */
	virtual void ApplyEnvironmentalEffects() override;

	/**
	 * Remove industrial atmosphere effects
	 */
	virtual void RemoveEnvironmentalEffects() override;

protected:
	// ========================================
	// CONFIGURATION
	// ========================================

	/** Steam vent particle effect */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* SteamVentEffect;

	/** Electrical surge particle effect */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* ElectricalSurgeEffect;

	/** Pipe burst particle effect */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* PipeBurstEffect;

	/** Industrial ambient sound */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* IndustrialAmbientSound;

	/** Steam hiss sound */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* SteamHissSound;

	/** Electrical surge sound */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* ElectricalSurgeSound;

	/** Metal creaking sound */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* MetalCreakingSound;

	/** Damage dealt by steam vents */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float SteamVentDamage;

	/** Damage dealt by electrical surges */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float ElectricalSurgeDamage;

	/** Interval between hazard triggers */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float HazardInterval;

	/** Chance for explosive barrel to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float ExplosiveBarrelSpawnChance;

private:
	// ========================================
	// RUNTIME STATE
	// ========================================

	/** Timer handle for hazard sequence */
	FTimerHandle HazardSequenceTimer;

	/** Timer handle for flickering lights */
	FTimerHandle FlickerTimer;

	/** Active steam vent locations */
	TArray<FVector> ActiveSteamVents;

	/** Active electrical surge zones */
	TArray<FVector> ActiveElectricalZones;

	/** Whether emergency lighting is active */
	bool bEmergencyLightingActive;

	/** Index for cycling hazard patterns */
	int32 CurrentHazardPattern;
};