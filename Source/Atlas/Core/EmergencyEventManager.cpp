// EmergencyEventManager.cpp
#include "EmergencyEventManager.h"
#include "../Components/StationIntegrityComponent.h"
#include "../Components/IntegrityVisualizerComponent.h"
#include "../Hazards/EnvironmentalHazardComponent.h"
#include "../Hazards/ElectricalSurgeHazard.h"
#include "../Hazards/ToxicLeakHazard.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/LightComponent.h"
#include "Engine/DirectionalLight.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "CollisionShape.h"

UEmergencyEventManager::UEmergencyEventManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UEmergencyEventManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoTriggerEmergencies)
    {
        TimeUntilNextEmergency = FMath::RandRange(MinTimeBetweenEmergencies, MaxTimeBetweenEmergencies);
    }
}

void UEmergencyEventManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ResolveAllEmergencies();
    CleanupEffects();
    CleanupAlarms();
    
    GetWorld()->GetTimerManager().ClearTimer(LockdownTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(PowerFailureTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(EscalationTimerHandle);
    
    Super::EndPlay(EndPlayReason);
}

void UEmergencyEventManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateEmergencies(DeltaTime);
    
    // Auto-trigger emergencies if enabled
    if (bAutoTriggerEmergencies && ActiveEmergencies.Num() < MaxSimultaneousEmergencies)
    {
        TimeUntilNextEmergency -= DeltaTime;
        if (TimeUntilNextEmergency <= 0.0f)
        {
            TriggerRandomEmergency();
            TimeUntilNextEmergency = FMath::RandRange(MinTimeBetweenEmergencies, MaxTimeBetweenEmergencies);
        }
    }
}

void UEmergencyEventManager::TriggerEmergencyEvent(EEmergencyType Type, const FVector& Location, float Duration, float Severity)
{
    if (Type == EEmergencyType::None) return;
    
    // Check if this type is already active
    if (IsEmergencyActive(Type))
    {
        UE_LOG(LogTemp, Warning, TEXT("Emergency type %d is already active"), (int32)Type);
        return;
    }
    
    // Check max emergencies
    if (ActiveEmergencies.Num() >= MaxSimultaneousEmergencies)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum number of emergencies reached"));
        return;
    }
    
    // Create new emergency
    FEmergencyEvent NewEvent;
    NewEvent.Type = Type;
    NewEvent.Location = Location;
    NewEvent.Duration = Duration;
    NewEvent.Severity = FMath::Clamp(Severity, 0.1f, 10.0f);
    NewEvent.Priority = DeterminePriority(Type, Severity);
    NewEvent.EventMessage = GenerateEventMessage(Type, Severity);
    NewEvent.StartTime = GetWorld()->GetTimeSeconds();
    NewEvent.bIsActive = true;
    
    // Determine if player action required
    NewEvent.bRequiresPlayerAction = (Type == EEmergencyType::FireOutbreak || 
                                      Type == EEmergencyType::HullBreach || 
                                      Type == EEmergencyType::ReactorMeltdown);
    
    ActiveEmergencies.Add(NewEvent);
    StartEmergency(ActiveEmergencies.Last());
    
    OnEmergencyEventStarted.Broadcast(NewEvent);
    OnEmergencyTriggered(Type, Location);
    
    if (NewEvent.Priority == EEmergencyPriority::Critical)
    {
        OnCriticalEmergency();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Emergency triggered: %s"), *NewEvent.EventMessage);
}

void UEmergencyEventManager::TriggerRandomEmergency()
{
    // Random emergency type
    TArray<EEmergencyType> PossibleTypes = {
        EEmergencyType::PowerFailure,
        EEmergencyType::HullBreach,
        EEmergencyType::FireOutbreak,
        EEmergencyType::SystemOverload,
        EEmergencyType::OxygenLeak
    };
    
    EEmergencyType RandomType = PossibleTypes[FMath::RandRange(0, PossibleTypes.Num() - 1)];
    
    // Random location within station bounds
    FVector RandomLocation = GetOwner()->GetActorLocation() + FMath::VRand() * 2000.0f;
    
    // Random severity
    float RandomSeverity = FMath::RandRange(0.5f, 2.0f);
    
    // Random duration
    float RandomDuration = FMath::RandRange(20.0f, 60.0f);
    
    TriggerEmergencyEvent(RandomType, RandomLocation, RandomDuration, RandomSeverity);
}

