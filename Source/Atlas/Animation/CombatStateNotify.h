#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "CombatStateNotify.generated.h"

/**
 * Animation notify for managing combat states
 * Place at the start and end of attack animations to properly set/clear states
 */
UCLASS(Blueprintable, meta=(DisplayName="Combat State Notify"))
class ATLAS_API UCombatStateNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UCombatStateNotify();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	// The combat state tag to add or remove
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
	FGameplayTag StateTag;

	// Whether to add (true) or remove (false) the state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
	bool bAddState = true;

	// Optional: Clear other attack states when adding
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
	bool bClearOtherAttackStates = false;
};