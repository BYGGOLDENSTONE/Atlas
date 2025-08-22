// IntegrityVisualizerComponent.cpp
#include "IntegrityVisualizerComponent.h"
#include "StationIntegrityComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/Light.h"
#include "Components/LightComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"

UIntegrityVisualizerComponent::UIntegrityVisualizerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    CurrentState = EIntegrityState::Normal;
    LastIntegrityPercent = 100.0f;
    CurrentSparksInterval = 0.0f;
    CurrentFlickerInterval = 0.0f;
    bStrobeActive = false;
    bEmergencyLightingActive = false;
}

void UIntegrityVisualizerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache all lights in the scene
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALight::StaticClass(), 
        reinterpret_cast<TArray<AActor*>&>(SceneLights));
}

void UIntegrityVisualizerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up all active effects
    ClearAllEffects();
    
    Super::EndPlay(EndPlayReason);
}

void UIntegrityVisualizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update active hull breaches
    for (FHullBreachData& Breach : ActiveBreaches)
    {
        if (Breach.VacuumEffect)
        {
            // Apply suction force to nearby actors
            ApplySuctionForce(Breach.Location, Breach.Radius, Breach.Severity);
        }
    }
}

void UIntegrityVisualizerComponent::UpdateIntegrityVisuals(float CurrentIntegrity)
{
    float IntegrityPercent = FMath::Clamp(CurrentIntegrity, 0.0f, 100.0f);
    
    // Determine new state
    EIntegrityState NewState = EIntegrityState::Normal;
    
    if (IntegrityPercent <= CriticalThreshold)
    {
        NewState = EIntegrityState::Critical;
    }
    else if (IntegrityPercent <= WarningThreshold)
    {
        NewState = EIntegrityState::Warning;
    }
    else if (IntegrityPercent <= MinorDamageThreshold)
    {
        NewState = EIntegrityState::MinorDamage;
    }
    
    // Handle state change
    if (NewState != CurrentState)
    {
        CurrentState = NewState;
        OnIntegrityStateChanged(CurrentState, IntegrityPercent);
        
        // Apply state-specific effects
        switch (CurrentState)
        {
            case EIntegrityState::Critical:
                HandleCriticalState();
                break;
            case EIntegrityState::Warning:
                HandleWarningState();
                break;
            case EIntegrityState::MinorDamage:
                HandleMinorDamageState();
                break;
            case EIntegrityState::Normal:
                HandleNormalState();
                break;
        }
    }
    
    // Update continuous effects based on integrity
    SpawnDamageEffects(IntegrityPercent);
    UpdateLightingState(IntegrityPercent);
    PlayIntegrityAudio(IntegrityPercent);
    
    LastIntegrityPercent = IntegrityPercent;
}

void UIntegrityVisualizerComponent::HandleCriticalState()
{
    // Red emergency lighting
    SetAllLightsColor(FLinearColor::Red);
    SetLightIntensity(0.5f);
    
    // Strobe effect
    EnableStrobeLighting(2.0f); // 2Hz strobe
    
    // Maximum particle effects
    SpawnSparksAtInterval(0.5f);
    SpawnSteamLeaks(10);
    
    // Audio
    PlayAlarmLoop(ESeverity::Critical);
    PlayCreakingSound(1.0f);
    
    // Screen effects
    ApplyCameraShake(ECameraShakeIntensity::Heavy);
    ApplyScreenDistortion(0.3f);
    
    // Hull breach effects
    CreateHullBreachPoints(3);
}

void UIntegrityVisualizerComponent::HandleWarningState()
{
    // Orange emergency lighting
    SetAllLightsColor(FLinearColor(1.0f, 0.5f, 0.0f));
    SetLightIntensity(0.7f);
    
    // Occasional flicker
    EnableLightFlicker(5.0f); // Flicker every 5 seconds
    
    // Moderate particle effects
    SpawnSparksAtInterval(2.0f);
    SpawnSteamLeaks(5);
    
    // Audio
    PlayAlarmLoop(ESeverity::Medium);
    PlayCreakingSound(0.5f);
    
    // Screen effects
    ApplyCameraShake(ECameraShakeIntensity::Medium);
}