void UEmergencyEventManager::ResolveEmergency(EEmergencyType Type)
{
    for (int32 i = ActiveEmergencies.Num() - 1; i >= 0; --i)
    {
        if (ActiveEmergencies[i].Type == Type)
        {
            FEmergencyEvent& Event = ActiveEmergencies[i];
            Event.bIsResolved = true;
            EndEmergency(Event);
            
            ResolvedEmergencies.Add(Event);
            ActiveEmergencies.RemoveAt(i);
            
            OnEmergencyEventEnded.Broadcast(Event);
            OnEmergencyResolved(Type);
            
            UE_LOG(LogTemp, Log, TEXT("Emergency resolved: %s"), *Event.EventMessage);
            
            // Play all clear sound if no more emergencies
            if (ActiveEmergencies.Num() == 0 && AllClearSound)
            {
                UGameplayStatics::PlaySound2D(GetWorld(), AllClearSound);
            }
            
            break;
        }
    }
}

void UEmergencyEventManager::ResolveAllEmergencies()
{
    for (int32 i = ActiveEmergencies.Num() - 1; i >= 0; --i)
    {
        FEmergencyEvent& Event = ActiveEmergencies[i];
        Event.bIsResolved = true;
        EndEmergency(Event);
        OnEmergencyEventEnded.Broadcast(Event);
    }
    
    ActiveEmergencies.Empty();
    
    if (AllClearSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), AllClearSound);
    }
}

void UEmergencyEventManager::EscalateEmergency(EEmergencyType Type)
{
    for (FEmergencyEvent& Event : ActiveEmergencies)
    {
        if (Event.Type == Type)
        {
            Event.Severity = FMath::Min(Event.Severity * 1.5f, 10.0f);
            Event.Priority = DeterminePriority(Event.Type, Event.Severity);
            Event.EventMessage = GenerateEventMessage(Event.Type, Event.Severity);
            
            OnEmergencyEventUpdated.Broadcast(Event);
            
            if (Event.Priority == EEmergencyPriority::Critical)
            {
                OnCriticalEmergency();
                SoundAlarm(Type);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Emergency escalated: %s"), *Event.EventMessage);
            break;
        }
    }
}

bool UEmergencyEventManager::IsEmergencyActive(EEmergencyType Type) const
{
    for (const FEmergencyEvent& Event : ActiveEmergencies)
    {
        if (Event.Type == Type && Event.bIsActive)
        {
            return true;
        }
    }
    return false;
}

TArray<FEmergencyEvent> UEmergencyEventManager::GetActiveEmergencies() const
{
    return ActiveEmergencies;
}

FEmergencyEvent UEmergencyEventManager::GetEmergencyByType(EEmergencyType Type) const
{
    for (const FEmergencyEvent& Event : ActiveEmergencies)
    {
        if (Event.Type == Type)
        {
            return Event;
        }
    }
    
    return FEmergencyEvent();
}

int32 UEmergencyEventManager::GetActiveEmergencyCount() const
{
    return ActiveEmergencies.Num();
}

EEmergencyPriority UEmergencyEventManager::GetHighestPriority() const
{
    EEmergencyPriority Highest = EEmergencyPriority::Low;
    
    for (const FEmergencyEvent& Event : ActiveEmergencies)
    {
        if ((int32)Event.Priority > (int32)Highest)
        {
            Highest = Event.Priority;
        }
    }
    
    return Highest;
}

void UEmergencyEventManager::ActivateEmergencyLighting()
{
    if (bEmergencyLightingActive) return;
    
    bEmergencyLightingActive = true;
    
    // Change all lights to red emergency lighting
    TArray<AActor*> Lights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALight::StaticClass(), Lights);
    
    for (AActor* LightActor : Lights)
    {
        if (ALight* Light = Cast<ALight>(LightActor))
        {
            if (ULightComponent* LightComp = Light->GetLightComponent())
            {
                LightComp->SetLightColor(FLinearColor(1.0f, 0.0f, 0.0f));
                LightComp->SetIntensity(LightComp->Intensity * 0.5f);
            }
        }
    }
}

