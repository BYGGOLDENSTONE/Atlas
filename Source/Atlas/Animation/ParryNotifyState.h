#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "ParryNotifyState.generated.h"

/**
 * ParryNotifyState handles the parry window timing and detection
 * Perfect parry window: 0.2s from start - negates all damage and applies vulnerability
 * Late parry window: 0.1s after perfect - reduces 50% damage
 */
UCLASS()
class ATLAS_API UParryNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UParryNotifyState();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	
	virtual FString GetNotifyName_Implementation() const override;

protected:
	/** Duration of perfect parry window in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
	float PerfectParryWindow = 0.2f;

	/** Duration of late parry window after perfect window */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
	float LateParryWindow = 0.1f;

	/** Vulnerability tier to apply to attacker on perfect parry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parry")
	FGameplayTag VulnerabilityTag;

	/** VFX to spawn on successful parry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class UParticleSystem* ParrySuccessVFX;

	/** VFX to spawn on perfect parry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class UParticleSystem* PerfectParryVFX;

	/** Time dilation for perfect parry effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	float PerfectParryTimeDilation = 0.5f;

	/** Duration of time dilation effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	float TimeDilationDuration = 0.2f;

private:
	/** Track current time in parry window */
	float CurrentParryTime;

	/** Check if we're in perfect parry window */
	bool IsInPerfectWindow() const;

	/** Check if we're in late parry window */
	bool IsInLateWindow() const;

	/** Handle successful parry */
	void OnParrySuccess(class AGameCharacterBase* Character, class AActor* Attacker, bool bPerfectParry);

	/** Apply time dilation effect for perfect parry */
	void ApplyTimeDilationEffect(UWorld* World);

	/** Clear time dilation effect */
	void ClearTimeDilationEffect(UWorld* World);

	/** Timer handle for clearing time dilation */
	FTimerHandle TimeDilationTimerHandle;

#if WITH_EDITOR
	/** Editor color for this notify */
	virtual FLinearColor GetEditorColor() { return FLinearColor(0.0f, 0.5f, 1.0f); } // Blue for parry
#endif
};