// EmergencyEventManager.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "EmergencyEventManager.generated.h"

UENUM(BlueprintType)
enum class EEmergencyType : uint8
{
    None            UMETA(DisplayName = "None"),
    PowerFailure    UMETA(DisplayName = "Power Failure"),
    HullBreach      UMETA(DisplayName = "Hull Breach"),
    FireOutbreak    UMETA(DisplayName = "Fire Outbreak"),
    SystemOverload  UMETA(DisplayName = "System Overload"),
    Lockdown        UMETA(DisplayName = "Lockdown"),
    OxygenLeak      UMETA(DisplayName = "Oxygen Leak"),
    ReactorMeltdown UMETA(DisplayName = "Reactor Meltdown")
};

UENUM(BlueprintType)
enum class EEmergencyPriority : uint8
{
    Low      UMETA(DisplayName = "Low"),
    Medium   UMETA(DisplayName = "Medium"),
    High     UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FEmergencyEvent
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmergencyType Type = EEmergencyType::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmergencyPriority Priority = EEmergencyPriority::Low;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 30.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Severity = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresPlayerAction = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventMessage = "";
    
    float StartTime = 0.0f;
    bool bIsActive = false;
    bool bIsResolved = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmergencyEventStarted, const FEmergencyEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmergencyEventEnded, const FEmergencyEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmergencyEventUpdated, const FEmergencyEvent&, Event);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UEmergencyEventManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEmergencyEventManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Event management
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerEmergencyEvent(EEmergencyType Type, const FVector& Location, float Duration = 30.0f, float Severity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerRandomEmergency();
    
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ResolveEmergency(EEmergencyType Type);
    
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ResolveAllEmergencies();
    
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void EscalateEmergency(EEmergencyType Type);
    
    // Query functions
    UFUNCTION(BlueprintPure, Category = "Emergency")
    bool IsEmergencyActive(EEmergencyType Type) const;
    
    UFUNCTION(BlueprintPure, Category = "Emergency")
    TArray<FEmergencyEvent> GetActiveEmergencies() const;
    
    UFUNCTION(BlueprintPure, Category = "Emergency")
    FEmergencyEvent GetEmergencyByType(EEmergencyType Type) const;
    
    UFUNCTION(BlueprintPure, Category = "Emergency")
    int32 GetActiveEmergencyCount() const;
    
    UFUNCTION(BlueprintPure, Category = "Emergency")
    EEmergencyPriority GetHighestPriority() const;
    
    // Emergency responses
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ActivateEmergencyLighting();
    
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void DeactivateEmergencyLighting();
    
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void SoundAlarm(EEmergencyType Type);
    
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void SilenceAlarms();
    
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void InitiateLockdown(float Duration = 60.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void EndLockdown();
    
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void ActivateFireSuppression(const FVector& Location);
    
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void VentAtmosphere(const FVector& Location);
    
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Emergency")
    FOnEmergencyEventStarted OnEmergencyEventStarted;
    
    UPROPERTY(BlueprintAssignable, Category = "Emergency")
    FOnEmergencyEventEnded OnEmergencyEventEnded;
    
    UPROPERTY(BlueprintAssignable, Category = "Emergency")
    FOnEmergencyEventUpdated OnEmergencyEventUpdated;

protected:
    void UpdateEmergencies(float DeltaTime);
    void StartEmergency(FEmergencyEvent& Event);
    void EndEmergency(FEmergencyEvent& Event);
    void ApplyEmergencyEffects(const FEmergencyEvent& Event);
    void RemoveEmergencyEffects(const FEmergencyEvent& Event);
    
    // Emergency type handlers
    void HandlePowerFailure(const FEmergencyEvent& Event, bool bStart);
    void HandleHullBreach(const FEmergencyEvent& Event, bool bStart);
    void HandleFireOutbreak(const FEmergencyEvent& Event, bool bStart);
    void HandleSystemOverload(const FEmergencyEvent& Event, bool bStart);
    void HandleLockdown(const FEmergencyEvent& Event, bool bStart);
    void HandleOxygenLeak(const FEmergencyEvent& Event, bool bStart);
    void HandleReactorMeltdown(const FEmergencyEvent& Event, bool bStart);
    
    EEmergencyPriority DeterminePriority(EEmergencyType Type, float Severity) const;
    FString GenerateEventMessage(EEmergencyType Type, float Severity) const;
    
    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Emergency")
    void OnEmergencyTriggered(EEmergencyType Type, const FVector& Location);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Emergency")
    void OnEmergencyResolved(EEmergencyType Type);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Emergency")
    void OnCriticalEmergency();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    bool bAutoTriggerEmergencies = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    float MinTimeBetweenEmergencies = 60.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    float MaxTimeBetweenEmergencies = 180.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    int32 MaxSimultaneousEmergencies = 3;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    float EmergencyEscalationTime = 30.0f;
    
    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    USoundBase* GeneralAlarmSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    USoundBase* CriticalAlarmSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    USoundBase* LockdownSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    USoundBase* AllClearSound;
    
    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    UParticleSystem* FireEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    UParticleSystem* SmokeEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    UParticleSystem* SparkEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    UParticleSystem* VentingEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TSubclassOf<UCameraShakeBase> EmergencyCameraShake;

private:
    // Runtime state
    UPROPERTY()
    TArray<FEmergencyEvent> ActiveEmergencies;
    
    UPROPERTY()
    TArray<FEmergencyEvent> ResolvedEmergencies;
    
    UPROPERTY()
    bool bEmergencyLightingActive = false;
    
    UPROPERTY()
    bool bLockdownActive = false;
    
    UPROPERTY()
    bool bAlarmsActive = false;
    
    float TimeUntilNextEmergency = 0.0f;
    float LastEmergencyTime = 0.0f;
    
    // Active effects
    UPROPERTY()
    TArray<class UParticleSystemComponent*> ActiveEffects;
    
    UPROPERTY()
    TArray<class UAudioComponent*> ActiveAlarms;
    
    // Timers
    FTimerHandle LockdownTimerHandle;
    FTimerHandle PowerFailureTimerHandle;
    FTimerHandle EscalationTimerHandle;
    
    void CleanupEffects();
    void CleanupAlarms();
};