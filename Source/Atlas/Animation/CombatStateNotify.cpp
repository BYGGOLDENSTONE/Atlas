#include "CombatStateNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Components/CombatComponent.h"
#include "../Characters/GameCharacterBase.h"
#include "../Core/AtlasGameplayTags.h"

UCombatStateNotify::UCombatStateNotify()
{
	// Default to attacking state
	StateTag = FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking"));
	bAddState = true;
}

void UCombatStateNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

	UCombatComponent* CombatComp = Character->GetCombatComponent();
	if (!CombatComp)
	{
		return;
	}

	if (bAddState)
	{
		// Clear other attack states if requested
		if (bClearOtherAttackStates)
		{
			CombatComp->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")));
			CombatComp->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")));
			CombatComp->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Combat.State.Dashing")));
		}
		
		// Add the new state
		CombatComp->AddCombatStateTag(StateTag);
		UE_LOG(LogTemp, Log, TEXT("CombatStateNotify: Added state %s"), *StateTag.ToString());
	}
	else
	{
		// Remove the state
		CombatComp->RemoveCombatStateTag(StateTag);
		UE_LOG(LogTemp, Log, TEXT("CombatStateNotify: Removed state %s"), *StateTag.ToString());
		
		// If we're ending an attack, also clear the action data
		if (StateTag == FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")))
		{
			CombatComp->SetCurrentActionData(nullptr);
		}
	}
}

FString UCombatStateNotify::GetNotifyName_Implementation() const
{
	if (bAddState)
	{
		return FString::Printf(TEXT("Set: %s"), *StateTag.ToString());
	}
	else
	{
		return FString::Printf(TEXT("Clear: %s"), *StateTag.ToString());
	}
}