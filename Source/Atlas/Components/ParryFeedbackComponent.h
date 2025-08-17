#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParryFeedbackComponent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;
class UCombatComponent;
class UVulnerabilityComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UParryFeedbackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UParryFeedbackComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry VFX")
    UNiagaraSystem* ParrySuccessVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry VFX")
    FVector ParryVFXOffset = FVector(0.0f, 0.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry VFX")
    float ParryVFXScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vulnerability VFX")
    UNiagaraSystem* VulnerabilityVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vulnerability VFX")
    FVector VulnerabilityVFXOffset = FVector(0.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vulnerability VFX")
    float VulnerabilityVFXScale = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IFrame VFX")
    UNiagaraSystem* IFrameVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry Audio")
    USoundBase* ParrySuccessSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry Audio")
    float ParrySuccessSoundVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vulnerability Audio")
    USoundBase* VulnerabilityAppliedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vulnerability Audio")
    USoundBase* VulnerabilityEndedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IFrame Audio")
    USoundBase* IFrameSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    TSubclassOf<class UCameraShakeBase> ParrySuccessCameraShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    float SlowMotionScale = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    float SlowMotionDuration = 0.2f;

    UFUNCTION(BlueprintCallable, Category = "Feedback")
    void PlayParrySuccessFeedback(AActor* Attacker);

    UFUNCTION(BlueprintCallable, Category = "Feedback")
    void PlayVulnerabilityAppliedFeedback();

    UFUNCTION(BlueprintCallable, Category = "Feedback")
    void PlayVulnerabilityEndedFeedback();

    UFUNCTION(BlueprintCallable, Category = "Feedback")
    void PlayIFrameFeedback();

    UFUNCTION(BlueprintCallable, Category = "Feedback")
    void TriggerSlowMotion();

private:
    UPROPERTY()
    UNiagaraComponent* ActiveVulnerabilityVFX;

    UPROPERTY()
    UCombatComponent* CombatComponent;

    UPROPERTY()
    UVulnerabilityComponent* VulnerabilityComponent;

    FTimerHandle SlowMotionTimerHandle;

    void SetupEventBindings();
    void EndSlowMotion();

    UFUNCTION()
    void OnParrySuccess(AActor* Attacker);

    UFUNCTION()
    void OnVulnerabilityStateApplied(int32 Charges, float Duration);

    UFUNCTION()
    void OnVulnerabilityEnded();

    UFUNCTION()
    void OnIFramesStarted();
};