void UIntegrityVisualizerComponent::HandleMinorDamageState()
{
    // Yellow lighting tint
    SetAllLightsColor(FLinearColor(1.0f, 0.9f, 0.7f));
    SetLightIntensity(0.85f);
    
    // Rare flicker
    EnableLightFlicker(10.0f);
    
    // Minimal particle effects
    SpawnSparksAtInterval(5.0f);
    SpawnSteamLeaks(2);
    
    // Audio
    PlayAmbientCreaking(0.25f);
    
    // Screen effects
    ApplyCameraShake(ECameraShakeIntensity::Light);
}

void UIntegrityVisualizerComponent::HandleNormalState()
{
    // Reset to normal lighting
    DisableAllLightingEffects();
    
    // Stop all effects
    GetWorld()->GetTimerManager().ClearTimer(SparksTimerHandle);
    StopAllAudio();
    
    // Clear any active screen effects
    ApplyScreenDistortion(0.0f);
}

void UIntegrityVisualizerComponent::TriggerHullBreach(FVector Location)
{
    FHullBreachData Breach;
    Breach.Location = Location;
    Breach.Severity = FMath::FRandRange(0.5f, 1.0f);
    Breach.Radius = 500.0f;
    
    // Spawn vacuum effect
    if (HullBreachEffect)
    {
        Breach.VacuumEffect = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), HullBreachEffect, Location, FRotator::ZeroRotator, true);
    }
    
    // Spawn vacuum sound
    if (VacuumSound)
    {
        Breach.VacuumSound = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(), VacuumSound, Location);
    }
    
    // Apply initial impulse to nearby actors
    ApplySuctionForce(Location, Breach.Radius, Breach.Severity * 2.0f);
    
    // Spawn debris
    SpawnDebris(Location, Breach.Severity);
    
    // Add to active breaches
    ActiveBreaches.Add(Breach);
    
    // Notify Blueprint
    OnHullBreachTriggered(Location);
    
    // Camera shake
    ApplyCameraShake(ECameraShakeIntensity::Heavy);
}

void UIntegrityVisualizerComponent::TriggerElectricalFailure(FVector Location)
{
    // Spawn electrical arc effect
    if (ElectricalArcEffect)
    {
        UParticleSystemComponent* ElectricalParticle = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), ElectricalArcEffect, Location, FRotator::ZeroRotator, true);
        ActiveParticles.Add(ElectricalParticle);
    }
    
    // Play electrical sound
    if (ElectricalSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ElectricalSound, Location);
    }
    
    // Flicker nearby lights
    for (ALight* Light : SceneLights)
    {
        if (Light && FVector::Dist(Light->GetActorLocation(), Location) < 1000.0f)
        {
            if (ULightComponent* LightComp = Light->GetLightComponent())
            {
                // Random flicker pattern
                FTimerHandle FlickerHandle;
                GetWorld()->GetTimerManager().SetTimer(FlickerHandle, [LightComp]()
                {
                    if (IsValid(LightComp))
                    {
                        LightComp->SetIntensity(FMath::FRandRange(0.1f, 1.0f));
                    }
                }, 0.1f, true, 0.0f);
                
                // Stop flickering after a few seconds
                FTimerHandle StopFlickerHandle;
                FTimerDelegate StopFlickerDel;
                StopFlickerDel.BindLambda([this, FlickerHandle]() mutable
                {
                    GetWorld()->GetTimerManager().ClearTimer(FlickerHandle);
                });
                GetWorld()->GetTimerManager().SetTimer(StopFlickerHandle, StopFlickerDel, FMath::FRandRange(2.0f, 5.0f), false);
            }
        }
    }
    
    // Notify Blueprint
    OnElectricalFailureTriggered(Location);
}

void UIntegrityVisualizerComponent::TriggerEmergencyLighting()
{
    bEmergencyLightingActive = true;
    
    // Set all lights to emergency red
    SetAllLightsColor(FLinearColor(1.0f, 0.0f, 0.0f));
    SetLightIntensity(0.3f);
    
    // Enable slow strobe for emergency effect
    EnableStrobeLighting(0.5f);
    
    // Play alarm
    PlayAlarmLoop(ESeverity::Critical);
}

