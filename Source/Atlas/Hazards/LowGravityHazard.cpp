// LowGravityHazard.cpp
#include "LowGravityHazard.h"
#include "../Characters/GameCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/EngineTypes.h"

ULowGravityHazard::ULowGravityHazard()
{
    HazardType = EHazardType::Gravity;
    DamagePerSecond = 0.0f; // No damage for gravity hazard
    HazardRadius = 600.0f;
    
    // Set gravity effect color (light blue/purple)
    EffectData.HazardColor = FLinearColor(0.5f, 0.3f, 1.0f, 0.3f);
    
    DebrisSpawnTimer = 0.0f;
}

void ULowGravityHazard::BeginPlay()
{
    Super::BeginPlay();
    
    // Start ambient sound
    if (AntiGravityHumSound && bIsActive)
    {
        AmbientHumSound = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            AntiGravityHumSound,
            GetComponentLocation(),
            FRotator::ZeroRotator,
            1.0f,
            1.0f,
            0.0f,
            nullptr,
            nullptr,
            true
        );
    }
}

void ULowGravityHazard::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsActive) return;
    
    // Update floating objects
    UpdateFloatingObjects(DeltaTime);
    
    // Spawn floating debris periodically
    DebrisSpawnTimer += DeltaTime;
    if (DebrisSpawnTimer >= 3.0f)
    {
        CreateFloatingDebris();
        DebrisSpawnTimer = 0.0f;
    }
}

void ULowGravityHazard::ApplyHazardEffect(AActor* Actor, float DeltaTime)
{
    // Low gravity doesn't deal damage, just affects movement
    // The gravity changes are handled in OnActorEnterHazard
    
    if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
    {
        // Apply slight upward force for floating effect
        if (Character->GetCharacterMovement()->IsFalling())
        {
            FVector UpwardForce = FVector(0, 0, FloatingForce * DeltaTime);
            Character->GetCharacterMovement()->AddImpulse(UpwardForce, true);
        }
        
        // Spawn floating particles around character
        if (FMath::FRand() < 0.1f) // 10% chance per tick
        {
            SpawnAntiGravityParticles(Character);
        }
    }
}

void ULowGravityHazard::UpdateHazardVisuals(float DeltaTime)
{
    Super::UpdateHazardVisuals(DeltaTime);
    
    // Update gravity distortion effect
    if (ActiveHazardEffect)
    {
        // Create a pulsing effect
        float PulseValue = FMath::Sin(GetWorld()->GetTimeSeconds() * 1.5f) * 0.5f + 0.5f;
        ActiveHazardEffect->SetFloatParameter(FName("DistortionIntensity"), PulseValue);
        
        // Rotate the effect slowly
        FRotator CurrentRotation = ActiveHazardEffect->GetRelativeRotation();
        CurrentRotation.Yaw += 10.0f * DeltaTime;
        ActiveHazardEffect->SetRelativeRotation(CurrentRotation);
    }
    
    // Update ambient sound volume based on number of affected actors
    if (AmbientHumSound)
    {
        float Volume = FMath::Clamp(0.3f + (AffectedActors.Num() * 0.1f), 0.3f, 1.0f);
        AmbientHumSound->SetVolumeMultiplier(Volume);
    }
}

void ULowGravityHazard::OnActorEnterHazard_Implementation(AActor* Actor)
{
    Super::OnActorEnterHazard_Implementation(Actor);
    
    if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
    {
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        if (MovementComp)
        {
            // Store original values
            FOriginalMovementValues OriginalValues;
            OriginalValues.GravityScale = MovementComp->GravityScale;
            OriginalValues.JumpZVelocity = MovementComp->JumpZVelocity;
            OriginalValues.AirControl = MovementComp->AirControl;
            OriginalValues.FallingLateralFriction = MovementComp->FallingLateralFriction;
            
            OriginalCharacterValues.Add(Character, OriginalValues);
            
            // Apply low gravity effects
            MovementComp->GravityScale = GravityScale;
            MovementComp->JumpZVelocity = OriginalValues.JumpZVelocity * JumpBoostMultiplier;
            MovementComp->AirControl = FMath::Min(AirControlBoost, 1.0f);
            MovementComp->FallingLateralFriction = 0.5f; // Add some friction for control
            
            // Give a small upward impulse on entry
            Character->LaunchCharacter(FVector(0, 0, 200), false, false);
            
            // Spawn entry effect
            SpawnAntiGravityParticles(Character);
        }
        
        OnActorEnteredLowGravity(Actor);
    }
    // Handle physics objects
    else if (bAffectsPhysicsObjects)
    {
        if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? 
            Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr)
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                ApplyAntiGravityToPhysicsObject(PrimComp);
            }
        }
    }
}

void ULowGravityHazard::OnActorExitHazard_Implementation(AActor* Actor)
{
    Super::OnActorExitHazard_Implementation(Actor);
    
    if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
    {
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        if (MovementComp)
        {
            // Restore original values
            if (FOriginalMovementValues* OriginalValues = OriginalCharacterValues.Find(Character))
            {
                MovementComp->GravityScale = OriginalValues->GravityScale;
                MovementComp->JumpZVelocity = OriginalValues->JumpZVelocity;
                MovementComp->AirControl = OriginalValues->AirControl;
                MovementComp->FallingLateralFriction = OriginalValues->FallingLateralFriction;
                
                OriginalCharacterValues.Remove(Character);
            }
        }
        
        OnActorExitedLowGravity(Actor);
    }
    // Handle physics objects
    else if (bAffectsPhysicsObjects)
    {
        if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? 
            Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr)
        {
            if (PrimComp->IsSimulatingPhysics())
            {
                RestoreGravityToPhysicsObject(PrimComp);
            }
        }
    }
}

