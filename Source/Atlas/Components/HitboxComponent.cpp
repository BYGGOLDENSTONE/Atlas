#include "HitboxComponent.h"
#include "CombatComponent.h"
#include "../Characters/GameCharacterBase.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UHitboxComponent::UHitboxComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Don't set collision or radius in constructor - do it in BeginPlay
    // This avoids crashes during object construction
}

void UHitboxComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Set up collision properties
    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetCollisionObjectType(ECC_WorldDynamic);
    SetCollisionResponseToAllChannels(ECR_Ignore);
    SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    SetHiddenInGame(true);
    SetSphereRadius(50.0f);
    
    OnComponentBeginOverlap.AddDynamic(this, &UHitboxComponent::OnBeginOverlap);
}

void UHitboxComponent::ActivateHitbox(const FGameplayTag& InAttackTag, float Duration)
{
    if (bHitboxActive)
    {
        DeactivateHitbox();
    }

    AttackTag = InAttackTag;
    bHitboxActive = true;
    AlreadyHitActors.Reset();
    
    SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    
    UE_LOG(LogTemp, Warning, TEXT("Hitbox Activated: %s (Radius: %.1f)"), 
        *AttackTag.ToString(), GetScaledSphereRadius());

    OnHitboxActivated.Broadcast(AttackTag, Duration);

    if (Duration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            DeactivationTimer,
            this,
            &UHitboxComponent::DeactivateHitbox,
            Duration,
            false
        );
    }

#if WITH_EDITOR
    if (bDebugDraw)
    {
        DrawDebugHitbox();
    }
#endif
}

void UHitboxComponent::DeactivateHitbox()
{
    if (!bHitboxActive)
    {
        return;
    }

    bHitboxActive = false;
    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AlreadyHitActors.Reset();
    
    GetWorld()->GetTimerManager().ClearTimer(DeactivationTimer);
    
    UE_LOG(LogTemp, Warning, TEXT("Hitbox Deactivated: %s"), *AttackTag.ToString());
    
    OnHitboxDeactivated.Broadcast();
}

void UHitboxComponent::ResetHitActors()
{
    AlreadyHitActors.Reset();
}

bool UHitboxComponent::HasAlreadyHit(AActor* Actor) const
{
    return AlreadyHitActors.Contains(Actor);
}

void UHitboxComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bHitboxActive || !OtherActor || OtherActor == GetOwner())
    {
        return;
    }

    if (AlreadyHitActors.Contains(OtherActor))
    {
        return;
    }

    AGameCharacterBase* HitCharacter = Cast<AGameCharacterBase>(OtherActor);
    if (!HitCharacter)
    {
        return;
    }

    ProcessHit(OtherActor, SweepResult);
}

void UHitboxComponent::ProcessHit(AActor* HitActor, const FHitResult& HitResult)
{
    AlreadyHitActors.AddUnique(HitActor);
    
    UE_LOG(LogTemp, Warning, TEXT("Hitbox Hit Detected: %s hit %s"), 
        *GetOwner()->GetName(), *HitActor->GetName());

    OnHitDetected.Broadcast(HitActor, HitResult);

    if (AGameCharacterBase* OwnerCharacter = Cast<AGameCharacterBase>(GetOwner()))
    {
        if (UCombatComponent* CombatComp = OwnerCharacter->GetCombatComponent())
        {
            if (AGameCharacterBase* HitCharacter = Cast<AGameCharacterBase>(HitActor))
            {
                CombatComp->ProcessHitFromAnimation(HitCharacter);
            }
        }
    }

#if WITH_EDITOR
    if (bDebugDraw)
    {
        DrawDebugSphere(GetWorld(), HitResult.Location, 20.0f, 8, FColor::Yellow, false, 1.0f);
        DrawDebugLine(GetWorld(), GetComponentLocation(), HitResult.Location, FColor::Yellow, false, 1.0f);
    }
#endif
}

void UHitboxComponent::DrawDebugHitbox()
{
#if WITH_EDITOR
    if (GetWorld())
    {
        FVector Location = GetComponentLocation();
        float Radius = GetScaledSphereRadius();
        
        DrawDebugSphere(GetWorld(), Location, Radius, 12, DebugColor, false, DebugDrawDuration);
        
        if (bHitboxActive)
        {
            DrawDebugSphere(GetWorld(), Location, Radius * 1.1f, 12, FColor::Green, false, 0.1f);
        }
    }
#endif
}