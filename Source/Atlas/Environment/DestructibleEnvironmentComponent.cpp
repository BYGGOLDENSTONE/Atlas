// DestructibleEnvironmentComponent.cpp
#include "DestructibleEnvironmentComponent.h"
#include "../Components/StationIntegrityComponent.h"
#include "../Characters/GameCharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/BodySetup.h"
#include "Components/PrimitiveComponent.h"

UDestructibleEnvironmentComponent::UDestructibleEnvironmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentHealth = MaxHealth;
    CurrentState = EDestructibleState::Intact;
    CurrentStageIndex = -1;
}

void UDestructibleEnvironmentComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentHealth = MaxHealth;
    CurrentState = EDestructibleState::Intact;
    CurrentStageIndex = -1;
    
    // Find mesh component on owner
    MeshComponent = GetMeshComponent();
    
    // Sort destruction stages by health threshold
    DestructionStages.Sort([](const FDestructionStage& A, const FDestructionStage& B)
    {
        return A.HealthThreshold > B.HealthThreshold;
    });
    
    UpdateDestructionState();
}

void UDestructibleEnvironmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);
    CleanupDebris();
    CleanupEffects();
    
    Super::EndPlay(EndPlayReason);
}

void UDestructibleEnvironmentComponent::ApplyDamage(float DamageAmount, AActor* DamageInstigator)
{
    if (CurrentState == EDestructibleState::Destroyed || DamageAmount <= 0.0f)
    {
        return;
    }
    
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
    
    // Play damage effects
    PlayDamageEffect(DamageAmount);
    
    // Update state
    UpdateDestructionState();
    
    // Fire damage event
    OnDamaged(DamageAmount, DamageInstigator);
    
    // Check if destroyed
    if (CurrentHealth <= 0.0f && CurrentState != EDestructibleState::Destroyed)
    {
        TransitionToState(EDestructibleState::Destroyed);
        
        // Damage station integrity if configured
        if (bDamagesStationIntegrity)
        {
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameCharacterBase::StaticClass(), FoundActors);
            
            for (AActor* Actor : FoundActors)
            {
                if (UStationIntegrityComponent* IntegrityComp = Actor->FindComponentByClass<UStationIntegrityComponent>())
                {
                    IntegrityComp->ApplyIntegrityDamage(IntegrityDamageOnDestruction);
                    break;
                }
            }
        }
        
        OnDestroyed();
        
        // Schedule actor destruction if configured
        if (bDestroyActorWhenDestroyed)
        {
            GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, [this]()
            {
                if (AActor* Owner = GetOwner())
                {
                    Owner->Destroy();
                }
            }, DestroyDelay, false);
        }
    }
}

void UDestructibleEnvironmentComponent::Repair(float RepairAmount)
{
    if (!bCanBeRepaired || CurrentState == EDestructibleState::Destroyed || RepairAmount <= 0.0f)
    {
        return;
    }
    
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth + RepairAmount, 0.0f, MaxHealth);
    
    // Update state
    UpdateDestructionState();
    
    // Fire repair event
    OnRepaired(RepairAmount);
}

void UDestructibleEnvironmentComponent::SetHealth(float NewHealth)
{
    CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
    UpdateDestructionState();
}

float UDestructibleEnvironmentComponent::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void UDestructibleEnvironmentComponent::ForceDestroy()
{
    CurrentHealth = 0.0f;
    TransitionToState(EDestructibleState::Destroyed);
    OnDestroyed();
}

void UDestructibleEnvironmentComponent::ResetToIntact()
{
    CurrentHealth = MaxHealth;
    CurrentState = EDestructibleState::Intact;
    CurrentStageIndex = -1;
    
    CleanupDebris();
    CleanupEffects();
    DisablePhysicsSimulation();
    
    UpdateVisuals();
}

void UDestructibleEnvironmentComponent::UpdateDestructionState()
{
    EDestructibleState NewState = EDestructibleState::Intact;
    float HealthPercent = GetHealthPercent();
    
    if (HealthPercent <= 0.0f)
    {
        NewState = EDestructibleState::Destroyed;
    }
    else if (HealthPercent <= 0.25f)
    {
        NewState = EDestructibleState::Critical;
    }
    else if (HealthPercent <= 0.5f)
    {
        NewState = EDestructibleState::Damaged;
    }
    else
    {
        NewState = EDestructibleState::Intact;
    }
    
    if (NewState != CurrentState)
    {
        TransitionToState(NewState);
    }
    
    // Check for stage transitions
    for (int32 i = 0; i < DestructionStages.Num(); i++)
    {
        if (HealthPercent <= DestructionStages[i].HealthThreshold && i != CurrentStageIndex)
        {
            CurrentStageIndex = i;
            ApplyStateEffects(NewState);
            break;
        }
    }
}

void UDestructibleEnvironmentComponent::TransitionToState(EDestructibleState NewState)
{
    EDestructibleState OldState = CurrentState;
    CurrentState = NewState;
    
    ApplyStateEffects(NewState);
    OnStateChanged(OldState, NewState);
}