void UEmergencyEventManager::DeactivateEmergencyLighting()
{
    if (!bEmergencyLightingActive) return;
    
    bEmergencyLightingActive = false;
    
    // Restore normal lighting
    TArray<AActor*> Lights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALight::StaticClass(), Lights);
    
    for (AActor* LightActor : Lights)
    {
        if (ALight* Light = Cast<ALight>(LightActor))
        {
            if (ULightComponent* LightComp = Light->GetLightComponent())
            {
                LightComp->SetLightColor(FLinearColor::White);
                LightComp->SetIntensity(LightComp->Intensity * 2.0f);
            }
        }
    }
}

void UEmergencyEventManager::SoundAlarm(EEmergencyType Type)
{
    if (bAlarmsActive) return;
    
    bAlarmsActive = true;
    
    USoundBase* AlarmSound = nullptr;
    
    switch (Type)
    {
        case EEmergencyType::Lockdown:
            AlarmSound = LockdownSound;
            break;
        case EEmergencyType::ReactorMeltdown:
        case EEmergencyType::HullBreach:
            AlarmSound = CriticalAlarmSound;
            break;
        default:
            AlarmSound = GeneralAlarmSound;
            break;
    }
    
    if (AlarmSound)
    {
        UAudioComponent* AlarmComp = UGameplayStatics::SpawnSound2D(GetWorld(), AlarmSound);
        if (AlarmComp)
        {
            ActiveAlarms.Add(AlarmComp);
        }
    }
}

void UEmergencyEventManager::SilenceAlarms()
{
    bAlarmsActive = false;
    CleanupAlarms();
}

void UEmergencyEventManager::InitiateLockdown(float Duration)
{
    if (bLockdownActive) return;
    
    bLockdownActive = true;
    
    FEmergencyEvent LockdownEvent;
    LockdownEvent.Type = EEmergencyType::Lockdown;
    LockdownEvent.Duration = Duration;
    LockdownEvent.Severity = 2.0f;
    
    TriggerEmergencyEvent(EEmergencyType::Lockdown, GetOwner()->GetActorLocation(), Duration, 2.0f);
    
    // Set timer to end lockdown
    GetWorld()->GetTimerManager().SetTimer(LockdownTimerHandle, [this]()
    {
        EndLockdown();
    }, Duration, false);
}

void UEmergencyEventManager::EndLockdown()
{
    if (!bLockdownActive) return;
    
    bLockdownActive = false;
    ResolveEmergency(EEmergencyType::Lockdown);
    
    GetWorld()->GetTimerManager().ClearTimer(LockdownTimerHandle);
}

void UEmergencyEventManager::ActivateFireSuppression(const FVector& Location)
{
    // Spawn fire suppression effect
    if (VentingEffect)
    {
        UParticleSystemComponent* Suppression = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            VentingEffect,
            Location,
            FRotator::ZeroRotator,
            FVector(2.0f)
        );
        
        if (Suppression)
        {
            ActiveEffects.Add(Suppression);
        }
    }
    
    // Resolve fire if active at this location
    for (FEmergencyEvent& Event : ActiveEmergencies)
    {
        if (Event.Type == EEmergencyType::FireOutbreak)
        {
            float Distance = FVector::Dist(Event.Location, Location);
            if (Distance < 500.0f)
            {
                ResolveEmergency(EEmergencyType::FireOutbreak);
                break;
            }
        }
    }
}

