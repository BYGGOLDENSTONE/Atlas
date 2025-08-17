#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AttackNotifyState.generated.h"

UCLASS(Blueprintable, meta=(DisplayName="Attack Notify State"))
class ATLAS_API UAttackNotifyState : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    UAttackNotifyState();

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    FName AttackName = "Jab";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float HitboxRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float HitboxForwardOffset = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    FName SocketName = "hand_r";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    bool bContinuousHitDetection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float HitDetectionInterval = 0.1f;

private:
    UPROPERTY()
    float TimeSinceLastCheck;
    
    UPROPERTY()
    TArray<AActor*> AlreadyHitActors;
    
    void PerformHitDetection(USkeletalMeshComponent* MeshComp);
};