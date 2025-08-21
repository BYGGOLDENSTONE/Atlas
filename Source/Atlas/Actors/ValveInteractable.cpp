#include "ValveInteractable.h"
#include "../Core/AtlasGameplayTags.h"
#include "../Characters/GameCharacterBase.h"
#include "../Characters/PlayerCharacter.h"
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
    // Get all actors in radius
    TArray<AActor*> AffectedActors = GetActorsInRadius();
    
    UE_LOG(LogTemp, Warning, TEXT("VALVE: %s AoE triggered (Type: %s, Radius: %.0f, Actors: %d)"), 
        *GetName(), 
        ValveType == EValveType::Vulnerability ? TEXT("Vulnerability") : TEXT("Stagger"),
        AoERadius, AffectedActors.Num());
    
    // Visual effects based on type
    FColor EffectColor = ValveType == EValveType::Vulnerability ? FColor::Purple : FColor::Yellow;
    
    // Draw area effect visualization
    DrawDebugSphere(GetWorld(), GetActorLocation(), AoERadius, 32, EffectColor, false, 3.0f, 0, 3.0f);
    
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
        DrawDebugLine(GetWorld(), Center, EndPoint, EffectColor, false, 3.0f, 0, 2.0f);
    }
    
    // Draw circles at different heights for volume effect
    for (int32 Height = -100; Height <= 100; Height += 50)
    {
        FVector HeightOffset = FVector(0, 0, Height);
        DrawDebugCircle(GetWorld(), Center + HeightOffset, AoERadius * (1.0f - FMath::Abs(Height) / 200.0f), 
            24, EffectColor, false, 3.0f, 0, 2.0f, 
            FVector(0, 1, 0), FVector(1, 0, 0), false);
    }
    
    // Apply effect to all actors in range (neutral behavior)
    for (AActor* Target : AffectedActors)
    {
        if (Target && IsValid(Target))
        {
            ApplyEffectToActor(Target);
            
            // Draw line from valve to affected actor
            DrawDebugLine(GetWorld(), GetActorLocation(), Target->GetActorLocation(), 
                EffectColor, false, 1.0f, 0, 1.0f);
        }
    }
    
    // Trigger Blueprint events
    SpawnVisualEffect(ValveType, AoERadius);
    OnAoETriggered(AffectedActors);
    
    UE_LOG(LogTemp, Warning, TEXT("Valve %s triggered %s AoE effect (Radius: %.0f)"), 
        *GetName(), 
        ValveType == EValveType::Vulnerability ? TEXT("Vulnerability") : TEXT("Stagger"),
        AoERadius);
}

void AValveInteractable::ApplyEffectToActor(AActor* Target)
{
    if (!Target || !IsValid(Target))
    {
        return;
    }
    
    // Apply to any character (neutral behavior)
    AGameCharacterBase* Character = Cast<AGameCharacterBase>(Target);
    if (!Character)
    {
        Character = Cast<APlayerCharacter>(Target);
    }
    
    if (!Character)
    {
        return;
    }
    
    if (ValveType == EValveType::Vulnerability)
    {
        // Apply vulnerability using new tier system
        if (UVulnerabilityComponent* VulnComp = Target->FindComponentByClass<UVulnerabilityComponent>())
        {
            // Valve applies Stunned tier (yellow) - lightest vulnerability
            VulnComp->ApplyVulnerabilityTier(EVulnerabilityTier::Stunned);
            UE_LOG(LogTemp, Warning, TEXT("Valve applied Stunned vulnerability to %s"), 
                *Target->GetName());
        }
    }
    else if (ValveType == EValveType::Stagger)
    {
        // Apply stagger
        if (UHealthComponent* HealthComp = Target->FindComponentByClass<UHealthComponent>())
        {
            HealthComp->TakePoiseDamage(StaggerPoiseDamage);
            HealthComp->PlayHitReaction();
            UE_LOG(LogTemp, Warning, TEXT("Valve applied %.0f poise damage to %s"), 
                StaggerPoiseDamage, *Target->GetName());
        }
    }
    
    // Visual feedback on the target
    FColor EffectColor = ValveType == EValveType::Vulnerability ? FColor::Purple : FColor::Yellow;
    
    // Draw effect indicator above target
    DrawDebugSphere(GetWorld(), Target->GetActorLocation() + FVector(0, 0, 100), 
        30, 12, EffectColor, false, 3.0f, 0, 2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Valve effect applied to %s"), *Target->GetName());
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
                // Check for any character type (neutral behavior)
                if (Actor->IsA(AGameCharacterBase::StaticClass()) || 
                    Actor->IsA(APlayerCharacter::StaticClass()))
                {
                    ActorsInRadius.Add(Actor);
                }
            }
        }
    }
    
    return ActorsInRadius;
}

