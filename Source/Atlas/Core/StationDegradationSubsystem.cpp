// StationDegradationSubsystem.cpp
#include "StationDegradationSubsystem.h"
#include "../Components/StationIntegrityComponent.h"
#include "../Components/IntegrityVisualizerComponent.h"
#include "../Hazards/EnvironmentalHazardComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/LightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PostProcessVolume.h"
#include "PhysicsEngine/PhysicsSettings.h"

void UStationDegradationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentDegradationRate = BaseDegradationRate;
    InitializeDefaultEffects();
    
    if (bAutoStart)
    {
        StartDegradation();
    }
}

void UStationDegradationSubsystem::Deinitialize()
{
    StopDegradation();
    CleanupTimers();
    
    Super::Deinitialize();
}

void UStationDegradationSubsystem::Tick(float DeltaTime)
{
    if (!bIsActive) return;
    
    UpdateDegradation(DeltaTime);
    UpdateEffects(DeltaTime);
    
    // Update acceleration
    if (AccelerationTimeRemaining > 0.0f)
    {
        AccelerationTimeRemaining -= DeltaTime;
        if (AccelerationTimeRemaining <= 0.0f)
        {
            AccelerationMultiplier = 1.0f;
        }
    }
}

TStatId UStationDegradationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UStationDegradationSubsystem, STATGROUP_Tickables);
}

void UStationDegradationSubsystem::StartDegradation()
{
    if (bIsActive) return;
    
    bIsActive = true;
    DegradationProgress = 0.0f;
    CurrentLevel = EDegradationLevel::None;
    
    UE_LOG(LogTemp, Log, TEXT("Station degradation started"));
}

void UStationDegradationSubsystem::StopDegradation()
{
    if (!bIsActive) return;
    
    bIsActive = false;
    CleanupTimers();
    
    UE_LOG(LogTemp, Log, TEXT("Station degradation stopped"));
}

