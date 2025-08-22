// ToxicLeakHazard.cpp
#include "ToxicLeakHazard.h"
#include "../Characters/GameCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/DamageType.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/DamageEvents.h"
#include "Engine/EngineTypes.h"
#include "Components/SphereComponent.h"
#include "../Interfaces/IHealthInterface.h"

// Custom damage type for toxic damage

UToxicLeakHazard::UToxicLeakHazard()
{
    HazardType = EHazardType::Toxic;
    DamagePerSecond = 8.0f;
    HazardRadius = 500.0f;
    
    // Set toxic effect color (green)
    EffectData.HazardColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);
    
    DamageTypeClass = UDamageType::StaticClass();
    
    CurrentCloudRadius = HazardRadius;
}

void UToxicLeakHazard::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentCloudRadius = HazardRadius;
}

void UToxicLeakHazard::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsActive) return;
    
    // Update poison DOTs
    UpdatePoisonDOTs(DeltaTime);
    
    // Expand toxic cloud over time
    ExpandToxicCloud(DeltaTime);
}

void UToxicLeakHazard::ApplyHazardEffect(AActor* Actor, float DeltaTime)
{
    if (!Actor) return;
    
    // Apply base toxic damage
    if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
    {
        // Apply damage
        if (Character->GetClass()->ImplementsInterface(UHealthInterface::StaticClass()))
        {
            IHealthInterface::Execute_ApplyDamage(Character, DamagePerSecond * DeltaTime, GetOwner());
        }
        else
        {
            // Use APawn's TakeDamage explicitly to avoid ambiguity
            Character->APawn::TakeDamage(DamagePerSecond * DeltaTime, FDamageEvent(UDamageType::StaticClass()), nullptr, GetOwner());
        }
        
        // Apply vision impairment to players
        if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
        {
            ApplyToxicScreenEffect(PC, VisionImpairment);
        }
        
        // Apply movement slow (already handled in OnActorEnterHazard)
        
        // Apply or refresh poison DOT
        ApplyPoisonDOT(Character);
        
        // Play coughing sound occasionally
        if (FMath::FRand() < 0.05f) // 5% chance per tick
        {
            if (CoughingSound)
            {
                UGameplayStatics::SpawnSoundAttached(
                    CoughingSound,
                    Character->GetRootComponent(),
                    NAME_None,
                    FVector::ZeroVector,
                    EAttachLocation::KeepRelativeOffset,
                    false,
                    1.0f,
                    1.0f,
                    0.0f,
                    nullptr,
                    nullptr,
                    true
                );
            }
        }
    }
}

void UToxicLeakHazard::UpdateHazardVisuals(float DeltaTime)
{
    Super::UpdateHazardVisuals(DeltaTime);
    
    // Update toxic cloud density based on duration
    if (ActiveHazardEffect)
    {
        float CloudDensity = FMath::Clamp(CurrentDuration / 10.0f, 0.3f, 1.0f);
        ActiveHazardEffect->SetFloatParameter(FName("CloudDensity"), CloudDensity);
        
        // Update cloud color intensity
        FLinearColor CloudColor = EffectData.HazardColor;
        CloudColor.A = CloudDensity * 0.7f;
        ActiveHazardEffect->SetColorParameter(FName("CloudColor"), CloudColor);
    }
    
    // Spawn additional cloud particles as it spreads
    if (ToxicCloudEffect && FMath::FRand() < 0.2f)
    {
        FVector RandomOffset = FMath::VRand() * CurrentCloudRadius;
        RandomOffset.Z = 0; // Keep at ground level
        
        UParticleSystemComponent* CloudParticle = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ToxicCloudEffect,
            GetComponentLocation() + RandomOffset,
            FRotator::ZeroRotator,
            FVector(1.0f)
        );
        
        if (CloudParticle)
        {
            // Auto-destroy after 5 seconds
            FTimerHandle DestroyTimer;
            GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [CloudParticle]()
            {
                if (IsValid(CloudParticle))
                {
                    CloudParticle->DestroyComponent();
                }
            }, 5.0f, false);
            ToxicCloudParticles.Add(CloudParticle);
        }
    }
}

void UToxicLeakHazard::OnActorEnterHazard_Implementation(AActor* Actor)
{
    Super::OnActorEnterHazard_Implementation(Actor);
    
    if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
    {
        // Store original movement speed and apply slow
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            if (!OriginalMovementSpeeds.Contains(Actor))
            {
                OriginalMovementSpeeds.Add(Actor, MovementComp->MaxWalkSpeed);
            }
            
            float NewSpeed = MovementComp->MaxWalkSpeed * (1.0f - MovementSlowPercent);
            MovementComp->MaxWalkSpeed = NewSpeed;
        }
        
        // Apply initial poison
        ApplyPoisonDOT(Actor);
    }
}

