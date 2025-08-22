// StationDegradationSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/EngineTypes.h"
#include "StationDegradationSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDegradationLevel : uint8
{
    None         UMETA(DisplayName = "None"),
    Minor        UMETA(DisplayName = "Minor"),
    Moderate     UMETA(DisplayName = "Moderate"),
    Severe       UMETA(DisplayName = "Severe"),
    Critical     UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FDegradationEffect
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDegradationLevel RequiredLevel = EDegradationLevel::Minor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActivationChance = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinInterval = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxInterval = 30.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EffectName = "Generic Degradation";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPermanent = false;
    
    float LastActivationTime = 0.0f;
    float NextActivationTime = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDegradationLevelChanged, EDegradationLevel, OldLevel, EDegradationLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDegradationEffectTriggered, const FString&, EffectName);

UCLASS()
class ATLAS_API UStationDegradationSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    // FTickableGameObject interface
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return bIsActive; }
    virtual TStatId GetStatId() const override;

    // Degradation management
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void StartDegradation();
    
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void StopDegradation();
    
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void ResetDegradation();
    
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void SetDegradationRate(float NewRate);
    
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void AccelerateDegradation(float Multiplier, float Duration);
    
    // Level management
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void SetDegradationLevel(EDegradationLevel NewLevel);
    
    UFUNCTION(BlueprintPure, Category = "Degradation")
    EDegradationLevel GetCurrentDegradationLevel() const { return CurrentLevel; }
    
    UFUNCTION(BlueprintPure, Category = "Degradation")
    float GetDegradationProgress() const { return DegradationProgress; }
    
    UFUNCTION(BlueprintPure, Category = "Degradation")
    float GetTimeUntilNextLevel() const;
    
    // Effect management
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void TriggerRandomEffect();
    
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void TriggerSpecificEffect(const FString& EffectName);
    
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void AddCustomEffect(const FDegradationEffect& Effect);
    
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void RemoveEffect(const FString& EffectName);
    
    // Environmental control
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void SetLightingIntensity(float Intensity);
    
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void SetGravityMultiplier(float Multiplier);
    
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void SetOxygenLevel(float Level);
    
    UFUNCTION(BlueprintCallable, Category = "Degradation")
    void TriggerPowerFluctuation(float Duration);
    
    // Queries
    UFUNCTION(BlueprintPure, Category = "Degradation")
    bool IsActive() const { return bIsActive; }
    
    UFUNCTION(BlueprintPure, Category = "Degradation")
    float GetCurrentDegradationRate() const { return CurrentDegradationRate; }
    
    UFUNCTION(BlueprintPure, Category = "Degradation")
    TArray<FString> GetActiveEffects() const;
    
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Degradation")
    FOnDegradationLevelChanged OnDegradationLevelChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Degradation")
    FOnDegradationEffectTriggered OnDegradationEffectTriggered;

protected:
    void UpdateDegradation(float DeltaTime);
    void UpdateEffects(float DeltaTime);
    void ProcessDegradationLevel();
    void ApplyLevelEffects(EDegradationLevel Level);
    void InitializeDefaultEffects();
    
    // Effect implementations
    void TriggerLightFlicker();
    void TriggerSpark();
    void TriggerAlarm();
    void TriggerHullBreach();
    void TriggerSystemFailure();
    void TriggerGravityFluctuation();
    void TriggerEmergencyLighting();
    void TriggerOxygenLeak();
    
    EDegradationLevel CalculateLevelFromProgress(float Progress) const;
    float GetProgressThresholdForLevel(EDegradationLevel Level) const;

protected:
    // Configuration
    UPROPERTY(EditDefaultsOnly, Category = "Config")
    float BaseDegradationRate = 0.01f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Config")
    float MaxDegradationProgress = 100.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Config")
    bool bAutoStart = false;
    
    UPROPERTY(EditDefaultsOnly, Category = "Config")
    TArray<FDegradationEffect> DefaultEffects;
    
    // Runtime state
    UPROPERTY()
    bool bIsActive = false;
    
    UPROPERTY()
    float DegradationProgress = 0.0f;
    
    UPROPERTY()
    EDegradationLevel CurrentLevel = EDegradationLevel::None;
    
    UPROPERTY()
    float CurrentDegradationRate;
    
    UPROPERTY()
    TArray<FDegradationEffect> ActiveEffects;
    
    UPROPERTY()
    TArray<FString> CurrentlyActiveEffects;
    
    // Acceleration
    float AccelerationMultiplier = 1.0f;
    float AccelerationTimeRemaining = 0.0f;
    
    // Environmental states
    float CurrentLightingIntensity = 1.0f;
    float CurrentGravityMultiplier = 1.0f;
    float CurrentOxygenLevel = 100.0f;
    
    // Timers
    FTimerHandle PowerFluctuationTimer;
    FTimerHandle GravityFluctuationTimer;
    
private:
    void CleanupTimers();
};