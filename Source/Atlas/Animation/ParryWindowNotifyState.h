#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "ParryWindowNotifyState.generated.h"

UCLASS(Blueprintable, meta=(DisplayName="Parry Window Notify State"))
class ATLAS_API UParryWindowNotifyState : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    UParryWindowNotifyState();

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
    bool bIsAttackerWindow = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
    float WindowDuration = 0.3f;
};