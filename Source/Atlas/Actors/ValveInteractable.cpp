#include "ValveInteractable.h"
#include "../Core/AtlasGameplayTags.h"
#include "../Characters/GameCharacterBase.h"
#include "../Characters/EnemyCharacter.h"
#include "../Components/CombatComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/VulnerabilityComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/OverlapResult.h"

AValveInteractable::AValveInteractable()
{
    InteractableTypeTag = FAtlasGameplayTags::Get().Interactable_Type_Valve;
    CooldownDuration = 10.0f;
    MaxInteractionDistance = 300.0f;
}

void AValveInteractable::BeginPlay()
{
    Super::BeginPlay();
}

void AValveInteractable::ExecuteInteraction(AActor* Interactor)
{
    Super::ExecuteInteraction(Interactor);
    
    LastInteractor = Interactor;
    TriggerAoEEffect(Interactor);
    PlayValveAnimation();
}

void AValveInteractable::TriggerAoEEffect(AActor* Interactor)
{
    // Get all actors in radius for visual effect
    TArray<AActor*> AffectedActors = GetActorsInRadius();
    
    UE_LOG(LogTemp, Warning, TEXT("VALVE: %s AoE triggered (Radius: %.0f, Actors: %d)"), 
        *GetName(), AoERadius, AffectedActors.Num());
    
    // Visual effects based on type
    FColor EffectColor = FColor::Red;
    FString EffectName = TEXT("Fire");
    
    switch (EffectType)
    {
        case EValveEffectType::Electric:
            EffectColor = FColor::Cyan;
            EffectName = TEXT("Electric");
            break;
        case EValveEffectType::Poison:
            EffectColor = FColor::Green;
            EffectName = TEXT("Poison");
            break;
        case EValveEffectType::Physical:
            EffectColor = FColor::White;
            EffectName = TEXT("Physical");
            break;
        default:
            break;
    }
    
    // Draw area effect visualization
    DrawDebugSphere(GetWorld(), GetActorLocation(), AoERadius, 32, EffectColor, false, EffectDuration, 0, 3.0f);
    
    // Draw radial lines for effect
    FVector Center = GetActorLocation();
    for (int32 i = 0; i < 12; i++)
    {
        float Angle = (360.0f / 12) * i;
        FVector EndPoint = Center + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * AoERadius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * AoERadius,
            0
        );
        DrawDebugLine(GetWorld(), Center, EndPoint, EffectColor, false, EffectDuration, 0, 2.0f);
    }
    
    // Draw circles at different heights for volume effect
    for (int32 Height = -100; Height <= 100; Height += 50)
    {
        FVector HeightOffset = FVector(0, 0, Height);
        DrawDebugCircle(GetWorld(), Center + HeightOffset, AoERadius * (1.0f - FMath::Abs(Height) / 200.0f), 
            24, EffectColor, false, EffectDuration, 0, 2.0f, 
            FVector(0, 1, 0), FVector(1, 0, 0), false);
    }
    
    // Apply visual effects to actors in range
    for (AActor* Target : AffectedActors)
    {
        if (Target && IsValid(Target))
        {
            ApplyVisualEffectToActor(Target, Interactor);
            
            // Draw line from valve to affected actor
            DrawDebugLine(GetWorld(), GetActorLocation(), Target->GetActorLocation(), 
                EffectColor, false, 1.0f, 0, 1.0f);
        }
    }
    
    // Trigger Blueprint events
    SpawnVisualEffect(EffectType, AoERadius);
    OnAoETriggered(AffectedActors);
    
    UE_LOG(LogTemp, Warning, TEXT("Valve %s triggered %s AoE effect (Radius: %.0f)"), 
        *GetName(), *EffectName, AoERadius);
    
    // Start effect over time if enabled
    if (bDamageOverTime && EffectDuration > 0)
    {
        ActiveTargets.Empty();
        for (AActor* Target : AffectedActors)
        {
            ActiveTargets.Add(Target);
        }
        StartEffectOverTime();
    }
}