void UStationDegradationSubsystem::ResetDegradation()
{
    StopDegradation();
    
    DegradationProgress = 0.0f;
    CurrentLevel = EDegradationLevel::None;
    AccelerationMultiplier = 1.0f;
    AccelerationTimeRemaining = 0.0f;
    
    CurrentLightingIntensity = 1.0f;
    CurrentGravityMultiplier = 1.0f;
    CurrentOxygenLevel = 100.0f;
    
    CurrentlyActiveEffects.Empty();
    
    // Reset all effect timers
    for (FDegradationEffect& Effect : ActiveEffects)
    {
        Effect.LastActivationTime = 0.0f;
        Effect.NextActivationTime = FMath::RandRange(Effect.MinInterval, Effect.MaxInterval);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Station degradation reset"));
}

void UStationDegradationSubsystem::SetDegradationRate(float NewRate)
{
    CurrentDegradationRate = FMath::Max(0.0f, NewRate);
}

void UStationDegradationSubsystem::AccelerateDegradation(float Multiplier, float Duration)
{
    AccelerationMultiplier = FMath::Max(1.0f, Multiplier);
    AccelerationTimeRemaining = FMath::Max(0.0f, Duration);
    
    UE_LOG(LogTemp, Log, TEXT("Degradation accelerated by %.1fx for %.1f seconds"), 
        AccelerationMultiplier, AccelerationTimeRemaining);
}

void UStationDegradationSubsystem::SetDegradationLevel(EDegradationLevel NewLevel)
{
    if (NewLevel == CurrentLevel) return;
    
    EDegradationLevel OldLevel = CurrentLevel;
    CurrentLevel = NewLevel;
    
    // Set progress to match level
    DegradationProgress = GetProgressThresholdForLevel(NewLevel);
    
    ApplyLevelEffects(NewLevel);
    OnDegradationLevelChanged.Broadcast(OldLevel, NewLevel);
    
    UE_LOG(LogTemp, Log, TEXT("Degradation level changed from %d to %d"), 
        (int32)OldLevel, (int32)NewLevel);
}

float UStationDegradationSubsystem::GetTimeUntilNextLevel() const
{
    if (!bIsActive || CurrentLevel == EDegradationLevel::Critical)
    {
        return -1.0f;
    }
    
    EDegradationLevel NextLevel = (EDegradationLevel)((int32)CurrentLevel + 1);
    float NextThreshold = GetProgressThresholdForLevel(NextLevel);
    float ProgressNeeded = NextThreshold - DegradationProgress;
    
    float EffectiveRate = CurrentDegradationRate * AccelerationMultiplier;
    if (EffectiveRate <= 0.0f)
    {
        return -1.0f;
    }
    
    return ProgressNeeded / EffectiveRate;
}

void UStationDegradationSubsystem::TriggerRandomEffect()
{
    TArray<FDegradationEffect*> ValidEffects;
    
    for (FDegradationEffect& Effect : ActiveEffects)
    {
        if (Effect.RequiredLevel <= CurrentLevel)
        {
            ValidEffects.Add(&Effect);
        }
    }
    
    if (ValidEffects.Num() > 0)
    {
        FDegradationEffect* SelectedEffect = ValidEffects[FMath::RandRange(0, ValidEffects.Num() - 1)];
        TriggerSpecificEffect(SelectedEffect->EffectName);
    }
}

void UStationDegradationSubsystem::TriggerSpecificEffect(const FString& EffectName)
{
    // Trigger built-in effects
    if (EffectName == "LightFlicker")
    {
        TriggerLightFlicker();
    }
    else if (EffectName == "Spark")
    {
        TriggerSpark();
    }
    else if (EffectName == "Alarm")
    {
        TriggerAlarm();
    }
    else if (EffectName == "HullBreach")
    {
        TriggerHullBreach();
    }
    else if (EffectName == "SystemFailure")
    {
        TriggerSystemFailure();
    }
    else if (EffectName == "GravityFluctuation")
    {
        TriggerGravityFluctuation();
    }
    else if (EffectName == "EmergencyLighting")
    {
        TriggerEmergencyLighting();
    }
    else if (EffectName == "OxygenLeak")
    {
        TriggerOxygenLeak();
    }
    
    CurrentlyActiveEffects.AddUnique(EffectName);
    OnDegradationEffectTriggered.Broadcast(EffectName);
    
    UE_LOG(LogTemp, Log, TEXT("Triggered degradation effect: %s"), *EffectName);
}

void UStationDegradationSubsystem::AddCustomEffect(const FDegradationEffect& Effect)
{
    ActiveEffects.Add(Effect);
}

void UStationDegradationSubsystem::RemoveEffect(const FString& EffectName)
{
    ActiveEffects.RemoveAll([&EffectName](const FDegradationEffect& Effect)
    {
        return Effect.EffectName == EffectName;
    });
    
    CurrentlyActiveEffects.Remove(EffectName);
}

void UStationDegradationSubsystem::SetLightingIntensity(float Intensity)
{
    CurrentLightingIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    // Update all lights in the world
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> Lights;
    UGameplayStatics::GetAllActorsOfClass(World, ALight::StaticClass(), Lights);
    
    for (AActor* LightActor : Lights)
    {
        if (ALight* Light = Cast<ALight>(LightActor))
        {
            if (ULightComponent* LightComp = Light->GetLightComponent())
            {
                LightComp->SetIntensity(LightComp->Intensity * CurrentLightingIntensity);
            }
        }
    }
}

void UStationDegradationSubsystem::SetGravityMultiplier(float Multiplier)
{
    CurrentGravityMultiplier = FMath::Clamp(Multiplier, 0.0f, 2.0f);
    
    // This would need to be implemented based on your physics setup
    // For now, we'll just store the value
}

void UStationDegradationSubsystem::SetOxygenLevel(float Level)
{
    CurrentOxygenLevel = FMath::Clamp(Level, 0.0f, 100.0f);
    
    // Apply oxygen effects to players
    if (CurrentOxygenLevel < 50.0f)
    {
        // Could apply damage or effects to players
    }
}

void UStationDegradationSubsystem::TriggerPowerFluctuation(float Duration)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Start flickering lights
    float FlickerRate = 0.1f;
    int32 FlickerCount = Duration / FlickerRate;
    
    FTimerDelegate FlickerDelegate;
    FlickerDelegate.BindLambda([this, FlickerCount]() mutable
    {
        static int32 CurrentFlicker = 0;
        
        if (CurrentFlicker < FlickerCount)
        {
            float Intensity = (CurrentFlicker % 2 == 0) ? 0.3f : 1.0f;
            SetLightingIntensity(Intensity);
            CurrentFlicker++;
        }
        else
        {
            SetLightingIntensity(1.0f);
            CurrentFlicker = 0;
            GetWorld()->GetTimerManager().ClearTimer(PowerFluctuationTimer);
        }
    });
    
    World->GetTimerManager().SetTimer(PowerFluctuationTimer, FlickerDelegate, FlickerRate, true);
}

