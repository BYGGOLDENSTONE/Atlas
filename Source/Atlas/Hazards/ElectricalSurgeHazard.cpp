// ElectricalSurgeHazard.cpp
#include "ElectricalSurgeHazard.h"
#include "../Characters/GameCharacterBase.h"
#include "../Components/ActionManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/DamageType.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "../Interfaces/IHealthInterface.h"
#include "Engine/DamageEvents.h"

// Custom damage type for electrical damage

UElectricalSurgeHazard::UElectricalSurgeHazard()
{
    HazardType = EHazardType::Electrical;
    DamagePerSecond = 15.0f;
    HazardRadius = 400.0f;
    
    // Set electrical effect color
    EffectData.HazardColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f); // Electric blue
    
    DamageTypeClass = UDamageType::StaticClass();
    
    LastChainTime = 0.0f;
}

void UElectricalSurgeHazard::ApplyHazardEffect(AActor* Actor, float DeltaTime)
{
    if (!Actor) return;
    
    // Apply base damage
    Super::ApplyHazardEffect(Actor, DeltaTime);
    
    // Apply stun effect
    if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
    {
        // Apply stun through the action manager
        if (UActionManagerComponent* ActionManager = Character->FindComponentByClass<UActionManagerComponent>())
        {
            // Stun logic would be implemented through the action system
            // For now, we'll use a simple movement disable
            Character->GetCharacterMovement()->DisableMovement();
            
            // Re-enable after stun duration
            FTimerHandle StunTimer;
            GetWorld()->GetTimerManager().SetTimer(StunTimer, [Character]()
            {
                if (IsValid(Character))
                {
                    Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
                }
            }, StunDuration, false);
        }
    }
    
    // Chain electricity every ElectricityJumpDelay seconds
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastChainTime >= ElectricityJumpDelay)
    {
        ChainElectricity(Actor);
        LastChainTime = CurrentTime;
    }
}

void UElectricalSurgeHazard::UpdateHazardVisuals(float DeltaTime)
{
    Super::UpdateHazardVisuals(DeltaTime);
    
    // Add electrical sparks and arcs
    if (ActiveHazardEffect)
    {
        // Update electrical intensity based on number of affected actors
        float Intensity = FMath::Clamp(AffectedActors.Num() / 5.0f, 0.1f, 1.0f);
        ActiveHazardEffect->SetFloatParameter(FName("ElectricalIntensity"), Intensity);
        
        // Create random electrical arcs
        if (FMath::FRand() < 0.1f) // 10% chance per frame
        {
            FVector RandomOffset = FMath::VRand() * HazardRadius;
            FVector ArcStart = GetComponentLocation();
            FVector ArcEnd = ArcStart + RandomOffset;
            
            SpawnElectricalArc(ArcStart, ArcEnd);
        }
    }
}

void UElectricalSurgeHazard::ChainElectricity(AActor* Source)
{
    if (!Source) return;
    
    // Clear previous chain list
    ChainedActors.Empty();
    ChainedActors.Add(Source);
    
    // Find nearby targets to chain to
    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(ChainRadius);
    
    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Source->GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        SphereShape
    );
    
    int32 ChainCount = 0;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Target = Overlap.GetActor();
        
        // Skip if it's the source, already chained, or not a valid target
        if (!Target || Target == Source || ChainedActors.Contains(Target) || !ShouldAffectActor(Target))
        {
            continue;
        }
        
        // Check line of sight
        FHitResult LineTraceHit;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Source);
        QueryParams.AddIgnoredActor(Target);
        
        bool bHasLineOfSight = !GetWorld()->LineTraceSingleByChannel(
            LineTraceHit,
            Source->GetActorLocation(),
            Target->GetActorLocation(),
            ECC_Visibility,
            QueryParams
        );
        
        if (bHasLineOfSight)
        {
            // Spawn chain lightning effect
            SpawnElectricalArc(Source->GetActorLocation(), Target->GetActorLocation());
            
            // Apply reduced damage to chained target
            float ChainDamage = DamagePerSecond * ChainDamageMultiplier;
            // Apply damage to chained target
            if (Target->GetClass()->ImplementsInterface(UHealthInterface::StaticClass()))
            {
                IHealthInterface::Execute_ApplyDamage(Target, ChainDamage, GetOwner());
            }
            else
            {
                Target->TakeDamage(ChainDamage, FDamageEvent(UDamageType::StaticClass()), nullptr, GetOwner());
            }
            
            // Add to chained list
            ChainedActors.Add(Target);
            
            // Fire chain event
            OnElectricityChained(Source, Target, ChainDamage);
            
            // Apply brief stun to chained target
            if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Target))
            {
                Character->GetCharacterMovement()->DisableMovement();
                
                FTimerHandle StunTimer;
                GetWorld()->GetTimerManager().SetTimer(StunTimer, [Character]()
                {
                    if (IsValid(Character))
                    {
                        Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
                    }
                }, StunDuration * 0.5f, false); // Half stun duration for chained targets
            }
            
            ChainCount++;
            if (ChainCount >= MaxChainTargets)
            {
                break;
            }
        }
    }
}

void UElectricalSurgeHazard::SpawnElectricalArc(const FVector& Start, const FVector& End)
{
    // Spawn lightning effect between two points
    if (ChainLightningEffect)
    {
        UParticleSystemComponent* Arc = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ChainLightningEffect,
            Start,
            (End - Start).Rotation(),
            FVector(1.0f)
        );
        
        if (Arc)
        {
            // Set beam target
            Arc->SetBeamTargetPoint(0, End, 0);
            
            // Auto-destroy after a short time
            FTimerHandle DestroyTimer;
            GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [Arc]()
            {
                if (IsValid(Arc))
                {
                    Arc->DestroyComponent();
                }
            }, 0.2f, false);
        }
    }
    
    // Play electricity jump sound
    if (ElectricityJumpSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            ElectricityJumpSound,
            (Start + End) * 0.5f,
            FRotator::ZeroRotator,
            0.5f
        );
    }
    
    // Camera shake for nearby players
    if (ElectricalShake)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            float DistanceToPlayer = FVector::Dist(PC->GetPawn()->GetActorLocation(), Start);
            if (DistanceToPlayer < 1000.0f)
            {
                float ShakeScale = 1.0f - (DistanceToPlayer / 1000.0f);
                PC->ClientStartCameraShake(ElectricalShake, ShakeScale);
            }
        }
    }
    
    // Debug visualization
#if WITH_EDITOR
    DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, false, 0.2f, 0, 5.0f);
#endif
}