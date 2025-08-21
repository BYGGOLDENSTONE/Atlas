#include "CombatStateNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Components/CombatComponent.h"
#include "../Characters/GameCharacterBase.h"
#include "../Characters/PlayerCharacter.h"
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

	// Handle input blocking for PlayerCharacter
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Character);
	
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
		// Added combat state
		
		// If this is an ability state and we're on a player, disable inputs
		if (PlayerChar)
		{
			// Disable inputs for any ability that should block other inputs
			if (StateTag == FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")) ||
				StateTag == FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")) ||
				StateTag == FGameplayTag::RequestGameplayTag(FName("Combat.State.Dashing")))
			{
				PlayerChar->SetAbilityInputsEnabled(false);
				PlayerChar->SetMovementInputEnabled(false);
				// Disabled inputs for ability execution
			}
		}
	}
	else
	{
		// Remove the state
		CombatComp->RemoveCombatStateTag(StateTag);
		// Removed combat state
		
		// If we're ending an ability state on a player, re-enable inputs
		if (PlayerChar)
		{
			if (StateTag == FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")) ||
				StateTag == FGameplayTag::RequestGameplayTag(FName("Combat.State.Blocking")) ||
				StateTag == FGameplayTag::RequestGameplayTag(FName("Combat.State.Dashing")))
			{
				PlayerChar->SetAbilityInputsEnabled(true);
				PlayerChar->SetMovementInputEnabled(true);
				// Re-enabled inputs after ability completion
				
				// Clear action data if it was an attack
				if (StateTag == FGameplayTag::RequestGameplayTag(FName("Combat.State.Attacking")))
				{
					CombatComp->SetCurrentActionData(nullptr);
				}
			}
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