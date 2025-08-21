#include "AttackNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Components/ActionManagerComponent.h"
#include "../Characters/GameCharacterBase.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

UAttackNotify::UAttackNotify()
{
    // Keep constructor empty to avoid crashes
}

void UAttackNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp || !MeshComp->GetOwner())
    {
        return;
    }

    AGameCharacterBase* Character = Cast<AGameCharacterBase>(MeshComp->GetOwner());
    if (!Character)
    {
        return;
    }

    UActionManagerComponent* ActionManager = Character->GetActionManagerComponent();
    if (!ActionManager)
    {
        return;
    }

    if (bIsStartNotify)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attack Notify Start: %s"), *AttackName.ToString());
        
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
        DrawDebugSphere(MeshComp->GetWorld(), HitboxCenter, HitboxRadius, 12, FColor::Red, false, 0.5f);
#endif

        if (bHit)
        {
            for (const FHitResult& Hit : HitResults)
            {
                if (AActor* HitActor = Hit.GetActor())
                {
                    if (AGameCharacterBase* HitCharacter = Cast<AGameCharacterBase>(HitActor))
                    {
                        if (HitCharacter != Character)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Hit detected on: %s"), *HitCharacter->GetName());
                            ActionManager->ProcessHitFromAnimation(HitCharacter);
                        }
                    }
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Attack Notify End: %s"), *AttackName.ToString());
    }
}