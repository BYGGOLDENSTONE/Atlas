// EnvironmentalHazardComponent.cpp
#include "EnvironmentalHazardComponent.h"
#include "../Components/StationIntegrityComponent.h"
#include "../Characters/GameCharacterBase.h"
#include "../Interfaces/IHealthInterface.h"
#include "Engine/DamageEvents.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"

UEnvironmentalHazardComponent::UEnvironmentalHazardComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    bIsActive = false;
    CurrentDuration = 0.0f;
    ActivationTimer = 0.0f;
    bWarningShown = false;
    
    HazardType = EHazardType::None;
    
    // Create trigger sphere
    HazardTriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HazardTriggerSphere"));
    if (HazardTriggerSphere)
    {
        HazardTriggerSphere->SetupAttachment(this);
        HazardTriggerSphere->SetSphereRadius(HazardRadius);
        HazardTriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        HazardTriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
        HazardTriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }
}

void UEnvironmentalHazardComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Setup trigger sphere
    if (HazardTriggerSphere)
    {
        HazardTriggerSphere->SetSphereRadius(HazardRadius);
        HazardTriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &UEnvironmentalHazardComponent::OnTriggerBeginOverlap);
        HazardTriggerSphere->OnComponentEndOverlap.AddDynamic(this, &UEnvironmentalHazardComponent::OnTriggerEndOverlap);
    }
    
    // Auto-activate if configured
    if (bStartActive)
    {
        ActivateHazard();
    }
}

void UEnvironmentalHazardComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DeactivateHazard();
    Super::EndPlay(EndPlayReason);
}

void UEnvironmentalHazardComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsActive) return;
    
    // Update duration
    if (!bPermanent)
    {
        CurrentDuration += DeltaTime;
        if (CurrentDuration >= Duration)
        {
            DeactivateHazard();
            return;
        }
    }
    
    // Apply hazard effects to all affected actors
    for (AActor* Actor : AffectedActors)
    {
        if (IsValid(Actor))
        {
            ApplyHazardEffect(Actor, DeltaTime);
        }
    }
    
    // Update visuals
    UpdateHazardVisuals(DeltaTime);
    
    // Damage station integrity if configured
    if (bDamagesStationIntegrity)
    {
        DamageStationIntegrity(DeltaTime);
    }
    
    // Debug visualization
#if WITH_EDITOR
    if (bShowWarningIndicator)
    {
        DrawDebugSphere(GetWorld(), GetComponentLocation(), HazardRadius, 32, 
            EffectData.HazardColor.ToFColor(true), false, -1, 0, 2.0f);
    }
#endif
}

void UEnvironmentalHazardComponent::ActivateHazard()
{
    if (bIsActive) return;
    
    // Show warning if configured
    if (bShowWarningIndicator && WarningIndicatorTime > 0.0f)
    {
        ShowWarningIndicator();
        
        // Delay activation
        GetWorld()->GetTimerManager().SetTimer(WarningTimerHandle, [this]()
        {
            StartHazard();
        }, WarningIndicatorTime, false);
    }
    else
    {
        // Activate immediately
        if (ActivationDelay > 0.0f)
        {
            GetWorld()->GetTimerManager().SetTimer(WarningTimerHandle, [this]()
            {
                StartHazard();
            }, ActivationDelay, false);
        }
        else
        {
            StartHazard();
        }
    }
}

void UEnvironmentalHazardComponent::StartHazard()
{
    bIsActive = true;
    CurrentDuration = 0.0f;
    SetComponentTickEnabled(true);
    
    // Spawn effects
    SpawnHazardEffects();
    
    // Start duration timer if not permanent
    if (!bPermanent)
    {
        GetWorld()->GetTimerManager().SetTimer(DeactivationTimerHandle, this, 
            &UEnvironmentalHazardComponent::DeactivateHazard, Duration, false);
    }
    
    // Find initial affected actors
    UpdateActiveActors();
    
    // Fire activation event
    OnHazardActivated();
}

