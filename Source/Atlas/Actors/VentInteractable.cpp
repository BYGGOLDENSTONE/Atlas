#include "VentInteractable.h"
#include "../Core/AtlasGameplayTags.h"
#include "../Characters/EnemyCharacter.h"
#include "../Characters/GameCharacterBase.h"
#include "../Components/CombatComponent.h"
#include "../Components/HealthComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"

AVentInteractable::AVentInteractable()
{
    InteractableTypeTag = FAtlasGameplayTags::Get().Interactable_Type_Vent;
    CooldownDuration = 999999.0f; // Effectively infinite - can only be triggered once
    MaxInteractionDistance = 1000.0f; // Not used anymore, focus range is used instead
    
    // Set default physics values
    Mass = 50.0f;
    LaunchSpeed = 1500.0f;
    bShouldBounce = true;
    Bounciness = 0.3f;
}

void AVentInteractable::BeginPlay()
{
    Super::BeginPlay();
    
    SetupPhysics();
    
    // Bind hit event
    if (MeshComponent)
    {
        MeshComponent->OnComponentHit.AddDynamic(this, &AVentInteractable::OnHit);
    }
}

void AVentInteractable::SetupPhysics()
{
    if (MeshComponent)
    {
        // Initially static, will become physics-simulated when launched
        MeshComponent->SetMobility(EComponentMobility::Movable);
        MeshComponent->SetSimulatePhysics(false);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        
        // Set mass
        MeshComponent->SetMassOverrideInKg(NAME_None, Mass, true);
        
        // Configure bounciness if needed
        if (bShouldBounce)
        {
            if (FBodyInstance* BodyInst = MeshComponent->GetBodyInstance())
            {
                BodyInst->bLockXRotation = false;
                BodyInst->bLockYRotation = false;
                BodyInst->bLockZRotation = false;
                
                // Set restitution (bounciness) for the physics material
                if (BodyInst->GetSimplePhysicalMaterial())
                {
                    BodyInst->GetSimplePhysicalMaterial()->Restitution = Bounciness;
                }
            }
        }
    }
}

void AVentInteractable::ExecuteInteraction(AActor* Interactor)
{
    // Prevent multiple triggers
    if (bHasBeenTriggered)
    {
        UE_LOG(LogTemp, Warning, TEXT("Vent %s has already been triggered!"), *GetName());
        return;
    }
    
    Super::ExecuteInteraction(Interactor);
    
    UE_LOG(LogTemp, Warning, TEXT("VENT: Launching projectile (Interactor: %s)"), 
        Interactor ? *Interactor->GetName() : TEXT("None"));
    
    bHasBeenTriggered = true;
    LaunchSelf(Interactor);
    PlayVentAnimation();
}

void AVentInteractable::LaunchSelf(AActor* Interactor)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Vent %s has no MeshComponent!"), *GetName());
        return;
    }
    
    // Use predetermined launch direction
    FVector FinalLaunchDirection = GetPredeterminedLaunchDirection();
    FVector LaunchVelocity = FinalLaunchDirection * LaunchSpeed;
    
    // Visual debug for launch
    FVector StartLocation = GetActorLocation();
    FVector PredictedEndLocation = StartLocation + FinalLaunchDirection * LaunchRange;
    
    DrawDebugLine(GetWorld(), StartLocation, PredictedEndLocation, FColor::Cyan, false, 5.0f, 0, 5.0f);
    DrawDebugSphere(GetWorld(), StartLocation, 30, 12, FColor::Green, false, 5.0f);
    DrawDebugSphere(GetWorld(), PredictedEndLocation, 50, 12, FColor::Red, false, 5.0f);
    
    // Enable physics simulation
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetEnableGravity(true);
    
    // Set gravity scale
    if (FBodyInstance* BodyInst = MeshComponent->GetBodyInstance())
    {
        BodyInst->SetEnableGravity(true);
        // Note: GravityScale is not directly settable in UE5, we'll use mass to affect fall speed
        // Heavier objects fall at the same rate but have more momentum
    }
    
    // Set collision to not block pawns while flying (optional)
    MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // Apply launch velocity
    MeshComponent->SetPhysicsLinearVelocity(LaunchVelocity);
    
    // Add some rotation for visual effect (optional)
    FVector RandomTorque = FVector(
        FMath::RandRange(-100.0f, 100.0f),
        FMath::RandRange(-100.0f, 100.0f),
        FMath::RandRange(-100.0f, 100.0f)
    );
    MeshComponent->SetPhysicsAngularVelocityInDegrees(RandomTorque);
    
    bIsFlying = true;
    
    // Disable interactability after launch
    bIsFocused = false;
    StateTags.RemoveTag(FAtlasGameplayTags::Get().Interactable_State_Ready);
    
    OnVentLaunched(LaunchVelocity);
    
    UE_LOG(LogTemp, Warning, TEXT("Vent %s launched with velocity: %s (Speed: %.0f)"), 
        *GetName(), *LaunchVelocity.ToString(), LaunchVelocity.Size());
}

