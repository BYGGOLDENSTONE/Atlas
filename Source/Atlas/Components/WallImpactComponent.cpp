#include "WallImpactComponent.h"
#include "CombatComponent.h"
#include "HealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UWallImpactComponent::UWallImpactComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    bIsTracking = false;
    CurrentKnockbackForce = 0.0f;
}

void UWallImpactComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UWallImpactComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Draw debug sphere following the tracked target
    #if WITH_EDITOR
    if (bIsTracking && TrackedTarget)
    {
        FVector CurrentPos = TrackedTarget->GetActorLocation();
        // Draw sphere at current enemy position
        DrawDebugSphere(GetWorld(), CurrentPos, CollisionSphereRadius, 
            16, FColor::Yellow, false, 0.0f, 0, 2.0f);
        DrawDebugString(GetWorld(), CurrentPos + FVector(0, 0, 150), 
            FString::Printf(TEXT("TRACKING\nPos: %.0f,%.0f,%.0f"), CurrentPos.X, CurrentPos.Y, CurrentPos.Z), 
            nullptr, FColor::Yellow, 0.0f, true, 1.5f);
        
        // Also show collision sphere actual position
        if (CollisionSphere)
        {
            FVector SpherePos = CollisionSphere->GetComponentLocation();
            DrawDebugLine(GetWorld(), CurrentPos, SpherePos, FColor::Orange, false, 0.0f, 0, 2.0f);
        }
    }
    #endif
}

