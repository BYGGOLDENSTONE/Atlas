#include "ParryWindowNotifyState.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Components/CombatComponent.h"
#include "../Characters/GameCharacterBase.h"

UParryWindowNotifyState::UParryWindowNotifyState()
{
    // Keep constructor empty to avoid crashes
}

void UParryWindowNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

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

    if (bIsAttackerWindow)
    {
        CombatComp->SetAttackerParryWindow(true);
        UE_LOG(LogTemp, Warning, TEXT("Attacker Parry Window Open (%.2fs)"), TotalDuration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Defender Parry Window Active (%.2fs)"), TotalDuration);
    }
}

void UParryWindowNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

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

    if (bIsAttackerWindow)
    {
        CombatComp->SetAttackerParryWindow(false);
        UE_LOG(LogTemp, Warning, TEXT("Attacker Parry Window Closed"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Defender Parry Window Ended"));
    }
}