FVector AVentInteractable::GetPredeterminedLaunchDirection() const
{
    FVector Direction = LaunchDirection;
    
    // If using local direction, transform by actor rotation
    if (bUseLocalDirection)
    {
        Direction = GetActorRotation().RotateVector(LaunchDirection);
    }
    
    // Normalize to ensure consistent speed
    Direction.Normalize();
    
    UE_LOG(LogTemp, Warning, TEXT("Vent %s launching in predetermined direction: %s"), 
        *GetName(), *Direction.ToString());
    
    return Direction;
}

void AVentInteractable::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!bIsFlying || !OtherActor || OtherActor == this)
    {
        return;
    }
    
    // Check if we hit an enemy
    if (Cast<AEnemyCharacter>(OtherActor) || Cast<AGameCharacterBase>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("Vent %s hit %s!"), *GetName(), *OtherActor->GetName());
        
        ApplyStaggerToTarget(OtherActor);
        
        // Visual feedback
        DrawDebugSphere(GetWorld(), Hit.Location, 75, 12, FColor::Yellow, false, 2.0f);
        
        // Reduce velocity after hit but keep flying
        if (MeshComponent)
        {
            FVector CurrentVelocity = MeshComponent->GetPhysicsLinearVelocity();
            MeshComponent->SetPhysicsLinearVelocity(CurrentVelocity * 0.5f);
        }
    }
    
    // Handle bouncing
    if (bShouldBounce && MeshComponent)
    {
        // Physics engine handles bouncing automatically with restitution
        UE_LOG(LogTemp, Log, TEXT("Vent bounced off %s"), *OtherActor->GetName());
    }
}

void AVentInteractable::ApplyStaggerToTarget(AActor* HitActor)
{
    if (!HitActor)
    {
        return;
    }
    
    if (UCombatComponent* CombatComp = HitActor->FindComponentByClass<UCombatComponent>())
    {
        CombatComp->TakePoiseDamage(StaggerPoiseDamage);
        
        if (CombatComp->IsStaggered())
        {
            UE_LOG(LogTemp, Warning, TEXT("Vent staggered %s!"), *HitActor->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Vent damaged %s's poise (%.0f damage)"), *HitActor->GetName(), StaggerPoiseDamage);
        }
    }
    
    // Apply knockback
    if (UPrimitiveComponent* TargetMesh = Cast<UPrimitiveComponent>(HitActor->GetRootComponent()))
    {
        if (TargetMesh->IsSimulatingPhysics())
        {
            FVector KnockbackDirection = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            FVector KnockbackImpulse = KnockbackDirection * 500.0f * Mass;
            TargetMesh->AddImpulseAtLocation(KnockbackImpulse, HitActor->GetActorLocation());
        }
    }
}