void UDestructibleEnvironmentComponent::ApplyStateEffects(EDestructibleState State)
{
    // Update mesh if we have a stage for current health
    if (CurrentStageIndex >= 0 && CurrentStageIndex < DestructionStages.Num())
    {
        const FDestructionStage& Stage = DestructionStages[CurrentStageIndex];
        
        // Change mesh
        if (Stage.StageMesh && MeshComponent)
        {
            MeshComponent->SetStaticMesh(Stage.StageMesh);
        }
        
        // Play transition effect
        if (Stage.TransitionEffect)
        {
            UParticleSystemComponent* Effect = UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                Stage.TransitionEffect,
                GetOwner()->GetActorLocation(),
                GetOwner()->GetActorRotation()
            );
            
            if (Effect)
            {
                ActiveEffects.Add(Effect);
            }
        }
        
        // Play transition sound
        if (Stage.TransitionSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                Stage.TransitionSound,
                GetOwner()->GetActorLocation()
            );
        }
        
        // Enable physics if configured
        if (Stage.bEnablePhysics)
        {
            EnablePhysicsSimulation();
        }
        
        // Create debris if configured
        if (Stage.bCreateDebris)
        {
            for (int32 i = 0; i < Stage.DebrisCount; i++)
            {
                SpawnDebris();
            }
        }
    }
    
    // State-specific effects
    switch (State)
    {
        case EDestructibleState::Destroyed:
            CreateExplosionEffect();
            if (DestroySound)
            {
                UGameplayStatics::PlaySoundAtLocation(
                    GetWorld(),
                    DestroySound,
                    GetOwner()->GetActorLocation()
                );
            }
            if (DestroyCameraShake)
            {
                UGameplayStatics::PlayWorldCameraShake(
                    GetWorld(),
                    DestroyCameraShake,
                    GetOwner()->GetActorLocation(),
                    0.0f,
                    1000.0f
                );
            }
            break;
            
        case EDestructibleState::Critical:
            // Could add sparks, smoke, etc.
            break;
            
        case EDestructibleState::Damaged:
            // Could add minor damage effects
            break;
            
        default:
            break;
    }
}

void UDestructibleEnvironmentComponent::UpdateVisuals()
{
    // Update mesh based on current stage
    if (CurrentStageIndex >= 0 && CurrentStageIndex < DestructionStages.Num())
    {
        const FDestructionStage& Stage = DestructionStages[CurrentStageIndex];
        if (Stage.StageMesh && MeshComponent)
        {
            MeshComponent->SetStaticMesh(Stage.StageMesh);
        }
    }
}

void UDestructibleEnvironmentComponent::SpawnDebris()
{
    if (DebrisMeshes.Num() == 0) return;
    
    // Pick random debris mesh
    UStaticMesh* DebrisMesh = DebrisMeshes[FMath::RandRange(0, DebrisMeshes.Num() - 1)];
    if (!DebrisMesh) return;
    
    // Spawn debris actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* DebrisActor = GetWorld()->SpawnActor<AActor>(
        AActor::StaticClass(),
        GetOwner()->GetActorLocation() + FMath::VRand() * 50.0f,
        FRotator::MakeFromEuler(FMath::VRand() * 360.0f),
        SpawnParams
    );
    
    if (DebrisActor)
    {
        // Add mesh component
        UStaticMeshComponent* DebrisMeshComp = NewObject<UStaticMeshComponent>(DebrisActor);
        DebrisMeshComp->SetStaticMesh(DebrisMesh);
        DebrisMeshComp->SetSimulatePhysics(true);
        DebrisMeshComp->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
        DebrisActor->SetRootComponent(DebrisMeshComp);
        DebrisMeshComp->RegisterComponent();
        
        // Apply impulse
        FVector ImpulseDirection = FMath::VRand();
        ImpulseDirection.Z = FMath::Abs(ImpulseDirection.Z); // Bias upward
        DebrisMeshComp->AddImpulse(ImpulseDirection * DebrisImpulseStrength);
        
        // Set lifespan
        DebrisActor->SetLifeSpan(DebrisLifespan);
        
        SpawnedDebris.Add(DebrisActor);
    }
}

void UDestructibleEnvironmentComponent::CreateExplosionEffect()
{
    if (DestroyEffect)
    {
        UParticleSystemComponent* Effect = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            DestroyEffect,
            GetOwner()->GetActorLocation(),
            GetOwner()->GetActorRotation(),
            FVector(1.0f),
            true
        );
        
        if (Effect)
        {
            ActiveEffects.Add(Effect);
        }
    }
}

void UDestructibleEnvironmentComponent::PlayDamageEffect(float DamageAmount)
{
    if (DamageEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            DamageEffect,
            GetOwner()->GetActorLocation(),
            GetOwner()->GetActorRotation(),
            FVector(1.0f)
        );
    }
    
    if (DamageSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            DamageSound,
            GetOwner()->GetActorLocation(),
            FRotator::ZeroRotator,
            FMath::Clamp(DamageAmount / 50.0f, 0.5f, 1.0f)
        );
    }
    
    if (DamageCameraShake && DamageAmount >= 20.0f)
    {
        UGameplayStatics::PlayWorldCameraShake(
            GetWorld(),
            DamageCameraShake,
            GetOwner()->GetActorLocation(),
            0.0f,
            500.0f,
            1.0f
        );
    }
}

void UDestructibleEnvironmentComponent::CreateDamageDecal(const FVector& Location)
{
    // This would create damage decals on the surface
    // Implementation depends on decal system setup
}

void UDestructibleEnvironmentComponent::EnablePhysicsSimulation()
{
    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
    }
}

void UDestructibleEnvironmentComponent::DisablePhysicsSimulation()
{
    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

UStaticMeshComponent* UDestructibleEnvironmentComponent::GetMeshComponent()
{
    if (!MeshComponent && GetOwner())
    {
        MeshComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    }
    return MeshComponent;
}

void UDestructibleEnvironmentComponent::CleanupDebris()
{
    for (AActor* Debris : SpawnedDebris)
    {
        if (IsValid(Debris))
        {
            Debris->Destroy();
        }
    }
    SpawnedDebris.Empty();
}

void UDestructibleEnvironmentComponent::CleanupEffects()
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