TArray<FString> UStationDegradationSubsystem::GetActiveEffects() const
{
    return CurrentlyActiveEffects;
}

void UStationDegradationSubsystem::UpdateDegradation(float DeltaTime)
{
    // Update degradation progress
    float EffectiveRate = CurrentDegradationRate * AccelerationMultiplier;
    DegradationProgress += EffectiveRate * DeltaTime;
    DegradationProgress = FMath::Clamp(DegradationProgress, 0.0f, MaxDegradationProgress);
    
    // Check for level changes
    ProcessDegradationLevel();
}

void UStationDegradationSubsystem::UpdateEffects(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    float CurrentTime = World->GetTimeSeconds();
    
    for (FDegradationEffect& Effect : ActiveEffects)
    {
        // Check if effect should activate
        if (Effect.RequiredLevel <= CurrentLevel && CurrentTime >= Effect.NextActivationTime)
        {
            // Roll for activation
            if (FMath::FRand() <= Effect.ActivationChance)
            {
                TriggerSpecificEffect(Effect.EffectName);
                Effect.LastActivationTime = CurrentTime;
            }
            
            // Schedule next activation
            Effect.NextActivationTime = CurrentTime + FMath::RandRange(Effect.MinInterval, Effect.MaxInterval);
        }
    }
}

void UStationDegradationSubsystem::ProcessDegradationLevel()
{
    EDegradationLevel NewLevel = CalculateLevelFromProgress(DegradationProgress);
    
    if (NewLevel != CurrentLevel)
    {
        EDegradationLevel OldLevel = CurrentLevel;
        CurrentLevel = NewLevel;
        
        ApplyLevelEffects(NewLevel);
        OnDegradationLevelChanged.Broadcast(OldLevel, NewLevel);
    }
}

void UStationDegradationSubsystem::ApplyLevelEffects(EDegradationLevel Level)
{
    switch (Level)
    {
        case EDegradationLevel::Minor:
            SetLightingIntensity(0.9f);
            break;
            
        case EDegradationLevel::Moderate:
            SetLightingIntensity(0.75f);
            TriggerRandomEffect();
            break;
            
        case EDegradationLevel::Severe:
            SetLightingIntensity(0.5f);
            SetOxygenLevel(75.0f);
            TriggerRandomEffect();
            break;
            
        case EDegradationLevel::Critical:
            SetLightingIntensity(0.3f);
            SetOxygenLevel(50.0f);
            SetGravityMultiplier(0.7f);
            TriggerEmergencyLighting();
            TriggerAlarm();
            break;
            
        default:
            break;
    }
}

void UStationDegradationSubsystem::InitializeDefaultEffects()
{
    // Light flicker
    FDegradationEffect LightFlicker;
    LightFlicker.EffectName = "LightFlicker";
    LightFlicker.RequiredLevel = EDegradationLevel::Minor;
    LightFlicker.ActivationChance = 0.3f;
    LightFlicker.MinInterval = 15.0f;
    LightFlicker.MaxInterval = 45.0f;
    ActiveEffects.Add(LightFlicker);
    
    // Sparks
    FDegradationEffect Sparks;
    Sparks.EffectName = "Spark";
    Sparks.RequiredLevel = EDegradationLevel::Moderate;
    Sparks.ActivationChance = 0.4f;
    Sparks.MinInterval = 10.0f;
    Sparks.MaxInterval = 30.0f;
    ActiveEffects.Add(Sparks);
    
    // Hull breach warning
    FDegradationEffect HullBreach;
    HullBreach.EffectName = "HullBreach";
    HullBreach.RequiredLevel = EDegradationLevel::Severe;
    HullBreach.ActivationChance = 0.2f;
    HullBreach.MinInterval = 30.0f;
    HullBreach.MaxInterval = 60.0f;
    ActiveEffects.Add(HullBreach);
    
    // System failure
    FDegradationEffect SystemFailure;
    SystemFailure.EffectName = "SystemFailure";
    SystemFailure.RequiredLevel = EDegradationLevel::Critical;
    SystemFailure.ActivationChance = 0.5f;
    SystemFailure.MinInterval = 20.0f;
    SystemFailure.MaxInterval = 40.0f;
    ActiveEffects.Add(SystemFailure);
    
    // Add default effects
    ActiveEffects.Append(DefaultEffects);
}