void UEmergencyEventManager::VentAtmosphere(const FVector& Location)
{
    // Create venting effect
    if (VentingEffect)
    {
        UParticleSystemComponent* Vent = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            VentingEffect,
            Location,
            FRotator(-90.0f, 0.0f, 0.0f),
            FVector(3.0f)
        );
        
        if (Vent)
        {
            ActiveEffects.Add(Vent);
        }
    }
    
    // Apply force to nearby actors
    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(1000.0f);
    
    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Location,
        FQuat::Identity,
        ECC_Pawn,
        SphereShape
    );
    
    for (const FOverlapResult& Overlap : Overlaps)
    {
        if (ACharacter* Character = Cast<ACharacter>(Overlap.GetActor()))
        {
            FVector Direction = (Character->GetActorLocation() - Location).GetSafeNormal();
            Character->LaunchCharacter(Direction * 1000.0f, true, true);
        }
    }
}

void UEmergencyEventManager::UpdateEmergencies(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = ActiveEmergencies.Num() - 1; i >= 0; --i)
    {
        FEmergencyEvent& Event = ActiveEmergencies[i];
        
        if (!Event.bIsActive) continue;
        
        // Check duration
        float ElapsedTime = CurrentTime - Event.StartTime;
        if (ElapsedTime >= Event.Duration)
        {
            if (!Event.bRequiresPlayerAction)
            {
                ResolveEmergency(Event.Type);
            }
            else
            {
                // Escalate if not resolved
                EscalateEmergency(Event.Type);
            }
        }
        
        // Check for auto-escalation
        if (ElapsedTime >= EmergencyEscalationTime && Event.Priority != EEmergencyPriority::Critical)
        {
            EscalateEmergency(Event.Type);
        }
    }
}

void UEmergencyEventManager::StartEmergency(FEmergencyEvent& Event)
{
    Event.bIsActive = true;
    ApplyEmergencyEffects(Event);
    
    // Handle specific emergency types
    switch (Event.Type)
    {
        case EEmergencyType::PowerFailure:
            HandlePowerFailure(Event, true);
            break;
        case EEmergencyType::HullBreach:
            HandleHullBreach(Event, true);
            break;
        case EEmergencyType::FireOutbreak:
            HandleFireOutbreak(Event, true);
            break;
        case EEmergencyType::SystemOverload:
            HandleSystemOverload(Event, true);
            break;
        case EEmergencyType::Lockdown:
            HandleLockdown(Event, true);
            break;
        case EEmergencyType::OxygenLeak:
            HandleOxygenLeak(Event, true);
            break;
        case EEmergencyType::ReactorMeltdown:
            HandleReactorMeltdown(Event, true);
            break;
    }
    
    // Sound appropriate alarm
    if (Event.Priority >= EEmergencyPriority::High)
    {
        SoundAlarm(Event.Type);
    }
}

void UEmergencyEventManager::EndEmergency(FEmergencyEvent& Event)
{
    Event.bIsActive = false;
    RemoveEmergencyEffects(Event);
    
    // Handle specific emergency types
    switch (Event.Type)
    {
        case EEmergencyType::PowerFailure:
            HandlePowerFailure(Event, false);
            break;
        case EEmergencyType::HullBreach:
            HandleHullBreach(Event, false);
            break;
        case EEmergencyType::FireOutbreak:
            HandleFireOutbreak(Event, false);
            break;
        case EEmergencyType::SystemOverload:
            HandleSystemOverload(Event, false);
            break;
        case EEmergencyType::Lockdown:
            HandleLockdown(Event, false);
            break;
        case EEmergencyType::OxygenLeak:
            HandleOxygenLeak(Event, false);
            break;
        case EEmergencyType::ReactorMeltdown:
            HandleReactorMeltdown(Event, false);
            break;
    }
    
    // Stop alarms if no more high priority emergencies
    if (GetHighestPriority() < EEmergencyPriority::High)
    {
        SilenceAlarms();
    }
}

