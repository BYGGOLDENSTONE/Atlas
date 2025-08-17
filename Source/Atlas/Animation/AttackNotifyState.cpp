#include "AttackNotifyState.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Components/CombatComponent.h"
#include "../Characters/GameCharacterBase.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

UAttackNotifyState::UAttackNotifyState()
{
    // Keep constructor empty to avoid crashes
    TimeSinceLastCheck = 0.0f;
}

void UAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    AlreadyHitActors.Reset();
    TimeSinceLastCheck = 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("Attack Window Begin: %s (Duration: %.2f)"), *AttackName.ToString(), TotalDuration);

    PerformHitDetection(MeshComp);
}

void UAttackNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    if (bContinuousHitDetection)
    {
        TimeSinceLastCheck += FrameDeltaTime;
        
        if (TimeSinceLastCheck >= HitDetectionInterval)
        {
            PerformHitDetection(MeshComp);
            TimeSinceLastCheck = 0.0f;
        }
    }
}

void UAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    UE_LOG(LogTemp, Warning, TEXT("Attack Window End: %s"), *AttackName.ToString());
    AlreadyHitActors.Reset();
}

void UAttackNotifyState::PerformHitDetection(USkeletalMeshComponent* MeshComp)
{
    if (!MeshComp || !MeshComp->GetOwner())
    {
        return;
    }

    AGameCharacterBase* Character = Cast<AGameCharacterBase>(MeshComp->GetOwner());
    if (!Character)
    {
        return;
    }

    UCombatComponent* CombatComp = Character->GetCombatComponent();
    if (!CombatComp)
    {
        return;
    }

    FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
    FVector ForwardVector = Character->GetActorForwardVector();
    FVector HitboxCenter = SocketLocation + (ForwardVector * HitboxForwardOffset);

    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    QueryParams.bTraceComplex = false;

    bool bHit = MeshComp->GetWorld()->SweepMultiByChannel(
        HitResults,
        HitboxCenter,
        HitboxCenter,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(HitboxRadius),
        QueryParams
    );

#if WITH_EDITOR
    DrawDebugSphere(MeshComp->GetWorld(), HitboxCenter, HitboxRadius, 12, FColor::Orange, false, HitDetectionInterval);
#endif

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                if (!AlreadyHitActors.Contains(HitActor))
                {
                    if (AGameCharacterBase* HitCharacter = Cast<AGameCharacterBase>(HitActor))
                    {
                        if (HitCharacter != Character)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("New hit detected on: %s"), *HitCharacter->GetName());
                            AlreadyHitActors.AddUnique(HitActor);
                            CombatComp->ProcessHitFromAnimation(HitCharacter);
                        }
                    }
                }
            }
        }
    }
}