void UWallImpactComponent::StartKnockbackTracking(AActor* Target, float KnockbackForce)
{
    if (!Target || KnockbackForce < MinImpactForce)
    {
        UE_LOG(LogTemp, Warning, TEXT("Knockback force %.1f below threshold %.1f - no tracking"), 
            KnockbackForce, MinImpactForce);
        return;
    }
    
    // Stop any existing tracking
    StopKnockbackTracking();
    
    TrackedTarget = Target;
    CurrentKnockbackForce = KnockbackForce;
    bIsTracking = true;
    
    // Enable ticking to update debug visualization
    SetComponentTickEnabled(true);
    
    // Try to use existing capsule collision from character
    if (ACharacter* Character = Cast<ACharacter>(Target))
    {
        if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
        {
            // Use existing capsule's hit events
            Capsule->OnComponentHit.AddDynamic(this, &UWallImpactComponent::OnTargetHit);
            UE_LOG(LogTemp, Warning, TEXT("Using character capsule for collision detection on %s"), 
                *Target->GetName());
        }
    }
    else
    {
        // Fallback: Create collision sphere for non-character actors
        if (!CollisionSphere)
        {
            CollisionSphere = NewObject<USphereComponent>(Target, USphereComponent::StaticClass());
            CollisionSphere->SetSphereRadius(CollisionSphereRadius);
            CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            CollisionSphere->SetCollisionObjectType(ECC_Pawn);
            CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
            CollisionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
            CollisionSphere->SetNotifyRigidBodyCollision(true);
            CollisionSphere->RegisterComponent();
            
            // Attach to root component
            CollisionSphere->AttachToComponent(Target->GetRootComponent(), 
                FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
            
            // Bind collision event
            CollisionSphere->OnComponentHit.AddDynamic(this, &UWallImpactComponent::OnTargetHit);
            
            UE_LOG(LogTemp, Warning, TEXT("Collision sphere created for %s"), *Target->GetName());
        }
    }
    
    // Initial debug draw will be handled by Tick
    
    UE_LOG(LogTemp, Warning, TEXT("[KNOCKBACK TRACKING] Started for %s | Force: %.1f | Threshold: %.1f"), 
        *Target->GetName(), KnockbackForce, MinImpactForce);
    
    // Set timeout to stop tracking after reasonable time (e.g., 2 seconds)
    GetWorld()->GetTimerManager().SetTimer(
        TrackingTimeoutHandle,
        this,
        &UWallImpactComponent::StopKnockbackTracking,
        2.0f,
        false
    );
}

void UWallImpactComponent::StopKnockbackTracking()
{
    if (!bIsTracking)
    {
        return;
    }
    
    bIsTracking = false;
    
    // Disable ticking when not tracking
    SetComponentTickEnabled(false);
    
    // Unbind from character capsule if used
    if (TrackedTarget)
    {
        if (ACharacter* Character = Cast<ACharacter>(TrackedTarget))
        {
            if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
            {
                Capsule->OnComponentHit.RemoveDynamic(this, &UWallImpactComponent::OnTargetHit);
            }
        }
    }
    
    if (CollisionSphere)
    {
        CollisionSphere->DestroyComponent();
        CollisionSphere = nullptr;
    }
    
    // Clear timeout timer
    GetWorld()->GetTimerManager().ClearTimer(TrackingTimeoutHandle);
    
    TrackedTarget = nullptr;
    CurrentKnockbackForce = 0.0f;
    
}

void UWallImpactComponent::OnTargetHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
    UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    UE_LOG(LogTemp, Warning, TEXT("OnTargetHit Called! HitComponent: %s, OtherActor: %s"), 
        HitComponent ? *HitComponent->GetName() : TEXT("NULL"),
        OtherActor ? *OtherActor->GetName() : TEXT("NULL"));
    
    if (!bIsTracking || !TrackedTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnTargetHit ignored - Tracking: %s, Target: %s"),
            bIsTracking ? TEXT("Yes") : TEXT("No"),
            TrackedTarget ? TEXT("Valid") : TEXT("NULL"));
        return;
    }
    
    // Log hit details
    UE_LOG(LogTemp, Warning, TEXT("Hit detected! BlockingHit: %s, ImpactPoint: %s, Normal: %s"),
        Hit.bBlockingHit ? TEXT("Yes") : TEXT("No"),
        *Hit.ImpactPoint.ToString(),
        *Hit.ImpactNormal.ToString());
    
    // Check if this is a significant impact
    if (Hit.bBlockingHit)
    {
        #if WITH_EDITOR
        // Debug visualization of impact
        DrawDebugSphere(GetWorld(), Hit.Location, 30.0f, 12, FColor::Red, false, 3.0f);
        DrawDebugDirectionalArrow(GetWorld(), Hit.Location, 
            Hit.Location + Hit.Normal * 100.0f, 
            30.0f, FColor::Cyan, false, 3.0f, 0, 3.0f);
        
        FString SurfaceType = IsWallHit(Hit) ? TEXT("WALL") : (IsFloorHit(Hit) ? TEXT("FLOOR") : TEXT("SLOPE"));
        FString ImpactColor = IsWallHit(Hit) ? TEXT("RED") : (IsFloorHit(Hit) ? TEXT("GREEN") : TEXT("YELLOW"));
        
        UE_LOG(LogTemp, Warning, TEXT("========================================"));
        UE_LOG(LogTemp, Warning, TEXT("KNOCKBACK IMPACT DETECTED!"));
        UE_LOG(LogTemp, Warning, TEXT("  Type: %s"), *SurfaceType);
        UE_LOG(LogTemp, Warning, TEXT("  Surface Normal Z: %.2f"), Hit.Normal.Z);
        UE_LOG(LogTemp, Warning, TEXT("  Impact Location: %s"), *Hit.Location.ToString());
        UE_LOG(LogTemp, Warning, TEXT("  Impact Actor: %s"), Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"));
        UE_LOG(LogTemp, Warning, TEXT("  Debug Color: %s"), *ImpactColor);
        UE_LOG(LogTemp, Warning, TEXT("========================================"));
        #endif
        
        // Determine impact type and apply appropriate effects
        if (IsWallHit(Hit))
        {
            ApplyWallImpactEffects(TrackedTarget, Hit);
            StopKnockbackTracking();  // Stop tracking after wall hit
        }
        else if (IsFloorHit(Hit))
        {
            ApplyFloorImpactEffects(TrackedTarget, Hit);
            StopKnockbackTracking();  // Stop tracking after floor hit
        }
        // Slopes could be handled differently if needed
    }
}

bool UWallImpactComponent::IsWallHit(const FHitResult& Hit) const
{
    // Wall = mostly vertical surface (Normal.Z < 0.3)
    return FMath::Abs(Hit.Normal.Z) < WallSurfaceAngleThreshold;
}

bool UWallImpactComponent::IsFloorHit(const FHitResult& Hit) const
{
    // Floor = mostly horizontal surface (Normal.Z > 0.7)
    return Hit.Normal.Z > FloorSurfaceAngleThreshold;
}