void UEmergencyEventManager::ApplyEmergencyEffects(const FEmergencyEvent& Event)
{
    // Camera shake
    if (EmergencyCameraShake && Event.Priority >= EEmergencyPriority::Medium)
    {
        UGameplayStatics::PlayWorldCameraShake(
            GetWorld(),
            EmergencyCameraShake,
            Event.Location,
            0.0f,
            2000.0f
        );
    }
    
    // Visual effects based on type
    UParticleSystem* EffectToSpawn = nullptr;
    
    switch (Event.Type)
    {
        case EEmergencyType::FireOutbreak:
            EffectToSpawn = FireEffect;
            break;
        case EEmergencyType::HullBreach:
            EffectToSpawn = VentingEffect;
            break;
        case EEmergencyType::SystemOverload:
            EffectToSpawn = SparkEffect;
            break;
        case EEmergencyType::OxygenLeak:
            EffectToSpawn = SmokeEffect;
            break;
    }
    
    if (EffectToSpawn)
    {
        UParticleSystemComponent* Effect = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            EffectToSpawn,
            Event.Location,
            FRotator::ZeroRotator,
            FVector(Event.Severity)
        );
        
        if (Effect)
        {
            ActiveEffects.Add(Effect);
        }
    }
}

void UEmergencyEventManager::RemoveEmergencyEffects(const FEmergencyEvent& Event)
{
    // Effects cleanup happens in CleanupEffects()
}

void UEmergencyEventManager::HandlePowerFailure(const FEmergencyEvent& Event, bool bStart)
{
    if (bStart)
    {
        ActivateEmergencyLighting();
        
        // Flicker lights
        FTimerDelegate FlickerDelegate;
        FlickerDelegate.BindLambda([this]()
        {
            static bool bLightsOn = false;
            bLightsOn = !bLightsOn;
            
            TArray<AActor*> Lights;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALight::StaticClass(), Lights);
            
            for (AActor* LightActor : Lights)
            {
                if (ALight* Light = Cast<ALight>(LightActor))
                {
                    Light->SetActorHiddenInGame(!bLightsOn);
                }
            }
        });
        
        GetWorld()->GetTimerManager().SetTimer(PowerFailureTimerHandle, FlickerDelegate, 0.5f, true);
    }
    else
    {
        DeactivateEmergencyLighting();
        GetWorld()->GetTimerManager().ClearTimer(PowerFailureTimerHandle);
        
        // Restore all lights
        TArray<AActor*> Lights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALight::StaticClass(), Lights);
        
        for (AActor* LightActor : Lights)
        {
            if (ALight* Light = Cast<ALight>(LightActor))
            {
                Light->SetActorHiddenInGame(false);
            }
        }
    }
}

void UEmergencyEventManager::HandleHullBreach(const FEmergencyEvent& Event, bool bStart)
{
    if (bStart)
    {
        // Create suction effect
        VentAtmosphere(Event.Location);
        
        // Damage station integrity
        TArray<AActor*> Actors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
        
        for (AActor* Actor : Actors)
        {
            if (UStationIntegrityComponent* Integrity = Actor->FindComponentByClass<UStationIntegrityComponent>())
            {
                Integrity->ApplyIntegrityDamage(Event.Severity * 5.0f);
                break;
            }
        }
    }
}

void UEmergencyEventManager::HandleFireOutbreak(const FEmergencyEvent& Event, bool bStart)
{
    if (bStart)
    {
        // Spawn fire hazard
        // This would create an actual fire hazard component at the location
    }
    else
    {
        // Activate fire suppression
        ActivateFireSuppression(Event.Location);
    }
}

void UEmergencyEventManager::HandleSystemOverload(const FEmergencyEvent& Event, bool bStart)
{
    if (bStart)
    {
        // Create electrical hazards
        // This would spawn electrical surge hazards
    }
}

void UEmergencyEventManager::HandleLockdown(const FEmergencyEvent& Event, bool bStart)
{
    if (bStart)
    {
        ActivateEmergencyLighting();
        // Lock doors, disable certain systems
    }
    else
    {
        DeactivateEmergencyLighting();
        // Unlock doors, restore systems
    }
}

