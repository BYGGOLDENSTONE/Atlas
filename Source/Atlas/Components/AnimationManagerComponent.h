#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AnimationManagerComponent.generated.h"

class UAnimMontage;
class UAnimInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnimationMontageStarted, UAnimMontage*, Montage, const FGameplayTag&, AnimationTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnimationMontageEnded, UAnimMontage*, Montage, bool, bInterrupted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimationMontageBlendOut, UAnimMontage*, Montage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMontageNotify, const FName&, NotifyName, const FGameplayTag&, AnimationTag);

USTRUCT(BlueprintType)
struct FAnimationRequest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    UAnimMontage* Montage = nullptr;

    UPROPERTY(BlueprintReadWrite)
    FGameplayTag AnimationTag;

    UPROPERTY(BlueprintReadWrite)
    float PlayRate = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float StartPosition = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    FName StartSection = NAME_None;

    UPROPERTY(BlueprintReadWrite)
    bool bStopAllMontages = true;

    UPROPERTY(BlueprintReadWrite)
    int32 Priority = 0;

    UPROPERTY(BlueprintReadWrite)
    bool bCanBeInterrupted = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UAnimationManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationManagerComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TMap<FGameplayTag, UAnimMontage*> MontageMap;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    UAnimMontage* CurrentMontage;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    FGameplayTag CurrentAnimationTag;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    int32 CurrentPriority = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsPlayingAnimation = false;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayAnimationByTag(const FGameplayTag& AnimationTag, float PlayRate = 1.0f, FName StartSection = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f, FName StartSection = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayAnimationRequest(const FAnimationRequest& Request);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopCurrentMontage(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopAllMontages(float BlendOutTime = 0.25f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsPlayingAnimation() const { return bIsPlayingAnimation; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsPlayingAnimationTag(const FGameplayTag& AnimationTag) const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetCurrentMontagePosition() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetCurrentMontageLength() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMontagePosition(float Position);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetPlayRate(float PlayRate);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PauseMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ResumeMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool CanInterruptCurrentAnimation() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void RegisterMontage(const FGameplayTag& AnimationTag, UAnimMontage* Montage);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    UAnimMontage* GetMontageByTag(const FGameplayTag& AnimationTag) const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void HandleHitReaction(float Damage, const FVector& HitDirection);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void HandleStagger(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void HandleBlock(bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void HandleDeath();

    UPROPERTY(BlueprintAssignable, Category = "Animation Events")
    FOnAnimationMontageStarted OnAnimationMontageStarted;

    UPROPERTY(BlueprintAssignable, Category = "Animation Events")
    FOnAnimationMontageEnded OnAnimationMontageEnded;

    UPROPERTY(BlueprintAssignable, Category = "Animation Events")
    FOnAnimationMontageBlendOut OnAnimationMontageBlendOut;

    UPROPERTY(BlueprintAssignable, Category = "Animation Events")
    FOnMontageNotify OnMontageNotify;

private:
    UPROPERTY()
    UAnimInstance* AnimInstance;

    UPROPERTY()
    class ACharacter* OwnerCharacter;

    FAnimationRequest CurrentRequest;
    bool bCanInterruptCurrent = true;

    UFUNCTION()
    void OnMontageEndedCallback(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnMontageBlendOutCallback(UAnimMontage* Montage, bool bInterrupted);

    void BindMontageEvents(UAnimMontage* Montage);
    void UnbindMontageEvents(UAnimMontage* Montage);
    bool CanPlayAnimation(const FAnimationRequest& Request) const;
    void UpdateAnimationState(bool bPlaying, UAnimMontage* Montage = nullptr, const FGameplayTag& Tag = FGameplayTag());
};