void UEnvironmentalHazardComponent::DeactivateHazard()
{
    if (!bIsActive) return;
    
    bIsActive = false;
    SetComponentTickEnabled(false);
    
    // Clear timers
    GetWorld()->GetTimerManager().ClearTimer(DeactivationTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(WarningTimerHandle);
    
    // Clean up effects
    DestroyHazardEffects();
    HideWarningIndicator();
    
    // Notify all affected actors
    for (AActor* Actor : AffectedActors)
    {
        if (IsValid(Actor))
        {
            OnActorExitHazard(Actor);
        }
    }
    AffectedActors.Empty();
    
    // Fire deactivation event
    OnHazardDeactivated();
}

void UEnvironmentalHazardComponent::ToggleHazard()
{
    if (bIsActive)
    {
        DeactivateHazard();
    }
    else
    {
        ActivateHazard();
    }
}

float UEnvironmentalHazardComponent::GetRemainingDuration() const
{
    if (bPermanent || !bIsActive)
    {
        return 0.0f;
    }
    
    return FMath::Max(0.0f, Duration - CurrentDuration);
}

void UEnvironmentalHazardComponent::ApplyHazardEffect(AActor* Actor, float DeltaTime)
{
    if (!Actor) return;
    
    // Apply damage
    if (DamagePerSecond > 0.0f)
    {
        float DamageAmount = DamagePerSecond * DeltaTime;
        
        // Apply damage directly through interface if available
        if (Actor->GetClass()->ImplementsInterface(UHealthInterface::StaticClass()))
        {
            IHealthInterface::Execute_ApplyDamage(Actor, DamageAmount, GetOwner());
        }
        else
        {
            // Fallback to standard damage
            Actor->TakeDamage(DamageAmount, FDamageEvent(DamageTypeClass ? DamageTypeClass.Get() : UDamageType::StaticClass()), nullptr, GetOwner());
        }
        
        OnActorDamagedByHazard(Actor, DamageAmount);
    }
}

void UEnvironmentalHazardComponent::UpdateHazardVisuals(float DeltaTime)
{
    // Update particle effect parameters if needed
    if (ActiveHazardEffect)
    {
        // Pulse effect based on intensity
        float PulseValue = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 0.5f + 0.5f;
        ActiveHazardEffect->SetFloatParameter(FName("Intensity"), PulseValue);
    }
}

bool UEnvironmentalHazardComponent::ShouldAffectActor(AActor* Actor) const
{
    if (!Actor) return false;
    
    // Check if it's a player or enemy
    bool bIsPlayer = Actor->ActorHasTag(FName("Player"));
    bool bIsEnemy = Actor->ActorHasTag(FName("Enemy"));
    
    if (bIsPlayer && !bAffectsPlayers) return false;
    if (bIsEnemy && !bAffectsEnemies) return false;
    
    // Check if actor has health component (can be damaged)
    if (Actor->GetClass()->ImplementsInterface(UHealthInterface::StaticClass()))
    {
        return true;
    }
    
    return false;
}

void UEnvironmentalHazardComponent::ShowWarningIndicator()
{
    if (bWarningShown) return;
    
    bWarningShown = true;
    
    // Create warning decal
    if (EffectData.HazardDecal && !ActiveHazardDecal)
    {
        ActiveHazardDecal = NewObject<UDecalComponent>(this);
        if (ActiveHazardDecal)
        {
            ActiveHazardDecal->SetupAttachment(this);
            ActiveHazardDecal->RegisterComponent();
            ActiveHazardDecal->SetDecalMaterial(EffectData.HazardDecal);
            ActiveHazardDecal->DecalSize = FVector(HazardRadius, HazardRadius, HazardRadius);
            ActiveHazardDecal->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
            
            // Pulse the warning indicator
            // This would be better done with a material parameter in production
        }
    }
    
    // Play warning sound
    if (EffectData.HazardSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), EffectData.HazardSound, 
            GetComponentLocation());
    }
}

void UEnvironmentalHazardComponent::HideWarningIndicator()
{
    bWarningShown = false;
    
    if (ActiveHazardDecal)
    {
        ActiveHazardDecal->DestroyComponent();
        ActiveHazardDecal = nullptr;
    }
}