void AValveInteractable::ApplyVisualEffectToActor(AActor* Target, AActor* Interactor)
{
    if (!Target || !IsValid(Target))
    {
        return;
    }
    
    // Only apply visual effects, no damage
    AGameCharacterBase* Character = Cast<AGameCharacterBase>(Target);
    if (!Character)
    {
        return;
    }
    
    bool bIsEnemy = Target->IsA(AEnemyCharacter::StaticClass());
    
    if ((bIsEnemy && !bAffectsEnemies) || (!bIsEnemy && !bAffectsAllies))
    {
        return;
    }
    
    // Get archetype effect for special behaviors
    FValveArchetypeEffect ArchetypeEffect = GetArchetypeEffect(Target);
    
    // Apply vulnerability if specified (non-damaging status effect)
    if (ArchetypeEffect.bAppliesVulnerability)
    {
        if (UVulnerabilityComponent* VulnComp = Target->FindComponentByClass<UVulnerabilityComponent>())
        {
            VulnComp->ApplyVulnerability(1, false);
            UE_LOG(LogTemp, Warning, TEXT("Valve applied vulnerability to %s"), *Target->GetName());
        }
    }
    
    // Apply stagger if specified (non-damaging crowd control)
    if (ArchetypeEffect.bAppliesStagger)
    {
        if (UCombatComponent* CombatComp = Character->GetCombatComponent())
        {
            CombatComp->TakePoiseDamage(30.0f);
            UE_LOG(LogTemp, Warning, TEXT("Valve applied stagger effect to %s"), *Target->GetName());
        }
    }
    
    // Visual feedback on the target
    FColor EffectColor = FColor::Red;
    switch (EffectType)
    {
        case EValveEffectType::Electric:
            EffectColor = FColor::Cyan;
            break;
        case EValveEffectType::Poison:
            EffectColor = FColor::Green;
            break;
        case EValveEffectType::Physical:
            EffectColor = FColor::White;
            break;
    }
    
    // Draw effect indicator above target
    DrawDebugSphere(GetWorld(), Target->GetActorLocation() + FVector(0, 0, 100), 
        30, 12, EffectColor, false, EffectDuration, 0, 2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Valve visual effect applied to %s"), *Target->GetName());
}

// Renamed from ApplyEffectToActor to avoid confusion
void AValveInteractable::ApplyEffectToActor(AActor* Target, AActor* Interactor)
{
    // Redirect to visual-only effect
    ApplyVisualEffectToActor(Target, Interactor);
}

TArray<AActor*> AValveInteractable::GetActorsInRadius() const
{
    TArray<AActor*> ActorsInRadius;
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(AoERadius);
    
    bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        SphereShape,
        QueryParams
    );
    
    if (bHasOverlaps)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            if (AActor* Actor = Result.GetActor())
            {
                if (Actor->IsA(AGameCharacterBase::StaticClass()))
                {
                    ActorsInRadius.Add(Actor);
                }
            }
        }
    }
    
    return ActorsInRadius;
}

FGameplayTag AValveInteractable::GetDamageTypeTag() const
{
    const FAtlasGameplayTags& GameplayTags = FAtlasGameplayTags::Get();
    
    switch (EffectType)
    {
        case EValveEffectType::Fire:
            return GameplayTags.Damage_Type_Fire;
        case EValveEffectType::Electric:
            return GameplayTags.Damage_Type_Electric;
        case EValveEffectType::Poison:
            return GameplayTags.Damage_Type_Poison;
        case EValveEffectType::Physical:
        default:
            return GameplayTags.Damage_Type_Physical;
    }
}

FValveArchetypeEffect AValveInteractable::GetArchetypeEffect(AActor* Target) const
{
    FValveArchetypeEffect DefaultEffect;
    DefaultEffect.DamageMultiplier = 1.0f;
    DefaultEffect.RadiusMultiplier = 1.0f;
    
    if (!Target)
    {
        return DefaultEffect;
    }
    
    for (const FValveArchetypeEffect& Effect : ArchetypeEffects)
    {
        if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Target))
        {
            if (UCombatComponent* CombatComp = Character->GetCombatComponent())
            {
                if (CombatComp->CombatStateTags.HasTag(Effect.ArchetypeTag))
                {
                    return Effect;
                }
            }
        }
    }
    
    return DefaultEffect;
}

void AValveInteractable::StartEffectOverTime()
{
    // Renamed from StartDamageOverTime
    if (DamageOverTimeHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(DamageOverTimeHandle);
    }
    
    GetWorld()->GetTimerManager().SetTimer(
        DamageOverTimeHandle,
        this,
        &AValveInteractable::ApplyTickEffect,
        TickDamageInterval,
        true,
        0.0f
    );
    
    FTimerHandle StopTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        StopTimerHandle,
        this,
        &AValveInteractable::StopEffectOverTime,
        EffectDuration,
        false
    );
}

void AValveInteractable::StopEffectOverTime()
{
    // Renamed from StopDamageOverTime
    GetWorld()->GetTimerManager().ClearTimer(DamageOverTimeHandle);
    ActiveTargets.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Valve %s stopped effect over time"), *GetName());
}

void AValveInteractable::ApplyTickEffect()
{
    // Renamed from ApplyTickDamage
    TArray<TWeakObjectPtr<AActor>> ValidTargets;
    
    for (const TWeakObjectPtr<AActor>& WeakTarget : ActiveTargets)
    {
        if (AActor* Target = WeakTarget.Get())
        {
            if (IsValid(Target))
            {
                float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
                if (Distance <= AoERadius * 1.2f)
                {
                    // Apply visual effect only, no damage
                    ApplyVisualEffectToActor(Target, LastInteractor.Get());
                    ValidTargets.Add(WeakTarget);
                }
            }
        }
    }
    
    ActiveTargets = ValidTargets;
    
    if (ActiveTargets.Num() == 0)
    {
        StopEffectOverTime();
    }
}

// Keep these for backward compatibility but they don't deal damage anymore
void AValveInteractable::StartDamageOverTime()
{
    StartEffectOverTime();
}

void AValveInteractable::StopDamageOverTime()
{
    StopEffectOverTime();
}

void AValveInteractable::ApplyTickDamage()
{
    ApplyTickEffect();
}