void UIntegrityVisualizerComponent::ClearAllEffects()
{
    // Clear timers
    GetWorld()->GetTimerManager().ClearTimer(SparksTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(FlickerTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(StrobeTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(CreakingTimerHandle);
    
    // Destroy active particles
    for (UParticleSystemComponent* Particle : ActiveParticles)
    {
        if (IsValid(Particle))
        {
            Particle->DestroyComponent();
        }
    }
    ActiveParticles.Empty();
    
    // Destroy breach effects
    for (FHullBreachData& Breach : ActiveBreaches)
    {
        if (IsValid(Breach.VacuumEffect))
        {
            Breach.VacuumEffect->DestroyComponent();
        }
        if (IsValid(Breach.VacuumSound))
        {
            Breach.VacuumSound->Stop();
            Breach.VacuumSound->DestroyComponent();
        }
    }
    ActiveBreaches.Empty();
    
    // Stop all audio
    StopAllAudio();
    
    // Reset lighting
    DisableAllLightingEffects();
}

void UIntegrityVisualizerComponent::SpawnDamageEffects(float IntegrityPercent)
{
    // Spawn effects based on integrity level
    float EffectIntensity = 1.0f - (IntegrityPercent / 100.0f);
    
    // Random spark spawning
    if (SparksEffect && FMath::FRand() < EffectIntensity * 0.1f)
    {
        FVector RandomLocation = GetOwner()->GetActorLocation() + 
            FMath::VRand() * 500.0f;
        
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), SparksEffect, RandomLocation);
    }
}

void UIntegrityVisualizerComponent::UpdateLightingState(float IntegrityPercent)
{
    // Update light colors and intensity based on integrity
    float IntensityMultiplier = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, 100.0f), FVector2D(0.3f, 1.0f), IntegrityPercent);
    
    for (ALight* Light : SceneLights)
    {
        if (Light && !bEmergencyLightingActive)
        {
            if (ULightComponent* LightComp = Light->GetLightComponent())
            {
                LightComp->SetIntensity(LightComp->Intensity * IntensityMultiplier);
            }
        }
    }
}

void UIntegrityVisualizerComponent::PlayIntegrityAudio(float IntegrityPercent)
{
    // Play ambient sounds based on integrity
    if (IntegrityPercent < 50.0f && !CreakingAudioComponent)
    {
        float Volume = FMath::GetMappedRangeValueClamped(
            FVector2D(0.0f, 50.0f), FVector2D(1.0f, 0.2f), IntegrityPercent);
        PlayCreakingSound(Volume);
    }
}

void UIntegrityVisualizerComponent::SetAllLightsColor(const FLinearColor& Color)
{
    for (ALight* Light : SceneLights)
    {
        if (Light)
        {
            if (ULightComponent* LightComp = Light->GetLightComponent())
            {
                LightComp->SetLightColor(Color);
            }
        }
    }
}

void UIntegrityVisualizerComponent::SetLightIntensity(float Intensity)
{
    for (ALight* Light : SceneLights)
    {
        if (Light)
        {
            if (ULightComponent* LightComp = Light->GetLightComponent())
            {
                LightComp->SetIntensity(Intensity);
            }
        }
    }
}

void UIntegrityVisualizerComponent::EnableStrobeLighting(float Frequency)
{
    if (bStrobeActive) return;
    
    bStrobeActive = true;
    
    GetWorld()->GetTimerManager().SetTimer(StrobeTimerHandle, [this]()
    {
        static bool bLightsOn = true;
        bLightsOn = !bLightsOn;
        
        for (ALight* Light : SceneLights)
        {
            if (Light)
            {
                if (ULightComponent* LightComp = Light->GetLightComponent())
                {
                    LightComp->SetVisibility(bLightsOn);
                }
            }
        }
    }, 1.0f / Frequency, true);
}