void UEnvironmentalHazardComponent::SpawnHazardEffects()
{
    // Spawn particle effect
    if (EffectData.HazardEffect && !ActiveHazardEffect)
    {
        ActiveHazardEffect = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            EffectData.HazardEffect,
            GetComponentLocation(),
            GetComponentRotation(),
            true
        );
        
        if (ActiveHazardEffect)
        {
            ActiveHazardEffect->SetColorParameter(FName("HazardColor"), EffectData.HazardColor);
        }
    }
    
    // Start looping sound
    if (EffectData.HazardSound && !ActiveHazardSound)
    {
        ActiveHazardSound = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            EffectData.HazardSound,
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
    
    // Update decal
    if (EffectData.HazardDecal && !ActiveHazardDecal)
    {
        ActiveHazardDecal = NewObject<UDecalComponent>(this);
        if (ActiveHazardDecal)
        {
            ActiveHazardDecal->SetupAttachment(this);
            ActiveHazardDecal->RegisterComponent();
            ActiveHazardDecal->SetDecalMaterial(EffectData.HazardDecal);
            ActiveHazardDecal->DecalSize = FVector(HazardRadius, HazardRadius, HazardRadius);
            ActiveHazardDecal->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
        }
    }
}

void UEnvironmentalHazardComponent::DestroyHazardEffects()
{
    if (ActiveHazardEffect)
    {
        ActiveHazardEffect->DestroyComponent();
        ActiveHazardEffect = nullptr;
    }
    
    if (ActiveHazardSound)
    {
        ActiveHazardSound->Stop();
        ActiveHazardSound->DestroyComponent();
        ActiveHazardSound = nullptr;
    }
    
    if (ActiveHazardDecal)
    {
        ActiveHazardDecal->DestroyComponent();
        ActiveHazardDecal = nullptr;
    }
}

void UEnvironmentalHazardComponent::UpdateActiveActors()
{
    // Get all overlapping actors
    if (HazardTriggerSphere)
    {
        TArray<AActor*> OverlappingActors;
        HazardTriggerSphere->GetOverlappingActors(OverlappingActors);
        
        // Check for new actors
        for (AActor* Actor : OverlappingActors)
        {
            if (ShouldAffectActor(Actor) && !AffectedActors.Contains(Actor))
            {
                AffectedActors.Add(Actor);
                OnActorEnterHazard(Actor);
            }
        }
        
        // Check for actors that left
        for (int32 i = AffectedActors.Num() - 1; i >= 0; --i)
        {
            if (!OverlappingActors.Contains(AffectedActors[i]))
            {
                AActor* LeavingActor = AffectedActors[i];
                AffectedActors.RemoveAt(i);
                OnActorExitHazard(LeavingActor);
            }
        }
    }
}

void UEnvironmentalHazardComponent::DamageStationIntegrity(float DeltaTime)
{
    // Find station integrity component
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameCharacterBase::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (UStationIntegrityComponent* IntegrityComp = Actor->FindComponentByClass<UStationIntegrityComponent>())
        {
            IntegrityComp->ApplyIntegrityDamage(IntegrityDamagePerSecond * DeltaTime);
            break;
        }
    }
}

void UEnvironmentalHazardComponent::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsActive || !OtherActor) return;
    
    if (ShouldAffectActor(OtherActor) && !AffectedActors.Contains(OtherActor))
    {
        AffectedActors.Add(OtherActor);
        OnActorEnterHazard(OtherActor);
    }
}

void UEnvironmentalHazardComponent::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!bIsActive || !OtherActor) return;
    
    if (AffectedActors.Contains(OtherActor))
    {
        AffectedActors.Remove(OtherActor);
        OnActorExitHazard(OtherActor);
    }
}

void UEnvironmentalHazardComponent::OnActorEnterHazard_Implementation(AActor* Actor)
{
    // Override in Blueprint or derived classes
}

void UEnvironmentalHazardComponent::OnActorExitHazard_Implementation(AActor* Actor)
{
    // Override in Blueprint or derived classes
}