void ULowGravityHazard::ApplyAntiGravityToPhysicsObject(UPrimitiveComponent* Component)
{
    if (!Component || !Component->IsSimulatingPhysics()) return;
    
    // Store original gravity
    if (!OriginalPhysicsGravity.Contains(Component))
    {
        OriginalPhysicsGravity.Add(Component, Component->IsGravityEnabled() ? 1.0f : 0.0f);
    }
    
    // Apply reduced gravity
    Component->SetEnableGravity(true);
    // Component->BodyInstance.bOverrideWalkableSlopeOnInstance = true; // This is protected, skip it
    
    // Add to floating objects list
    FloatingObjects.AddUnique(Component);
    
    // Give initial upward impulse
    FVector Impulse = FVector(0, 0, Component->GetMass() * 200.0f);
    Component->AddImpulse(Impulse);
    
    // Add some random angular velocity for visual interest
    FVector AngularImpulse = FMath::VRand() * 100.0f;
    Component->AddAngularImpulseInRadians(AngularImpulse);
}

void ULowGravityHazard::RestoreGravityToPhysicsObject(UPrimitiveComponent* Component)
{
    if (!Component) return;
    
    // Restore original gravity setting
    if (float* OriginalGravity = OriginalPhysicsGravity.Find(Component))
    {
        Component->SetEnableGravity(*OriginalGravity > 0.0f);
        OriginalPhysicsGravity.Remove(Component);
    }
    
    // Remove from floating objects
    FloatingObjects.Remove(Component);
}

void ULowGravityHazard::SpawnAntiGravityParticles(AActor* Target)
{
    if (!AntiGravityParticles || !Target) return;
    
    UParticleSystemComponent* Particles = UGameplayStatics::SpawnEmitterAttached(
        AntiGravityParticles,
        Target->GetRootComponent(),
        NAME_None,
        FVector(0, 0, -50), // Spawn at feet
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        false
    );
    
    if (Particles)
    {
        Particles->SetColorParameter(FName("GravityColor"), EffectData.HazardColor);
        // Auto-destroy after 2 seconds
        FTimerHandle DestroyTimer;
        GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [Particles]()
        {
            if (IsValid(Particles))
            {
                Particles->DestroyComponent();
            }
        }, 2.0f, false);
        ActiveAntiGravityEffects.Add(Particles);
    }
}

void ULowGravityHazard::UpdateFloatingObjects(float DeltaTime)
{
    // Apply upward force to floating physics objects
    for (int32 i = FloatingObjects.Num() - 1; i >= 0; --i)
    {
        UPrimitiveComponent* Component = FloatingObjects[i];
        
        if (!IsValid(Component))
        {
            FloatingObjects.RemoveAt(i);
            continue;
        }
        
        // Check if still in range
        float Distance = FVector::Dist(Component->GetComponentLocation(), GetComponentLocation());
        if (Distance > HazardRadius)
        {
            RestoreGravityToPhysicsObject(Component);
            continue;
        }
        
        // Apply anti-gravity force
        FVector AntiGravity = FVector(0, 0, Component->GetMass() * 980.0f * (1.0f - PhysicsObjectGravityScale));
        Component->AddForce(AntiGravity);
        
        // Add slight floating motion
        float FloatOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f + i) * 10.0f;
        Component->AddForce(FVector(0, 0, FloatOffset * Component->GetMass()));
    }
    
    // Clean up expired particle effects
    for (int32 i = ActiveAntiGravityEffects.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(ActiveAntiGravityEffects[i]))
        {
            ActiveAntiGravityEffects.RemoveAt(i);
        }
    }
}

void ULowGravityHazard::CreateFloatingDebris()
{
    if (!FloatingDebrisEffect) return;
    
    // Spawn random floating debris particles
    int32 DebrisCount = FMath::RandRange(3, 8);
    
    for (int32 i = 0; i < DebrisCount; ++i)
    {
        FVector RandomLocation = GetComponentLocation() + FMath::VRand() * HazardRadius * 0.8f;
        RandomLocation.Z = GetComponentLocation().Z + FMath::FRandRange(-100.0f, 200.0f);
        
        UParticleSystemComponent* Debris = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            FloatingDebrisEffect,
            RandomLocation,
            FRotator::ZeroRotator,
            FVector(FMath::FRandRange(0.5f, 1.5f))
        );
        
        if (Debris)
        {
            // Set floating velocity
            FVector FloatVelocity = FVector(
                FMath::FRandRange(-50.0f, 50.0f),
                FMath::FRandRange(-50.0f, 50.0f),
                FMath::FRandRange(10.0f, 50.0f)
            );
            Debris->SetVectorParameter(FName("FloatVelocity"), FloatVelocity);
            // Auto-destroy after 10 seconds
            FTimerHandle DestroyTimer2;
            GetWorld()->GetTimerManager().SetTimer(DestroyTimer2, [Debris]()
            {
                if (IsValid(Debris))
                {
                    Debris->DestroyComponent();
                }
            }, 10.0f, false);
        }
    }
}