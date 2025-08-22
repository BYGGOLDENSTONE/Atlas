// IntegrityVisualizerComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Light.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Camera/CameraShakeBase.h"
#include "IntegrityVisualizerComponent.generated.h"

UENUM(BlueprintType)
enum class EIntegrityState : uint8
{
    Normal         UMETA(DisplayName = "Normal"),
    MinorDamage    UMETA(DisplayName = "Minor Damage"),
    Warning        UMETA(DisplayName = "Warning"),
    Critical       UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ESeverity : uint8
{
    Light     UMETA(DisplayName = "Light"),
    Medium    UMETA(DisplayName = "Medium"),
    Heavy     UMETA(DisplayName = "Heavy"),
    Critical  UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class ECameraShakeIntensity : uint8
{
    None      UMETA(DisplayName = "None"),
    Light     UMETA(DisplayName = "Light"),
    Medium    UMETA(DisplayName = "Medium"),
    Heavy     UMETA(DisplayName = "Heavy")
};

USTRUCT(BlueprintType)
struct FHullBreachData
{
    GENERATED_BODY()
    
    UPROPERTY()
    FVector Location;
    
    UPROPERTY()
    float Severity = 1.0f;
    
    UPROPERTY()
    float Radius = 500.0f;
    
    UPROPERTY()
    class UParticleSystemComponent* VacuumEffect = nullptr;
    
    UPROPERTY()
    class UAudioComponent* VacuumSound = nullptr;
    
    FHullBreachData()
    {
        Location = FVector::ZeroVector;
        Severity = 1.0f;
        Radius = 500.0f;
        VacuumEffect = nullptr;
        VacuumSound = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UIntegrityVisualizerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UIntegrityVisualizerComponent();

    // Integrity thresholds for visual states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
    float CriticalThreshold = 25.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
    float WarningThreshold = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
    float MinorDamageThreshold = 75.0f;
    
    // Visual effect assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UParticleSystem* SparksEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UParticleSystem* SteamLeakEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UParticleSystem* ElectricalArcEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UParticleSystem* HullBreachEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UParticleSystem* DebrisEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Audio")
    USoundCue* AlarmSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Audio")
    USoundCue* CreakingSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Audio")
    USoundCue* ElectricalSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Audio")
    USoundCue* VacuumSound;
    
    // Camera shake classes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Camera")
    TSubclassOf<UCameraShakeBase> LightShake;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Camera")
    TSubclassOf<UCameraShakeBase> MediumShake;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Camera")
    TSubclassOf<UCameraShakeBase> HeavyShake;
    
    // Update visual state based on integrity
    UFUNCTION(BlueprintCallable, Category = "Integrity")
    void UpdateIntegrityVisuals(float CurrentIntegrity);
    
    // Trigger specific damage effects
    UFUNCTION(BlueprintCallable, Category = "Integrity")
    void TriggerHullBreach(FVector Location);
    
    UFUNCTION(BlueprintCallable, Category = "Integrity")
    void TriggerElectricalFailure(FVector Location);
    
    UFUNCTION(BlueprintCallable, Category = "Integrity")
    void TriggerEmergencyLighting();
    
    UFUNCTION(BlueprintCallable, Category = "Integrity")
    void ClearAllEffects();
    
    // Get current integrity state
    UFUNCTION(BlueprintPure, Category = "Integrity")
    EIntegrityState GetCurrentIntegrityState() const { return CurrentState; }
    
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Integrity")
    void OnIntegrityStateChanged(EIntegrityState NewState, float IntegrityPercent);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Integrity")
    void OnHullBreachTriggered(const FVector& Location);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Integrity")
    void OnElectricalFailureTriggered(const FVector& Location);

private:
    // State management
    void HandleCriticalState();
    void HandleWarningState();
    void HandleMinorDamageState();
    void HandleNormalState();
    
    // Effect spawning
    void SpawnDamageEffects(float IntegrityPercent);
    void UpdateLightingState(float IntegrityPercent);
    void PlayIntegrityAudio(float IntegrityPercent);
    void ApplyCameraShake(ECameraShakeIntensity Intensity);
    void ApplyScreenDistortion(float Intensity);
    
    // Lighting control
    void SetAllLightsColor(const FLinearColor& Color);
    void SetLightIntensity(float Intensity);
    void EnableStrobeLighting(float Frequency);
    void EnableLightFlicker(float Interval);
    void DisableAllLightingEffects();
    
    // Particle management
    void SpawnSparksAtInterval(float Interval);
    void SpawnSteamLeaks(int32 Count);
    void CreateHullBreachPoints(int32 Count);
    
    // Audio management
    void PlayAlarmLoop(ESeverity Severity);
    void PlayCreakingSound(float Volume);
    void PlayAmbientCreaking(float Volume);
    void StopAllAudio();
    
    // Helper functions
    void ApplySuctionForce(const FVector& Location, float Radius, float Severity);
    void SpawnDebris(const FVector& Location, float Severity);
    class UParticleSystemComponent* SpawnVacuumParticles(const FVector& Location);
    
private:
    // Current state
    EIntegrityState CurrentState;
    float LastIntegrityPercent;
    
    // Active effects
    TArray<FHullBreachData> ActiveBreaches;
    TArray<class UParticleSystemComponent*> ActiveParticles;
    TArray<class UAudioComponent*> ActiveAudioComponents;
    
    // Timers
    FTimerHandle SparksTimerHandle;
    FTimerHandle FlickerTimerHandle;
    FTimerHandle StrobeTimerHandle;
    FTimerHandle CreakingTimerHandle;
    
    // Cached references
    UPROPERTY()
    class UAudioComponent* AlarmAudioComponent;
    
    UPROPERTY()
    class UAudioComponent* CreakingAudioComponent;
    
    UPROPERTY()
    TArray<ALight*> SceneLights;
    
    // Effect intervals
    float CurrentSparksInterval;
    float CurrentFlickerInterval;
    bool bStrobeActive;
    bool bEmergencyLightingActive;
};