void UWallImpactComponent::ApplyWallImpactEffects(AActor* Target, const FHitResult& WallHit)
{
    if (!Target)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT(">>> WALL IMPACT EFFECTS APPLIED <<<"));
    UE_LOG(LogTemp, Warning, TEXT("  - Stagger Duration: %.1f seconds"), WallImpactStaggerDuration);
    UE_LOG(LogTemp, Warning, TEXT("  - Bounce Force Applied"));
    UE_LOG(LogTemp, Warning, TEXT("  - Wall Break VFX at: %s"), *WallHit.Location.ToString());
    
    // Apply extended stagger for wall impact
    UHealthComponent* HealthComp = Target->FindComponentByClass<UHealthComponent>();
    if (HealthComp)
    {
        // Apply full poise damage to guarantee stagger
        HealthComp->TakePoiseDamage(100.0f);
        
        // Set up extended stagger timer for wall impact (2 seconds)
        FTimerHandle ExtendedStaggerTimer;
        FTimerDelegate StaggerDelegate;
        StaggerDelegate.BindLambda([HealthComp, this]()
        {
            if (HealthComp && HealthComp->IsStaggered())
            {
                // Keep them staggered for the full duration
                HealthComp->TakePoiseDamage(50.0f);
            }
        });
        
        GetWorld()->GetTimerManager().SetTimer(
            ExtendedStaggerTimer,
            StaggerDelegate,
            WallImpactStaggerDuration * 0.5f,
            false
        );
    }
    
    // Visual feedback - character bounce off wall
    if (ACharacter* Character = Cast<ACharacter>(Target))
    {
        FVector BounceDirection = WallHit.Normal;
        BounceDirection.Z = 0.2f;
        BounceDirection.Normalize();
        
        // Smaller bounce for wall impact
        Character->LaunchCharacter(BounceDirection * CurrentKnockbackForce * 0.2f, true, true);
    }
    
    // Spawn wall breaking effect at impact point
    #if WITH_EDITOR
    // Large red sphere for wall impact
    DrawDebugSphere(GetWorld(), WallHit.Location, 75.0f, 24, FColor::Red, false, 5.0f, 0, 8.0f);
    // Show impact force direction
    DrawDebugDirectionalArrow(GetWorld(), WallHit.Location, 
        WallHit.Location + WallHit.Normal * 200.0f, 
        50.0f, FColor::Orange, false, 5.0f, 0, 5.0f);
    // Impact text
    DrawDebugString(GetWorld(), WallHit.Location + FVector(0, 0, 100), 
        FString::Printf(TEXT("WALL IMPACT!\n2 sec stagger\nForce: %.0f"), CurrentKnockbackForce), 
        nullptr, FColor::Red, 5.0f, true, 2.0f);
    #endif
    
    // Broadcast wall impact event
    OnWallImpact.Broadcast(Target, WallHit, CurrentKnockbackForce);
    
    // TODO: Play wall impact animation
    // TODO: Spawn wall break particle effect
}

void UWallImpactComponent::ApplyFloorImpactEffects(AActor* Target, const FHitResult& FloorHit)
{
    if (!Target)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT(">>> FLOOR IMPACT EFFECTS APPLIED <<<"));
    UE_LOG(LogTemp, Warning, TEXT("  - Ragdoll Duration: %.1f seconds"), FloorRagdollDuration);
    UE_LOG(LogTemp, Warning, TEXT("  - Recovery Timer Started"));
    UE_LOG(LogTemp, Warning, TEXT("  - Floor Impact at: %s"), *FloorHit.Location.ToString());
    
    UHealthComponent* HealthComp = Target->FindComponentByClass<UHealthComponent>();
    if (HealthComp)
    {
        // Apply moderate poise damage for floor impact
        HealthComp->TakePoiseDamage(50.0f);
        
        // Set up recovery timer for floor impact (1 second)
        FTimerHandle FloorRecoveryTimer;
        FTimerDelegate RecoveryDelegate;
        RecoveryDelegate.BindLambda([Target]()
        {
            // TODO: Play get-up animation
            UE_LOG(LogTemp, Warning, TEXT("Floor recovery complete - playing get-up animation"));
        });
        
        GetWorld()->GetTimerManager().SetTimer(
            FloorRecoveryTimer,
            RecoveryDelegate,
            FloorRagdollDuration,
            false
        );
    }
    
    // Spawn floor impact effect
    #if WITH_EDITOR
    // Medium green circle for floor impact
    DrawDebugCircle(GetWorld(), FloorHit.Location, 60.0f, 32, FColor::Green, false, 3.0f, 0, 4.0f, 
        FVector(0, 0, 1), FVector(1, 0, 0));
    DrawDebugSphere(GetWorld(), FloorHit.Location, 40.0f, 16, FColor::Green, false, 3.0f, 0, 3.0f);
    // Impact text
    DrawDebugString(GetWorld(), FloorHit.Location + FVector(0, 0, 50), 
        FString::Printf(TEXT("FLOOR IMPACT!\n1 sec ragdoll\nForce: %.0f"), CurrentKnockbackForce), 
        nullptr, FColor::Green, 3.0f, true, 1.5f);
    #endif
    
    // Broadcast floor impact event
    OnFloorImpact.Broadcast(Target, FloorHit);
    
    // TODO: Play floor impact animation
}