void UIntegrityVisualizerComponent::EnableLightFlicker(float Interval)
{
    GetWorld()->GetTimerManager().SetTimer(FlickerTimerHandle, [this]()
    {
        // Randomly flicker a light
        if (SceneLights.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, SceneLights.Num() - 1);
            if (SceneLights[RandomIndex])
            {
                if (ULightComponent* LightComp = SceneLights[RandomIndex]->GetLightComponent())
                {
                    // Quick flicker effect
                    float OriginalIntensity = LightComp->Intensity;
                    LightComp->SetIntensity(OriginalIntensity * 0.1f);
                    
                    // Simple timer to restore light intensity
                    FTimerDelegate TimerDel;
                    TimerDel.BindLambda([LightComp, OriginalIntensity]()
                    {
                        if (IsValid(LightComp))
                        {
                            LightComp->SetIntensity(OriginalIntensity);
                        }
                    });
                    FTimerHandle RestoreHandle;
                    GetWorld()->GetTimerManager().SetTimer(RestoreHandle, TimerDel, 0.1f, false);
                }
            }
        }
    }, Interval, true);
}

void UIntegrityVisualizerComponent::DisableAllLightingEffects()
{
    bStrobeActive = false;
    bEmergencyLightingActive = false;
    
    GetWorld()->GetTimerManager().ClearTimer(StrobeTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(FlickerTimerHandle);
    
    // Reset all lights
    for (ALight* Light : SceneLights)
    {
        if (Light)
        {
            if (ULightComponent* LightComp = Light->GetLightComponent())
            {
                LightComp->SetVisibility(true);
                LightComp->SetLightColor(FLinearColor::White);
                LightComp->SetIntensity(1.0f);
            }
        }
    }
}

void UIntegrityVisualizerComponent::SpawnSparksAtInterval(float Interval)
{
    if (Interval != CurrentSparksInterval)
    {
        CurrentSparksInterval = Interval;
        
        GetWorld()->GetTimerManager().SetTimer(SparksTimerHandle, [this]()
        {
            if (SparksEffect)
            {
                FVector RandomLocation = GetOwner()->GetActorLocation() + 
                    FMath::VRand() * 800.0f;
                
                UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(), SparksEffect, RandomLocation);
            }
        }, Interval, true);
    }
}

void UIntegrityVisualizerComponent::SpawnSteamLeaks(int32 Count)
{
    if (!SteamLeakEffect) return;
    
    for (int32 i = 0; i < Count; ++i)
    {
        FVector RandomLocation = GetOwner()->GetActorLocation() + 
            FMath::VRand() * 1000.0f;
        
        UParticleSystemComponent* Steam = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), SteamLeakEffect, RandomLocation, FRotator::ZeroRotator, true);
        
        if (Steam)
        {
            ActiveParticles.Add(Steam);
        }
    }
}

void UIntegrityVisualizerComponent::CreateHullBreachPoints(int32 Count)
{
    for (int32 i = 0; i < Count; ++i)
    {
        FVector RandomLocation = GetOwner()->GetActorLocation() + 
            FMath::VRand() * 1500.0f;
        
        TriggerHullBreach(RandomLocation);
    }
}

void UIntegrityVisualizerComponent::PlayAlarmLoop(ESeverity Severity)
{
    if (AlarmSound && !AlarmAudioComponent)
    {
        AlarmAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(), AlarmSound, GetOwner()->GetActorLocation());
        
        if (AlarmAudioComponent)
        {
            ActiveAudioComponents.Add(AlarmAudioComponent);
            
            // Adjust volume based on severity
            float Volume = Severity == ESeverity::Critical ? 1.0f : 
                          Severity == ESeverity::Heavy ? 0.7f : 0.5f;
            AlarmAudioComponent->SetVolumeMultiplier(Volume);
        }
    }
}

void UIntegrityVisualizerComponent::PlayCreakingSound(float Volume)
{
    if (CreakingSound && !CreakingAudioComponent)
    {
        CreakingAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(), CreakingSound, GetOwner()->GetActorLocation());
        
        if (CreakingAudioComponent)
        {
            ActiveAudioComponents.Add(CreakingAudioComponent);
            CreakingAudioComponent->SetVolumeMultiplier(Volume);
        }
    }
}

void UIntegrityVisualizerComponent::PlayAmbientCreaking(float Volume)
{
    GetWorld()->GetTimerManager().SetTimer(CreakingTimerHandle, [this, Volume]()
    {
        if (CreakingSound)
        {
            FVector RandomLocation = GetOwner()->GetActorLocation() + 
                FMath::VRand() * 2000.0f;
            
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(), CreakingSound, RandomLocation, Volume);
        }
    }, FMath::FRandRange(5.0f, 15.0f), true);
}