void UToxicLeakHazard::OnActorExitHazard_Implementation(AActor* Actor)
{
    Super::OnActorExitHazard_Implementation(Actor);
    
    if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
    {
        // Restore original movement speed
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            if (float* OriginalSpeed = OriginalMovementSpeeds.Find(Actor))
            {
                MovementComp->MaxWalkSpeed = *OriginalSpeed;
                OriginalMovementSpeeds.Remove(Actor);
            }
        }
        
        // Remove vision impairment
        if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
        {
            RemoveToxicScreenEffect(PC);
        }
        
        // Note: DOT continues even after leaving the hazard area
    }
}

void UToxicLeakHazard::ApplyPoisonDOT(AActor* Target)
{
    if (!Target) return;
    
    // Check if target already has a DOT
    for (FPoisonDOTData& DOT : ActiveDOTs)
    {
        if (DOT.Target == Target)
        {
            // Refresh the duration
            DOT.TimeRemaining = DOTDuration;
            return;
        }
    }
    
    // Add new DOT
    FPoisonDOTData NewDOT;
    NewDOT.Target = Target;
    NewDOT.Duration = DOTDuration;
    NewDOT.DamagePerSecond = DOTDamagePerSecond;
    NewDOT.TimeRemaining = DOTDuration;
    
    ActiveDOTs.Add(NewDOT);
    
    // Fire event
    OnPoisonApplied(Target, DOTDuration, DOTDamagePerSecond);
}

void UToxicLeakHazard::ApplyToxicScreenEffect(APlayerController* PC, float Intensity)
{
    if (!PC || !PC->PlayerCameraManager) return;
    
    // Apply green tint overlay using camera fade
    // Note: Post-process settings require additional setup, using fade for now
    PC->PlayerCameraManager->StartCameraFade(
        0.0f,
        Intensity,
        0.5f,  // Fade in time
        FLinearColor(0.0f, 1.0f, 0.0f, 1.0f),  // Green color
        false,
        true   // Hold when finished
    );
}

void UToxicLeakHazard::RemoveToxicScreenEffect(APlayerController* PC)
{
    if (!PC || !PC->PlayerCameraManager) return;
    
    // Stop camera fade
    PC->PlayerCameraManager->StopCameraFade();
}

void UToxicLeakHazard::UpdatePoisonDOTs(float DeltaTime)
{
    // Update all active DOTs
    for (int32 i = ActiveDOTs.Num() - 1; i >= 0; --i)
    {
        FPoisonDOTData& DOT = ActiveDOTs[i];
        
        if (!IsValid(DOT.Target))
        {
            ActiveDOTs.RemoveAt(i);
            continue;
        }
        
        // Apply DOT damage
        // Apply DOT damage
        if (DOT.Target->GetClass()->ImplementsInterface(UHealthInterface::StaticClass()))
        {
            IHealthInterface::Execute_ApplyDamage(DOT.Target, DOT.DamagePerSecond * DeltaTime, GetOwner());
        }
        else
        {
            // Use AActor's TakeDamage for generic actors
            DOT.Target->AActor::TakeDamage(DOT.DamagePerSecond * DeltaTime, FDamageEvent(UDamageType::StaticClass()), nullptr, GetOwner());
        }
        
        // Update remaining time
        DOT.TimeRemaining -= DeltaTime;
        
        // Remove expired DOTs
        if (DOT.TimeRemaining <= 0.0f)
        {
            ActiveDOTs.RemoveAt(i);
        }
    }
}

void UToxicLeakHazard::ExpandToxicCloud(float DeltaTime)
{
    // Gradually expand the toxic cloud radius
    if (CurrentCloudRadius < HazardRadius + ToxicCloudSpreadRadius)
    {
        CurrentCloudRadius += ToxicCloudSpreadRate * DeltaTime;
        
        // Update trigger sphere radius
        if (HazardTriggerSphere)
        {
            HazardTriggerSphere->SetSphereRadius(CurrentCloudRadius);
        }
    }
    
    // Clean up old cloud particles
    for (int32 i = ToxicCloudParticles.Num() - 1; i >= 0; --i)
    {
        if (!IsValid(ToxicCloudParticles[i]))
        {
            ToxicCloudParticles.RemoveAt(i);
        }
    }
}