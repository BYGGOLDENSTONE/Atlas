#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ComboWindowNotifyState.generated.h"

/**
 * Animation notify state for combo input windows
 * Place this where the player can buffer their next attack input
 */
UCLASS(Blueprintable, meta=(DisplayName="Combo Window Notify State"))
class ATLAS_API UComboWindowNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UComboWindowNotifyState();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	// Identifier for this combo window (e.g., "Light1", "Light2", "Heavy1")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FName ComboWindowName = "Default";

	// Whether this window allows chaining into any attack or specific ones
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	bool bAllowAnyAttack = true;

	// If not allowing any attack, specify which attacks can be chained
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo", meta=(EditCondition="!bAllowAnyAttack"))
	TArray<FName> AllowedNextAttacks;
};