void UIntegrityVisualizerComponent::StopAllAudio()
{
    for (UAudioComponent* Audio : ActiveAudioComponents)
    {
        if (IsValid(Audio))
        {
            Audio->Stop();
            Audio->DestroyComponent();
        }
    }
    ActiveAudioComponents.Empty();
    
    AlarmAudioComponent = nullptr;
    CreakingAudioComponent = nullptr;
    
    GetWorld()->GetTimerManager().ClearTimer(CreakingTimerHandle);
}

void UIntegrityVisualizerComponent::ApplyCameraShake(ECameraShakeIntensity Intensity)
{
    if (Intensity == ECameraShakeIntensity::None) return;
    
    TSubclassOf<UCameraShakeBase> ShakeClass = nullptr;
    
    switch (Intensity)
    {
        case ECameraShakeIntensity::Light:
            ShakeClass = LightShake;
            break;
        case ECameraShakeIntensity::Medium:
            ShakeClass = MediumShake;
            break;
        case ECameraShakeIntensity::Heavy:
            ShakeClass = HeavyShake;
            break;
    }
    
    if (ShakeClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            PC->ClientStartCameraShake(ShakeClass);
        }
    }
}

void UIntegrityVisualizerComponent::ApplyScreenDistortion(float Intensity)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->PlayerCameraManager)
    {
        // Use camera fade for distortion effect instead of post-process
        if (Intensity > 0.0f)
        {
            PC->PlayerCameraManager->StartCameraFade(0.0f, Intensity * 0.3f, 0.5f, FLinearColor(1.0f, 0.0f, 0.0f, 1.0f), false, true);
        }
        else
        {
            PC->PlayerCameraManager->StopCameraFade();
        }
    }
}

void UIntegrityVisualizerComponent::ApplySuctionForce(const FVector& Location, float Radius, float Severity)
{
    // Find all actors in range
    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
    
    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Location,
        FQuat::Identity,
        ECC_WorldDynamic,
        SphereShape
    );
    
    for (const FOverlapResult& Overlap : Overlaps)
    {
        if (AActor* Actor = Overlap.GetActor())
        {
            FVector ToCenter = Location - Actor->GetActorLocation();
            float Distance = ToCenter.Size();
            
            if (Distance > 10.0f && Distance < Radius)
            {
                ToCenter.Normalize();
                
                // Apply force based on distance
                float ForceMultiplier = 1.0f - (Distance / Radius);
                float Force = 1000.0f * Severity * ForceMultiplier;
                
                // Apply to characters
                if (ACharacter* Character = Cast<ACharacter>(Actor))
                {
                    Character->LaunchCharacter(ToCenter * Force, false, false);
                }
                // Apply to physics objects
                else if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? 
                    Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr)
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        PrimComp->AddImpulse(ToCenter * Force * PrimComp->GetMass());
                    }
                }
            }
        }
    }
}

void UIntegrityVisualizerComponent::SpawnDebris(const FVector& Location, float Severity)
{
    if (!DebrisEffect) return;
    
    int32 DebrisCount = FMath::RandRange(3, 8) * Severity;
    
    for (int32 i = 0; i < DebrisCount; ++i)
    {
        FVector RandomOffset = FMath::VRand() * 100.0f;
        FVector RandomVelocity = FMath::VRand() * 500.0f * Severity;
        RandomVelocity.Z = FMath::Abs(RandomVelocity.Z); // Ensure upward motion
        
        UParticleSystemComponent* Debris = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), 
            DebrisEffect, 
            Location + RandomOffset,
            FRotator::ZeroRotator,
            FVector(1.0f),
            true
        );
        
        if (Debris)
        {
            Debris->SetVectorParameter(FName("InitialVelocity"), RandomVelocity);
        }
    }
}

UParticleSystemComponent* UIntegrityVisualizerComponent::SpawnVacuumParticles(const FVector& Location)
{
    if (HullBreachEffect)
    {
        return UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), 
            HullBreachEffect, 
            Location,
            FRotator::ZeroRotator,
            true
        );
    }
    
    return nullptr;
}