void UEmergencyEventManager::HandleOxygenLeak(const FEmergencyEvent& Event, bool bStart)
{
    if (bStart)
    {
        // Create toxic hazard at location
        // This would spawn a toxic leak hazard
    }
}

void UEmergencyEventManager::HandleReactorMeltdown(const FEmergencyEvent& Event, bool bStart)
{
    if (bStart)
    {
        // Critical emergency - multiple effects
        ActivateEmergencyLighting();
        SoundAlarm(EEmergencyType::ReactorMeltdown);
        
        // Damage station heavily
        TArray<AActor*> Actors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
        
        for (AActor* Actor : Actors)
        {
            if (UStationIntegrityComponent* Integrity = Actor->FindComponentByClass<UStationIntegrityComponent>())
            {
                Integrity->ApplyIntegrityDamage(Event.Severity * 10.0f);
                break;
            }
        }
    }
}

EEmergencyPriority UEmergencyEventManager::DeterminePriority(EEmergencyType Type, float Severity) const
{
    float PriorityScore = 0.0f;
    
    // Base priority by type
    switch (Type)
    {
        case EEmergencyType::ReactorMeltdown:
            PriorityScore = 4.0f;
            break;
        case EEmergencyType::HullBreach:
            PriorityScore = 3.0f;
            break;
        case EEmergencyType::FireOutbreak:
        case EEmergencyType::OxygenLeak:
            PriorityScore = 2.5f;
            break;
        case EEmergencyType::SystemOverload:
        case EEmergencyType::Lockdown:
            PriorityScore = 2.0f;
            break;
        case EEmergencyType::PowerFailure:
            PriorityScore = 1.0f;
            break;
        default:
            PriorityScore = 1.0f;
            break;
    }
    
    // Modify by severity
    PriorityScore *= Severity;
    
    if (PriorityScore >= 4.0f) return EEmergencyPriority::Critical;
    if (PriorityScore >= 3.0f) return EEmergencyPriority::High;
    if (PriorityScore >= 2.0f) return EEmergencyPriority::Medium;
    
    return EEmergencyPriority::Low;
}

FString UEmergencyEventManager::GenerateEventMessage(EEmergencyType Type, float Severity) const
{
    FString TypeString;
    FString SeverityString;
    
    switch (Type)
    {
        case EEmergencyType::PowerFailure:
            TypeString = "Power Failure";
            break;
        case EEmergencyType::HullBreach:
            TypeString = "Hull Breach Detected";
            break;
        case EEmergencyType::FireOutbreak:
            TypeString = "Fire Outbreak";
            break;
        case EEmergencyType::SystemOverload:
            TypeString = "System Overload";
            break;
        case EEmergencyType::Lockdown:
            TypeString = "Emergency Lockdown";
            break;
        case EEmergencyType::OxygenLeak:
            TypeString = "Oxygen Leak Detected";
            break;
        case EEmergencyType::ReactorMeltdown:
            TypeString = "REACTOR MELTDOWN IMMINENT";
            break;
        default:
            TypeString = "Unknown Emergency";
            break;
    }
    
    if (Severity >= 2.0f)
        SeverityString = "CRITICAL";
    else if (Severity >= 1.5f)
        SeverityString = "Severe";
    else if (Severity >= 1.0f)
        SeverityString = "Moderate";
    else
        SeverityString = "Minor";
    
    return FString::Printf(TEXT("%s - %s"), *TypeString, *SeverityString);
}

void UEmergencyEventManager::CleanupEffects()
{
    for (UParticleSystemComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveEffects.Empty();
}

void UEmergencyEventManager::CleanupAlarms()
{
    for (UAudioComponent* Alarm : ActiveAlarms)
    {
        if (IsValid(Alarm))
        {
            Alarm->Stop();
            Alarm->DestroyComponent();
        }
    }
    ActiveAlarms.Empty();
}