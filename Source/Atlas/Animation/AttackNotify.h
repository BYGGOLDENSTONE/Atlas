#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AttackNotify.generated.h"

UCLASS(Blueprintable, meta=(DisplayName="Attack Notify"))
class ATLAS_API UAttackNotify : public UAnimNotify
{
    GENERATED_BODY()

public:
    UAttackNotify();

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    FName AttackName = "Jab";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    bool bIsStartNotify = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float HitboxRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float HitboxForwardOffset = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    FName SocketName = "hand_r";
};