void UStationDegradationSubsystem::TriggerLightFlicker()
{
    TriggerPowerFluctuation(2.0f);
}

void UStationDegradationSubsystem::TriggerSpark()
{
    // Find integrity visualizers and trigger spark effect
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
    
    for (AActor* Actor : Actors)
    {
        if (UIntegrityVisualizerComponent* Visualizer = Actor->FindComponentByClass<UIntegrityVisualizerComponent>())
        {
            // Generate random location for electrical failure
            FVector RandomLocation = Actor->GetActorLocation() + FMath::VRand() * 500.0f;
            Visualizer->TriggerElectricalFailure(RandomLocation);
            break;
        }
    }
}

void UStationDegradationSubsystem::TriggerAlarm()
{
    // Would play alarm sounds and flash warning lights
    UE_LOG(LogTemp, Warning, TEXT("ALARM: Station integrity critical!"));
}

void UStationDegradationSubsystem::TriggerHullBreach()
{
    // Find integrity visualizers and trigger breach
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
    
    for (AActor* Actor : Actors)
    {
        if (UIntegrityVisualizerComponent* Visualizer = Actor->FindComponentByClass<UIntegrityVisualizerComponent>())
        {
            // Trigger hull breach at random location
            FVector BreachLocation = Actor->GetActorLocation() + FMath::VRand() * 500.0f;
            Visualizer->TriggerHullBreach(BreachLocation);
            break;
        }
    }
}

void UStationDegradationSubsystem::TriggerSystemFailure()
{
    // Major system failure - multiple effects
    TriggerPowerFluctuation(5.0f);
    SetOxygenLevel(CurrentOxygenLevel - 10.0f);
    TriggerAlarm();
}

void UStationDegradationSubsystem::TriggerGravityFluctuation()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Fluctuate gravity over time
    FTimerDelegate GravityDelegate;
    GravityDelegate.BindLambda([this]()
    {
        static int32 FluctuationCount = 0;
        
        if (FluctuationCount < 5)
        {
            float NewGravity = FMath::RandRange(0.3f, 1.5f);
            SetGravityMultiplier(NewGravity);
            FluctuationCount++;
        }
        else
        {
            SetGravityMultiplier(1.0f);
            FluctuationCount = 0;
            GetWorld()->GetTimerManager().ClearTimer(GravityFluctuationTimer);
        }
    });
    
    World->GetTimerManager().SetTimer(GravityFluctuationTimer, GravityDelegate, 1.0f, true);
}

void UStationDegradationSubsystem::TriggerEmergencyLighting()
{
    SetLightingIntensity(0.3f);
    // Would also change light colors to red/emergency colors
}

void UStationDegradationSubsystem::TriggerOxygenLeak()
{
    SetOxygenLevel(CurrentOxygenLevel - 20.0f);
}

EDegradationLevel UStationDegradationSubsystem::CalculateLevelFromProgress(float Progress) const
{
    float Percent = Progress / MaxDegradationProgress;
    
    if (Percent >= 0.8f) return EDegradationLevel::Critical;
    if (Percent >= 0.6f) return EDegradationLevel::Severe;
    if (Percent >= 0.4f) return EDegradationLevel::Moderate;
    if (Percent >= 0.2f) return EDegradationLevel::Minor;
    
    return EDegradationLevel::None;
}

float UStationDegradationSubsystem::GetProgressThresholdForLevel(EDegradationLevel Level) const
{
    switch (Level)
    {
        case EDegradationLevel::Minor: return MaxDegradationProgress * 0.2f;
        case EDegradationLevel::Moderate: return MaxDegradationProgress * 0.4f;
        case EDegradationLevel::Severe: return MaxDegradationProgress * 0.6f;
        case EDegradationLevel::Critical: return MaxDegradationProgress * 0.8f;
        default: return 0.0f;
    }
}

void UStationDegradationSubsystem::CleanupTimers()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    World->GetTimerManager().ClearTimer(PowerFluctuationTimer);
    World->GetTimerManager().ClearTimer(GravityFluctuationTimer);
}