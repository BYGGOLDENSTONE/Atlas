#include "WallImpactComponent.h"
#include "CombatComponent.h"
#include "HealthComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UWallImpactComponent::UWallImpactComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UWallImpactComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UWallImpactComponent::CheckForWallImpact(const FVector& KnockbackDirection, float KnockbackForce, FHitResult& OutWallHit)
{
    if (!GetOwner() || KnockbackForce < MinImpactForce)
    {
        return false;
    }
    
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = StartLocation + (KnockbackDirection * WallDetectionDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;
    
    // Check for walls, static geometry
    bool bHitWall = GetWorld()->LineTraceSingleByChannel(
        OutWallHit,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHitWall)
    {
        // Verify it's actually a wall (mostly vertical surface)
        float DotProduct = FVector::DotProduct(OutWallHit.Normal, FVector::UpVector);
        bool bIsWall = FMath::Abs(DotProduct) < 0.3f; // Less than 30 degrees from vertical
        
        if (bIsWall)
        {
            UE_LOG(LogTemp, Warning, TEXT("Wall Impact Detected! Distance: %.1f"), OutWallHit.Distance);
            
            #if WITH_EDITOR
            DrawDebugLine(GetWorld(), StartLocation, OutWallHit.Location, FColor::Red, false, 2.0f, 0, 5.0f);
            DrawDebugSphere(GetWorld(), OutWallHit.Location, 20.0f, 12, FColor::Red, false, 2.0f);
            DrawDebugDirectionalArrow(GetWorld(), OutWallHit.Location, 
                OutWallHit.Location + OutWallHit.Normal * 100.0f, 
                30.0f, FColor::Yellow, false, 2.0f, 0, 3.0f);
            #endif
            
            return true;
        }
    }
    
    return false;
}

void UWallImpactComponent::ApplyWallImpactEffects(AActor* Target, const FHitResult& WallHit, float ImpactForce)
{
    if (!Target)
    {
        return;
    }
    
    // Apply bonus damage
    if (UHealthComponent* HealthComp = Target->FindComponentByClass<UHealthComponent>())
    {
        float BonusDamage = CalculateWallImpactDamage(20.0f); // Base wall impact damage
        HealthComp->TakeDamage(BonusDamage, GetOwner());
        
        UE_LOG(LogTemp, Warning, TEXT("Wall Impact Bonus Damage: %.1f"), BonusDamage);
    }
    
    // Apply extended stagger
    ApplyExtendedStagger(Target);
    
    // Visual feedback - character bounce off wall
    if (ACharacter* Character = Cast<ACharacter>(Target))
    {
        FVector BounceDirection = WallHit.Normal;
        BounceDirection.Z = 0.2f;
        BounceDirection.Normalize();
        
        Character->LaunchCharacter(BounceDirection * ImpactForce * 0.3f, true, true);
    }
    
    // Broadcast event
    OnWallImpact.Broadcast(Target, WallHit, ImpactForce);
}

float UWallImpactComponent::CalculateWallImpactDamage(float BaseDamage)
{
    return BaseDamage * WallImpactDamageMultiplier;
}

void UWallImpactComponent::ApplyExtendedStagger(AActor* Target)
{
    if (!Target)
    {
        return;
    }
    
    UCombatComponent* CombatComp = Target->FindComponentByClass<UCombatComponent>();
    if (!CombatComp)
    {
        return;
    }
    
    // Apply full poise damage to guarantee stagger
    CombatComp->TakePoiseDamage(100.0f);
    
    // Set up extended stagger timer
    FTimerHandle ExtendedStaggerTimer;
    FTimerDelegate StaggerDelegate;
    StaggerDelegate.BindLambda([CombatComp]()
    {
        if (CombatComp && CombatComp->IsStaggered())
        {
            // Keep them staggered longer by resetting poise
            CombatComp->TakePoiseDamage(50.0f);
        }
    });
    
    GetWorld()->GetTimerManager().SetTimer(
        ExtendedStaggerTimer,
        StaggerDelegate,
        WallImpactStaggerDuration * 0.5f,
        false
    );
    
    UE_LOG(LogTemp, Warning, TEXT("Extended Wall Impact Stagger Applied for %.1f seconds"), WallImpactStaggerDuration);
}