#include "CombatStateNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Components/ActionManagerComponent.h"
#include "../Characters/GameCharacterBase.h"
#include "../Characters/PlayerCharacter.h"
#include "../Core/AtlasGameplayTags.h"

UCombatStateNotify::UCombatStateNotify()
{
	// Default to attacking state - using clean tag structure
	StateTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Attacking"));
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

	UActionManagerComponent* ActionManager = Character->GetActionManagerComponent();
	if (!ActionManager)
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
			ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.Attacking")));
			ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.Blocking")));
			ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.Dashing")));
		}
		
		// Add the new state
		ActionManager->AddCombatStateTag(StateTag);
		// Added combat state
		
		// If this is an ability state and we're on a player, disable inputs
		if (PlayerChar)
		{
			// Disable inputs for any ability that should block other inputs
			if (StateTag == FGameplayTag::RequestGameplayTag(FName("State.Combat.Attacking")) ||
				StateTag == FGameplayTag::RequestGameplayTag(FName("State.Combat.Blocking")) ||
				StateTag == FGameplayTag::RequestGameplayTag(FName("State.Combat.Dashing")))
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
		ActionManager->RemoveCombatStateTag(StateTag);
		// Removed combat state
		
		// If we're ending an ability state on a player, re-enable inputs
		if (PlayerChar)
		{
			if (StateTag == FGameplayTag::RequestGameplayTag(FName("State.Combat.Attacking")) ||
				StateTag == FGameplayTag::RequestGameplayTag(FName("State.Combat.Blocking")) ||
				StateTag == FGameplayTag::RequestGameplayTag(FName("State.Combat.Dashing")))
			{
				PlayerChar->SetAbilityInputsEnabled(true);
				PlayerChar->SetMovementInputEnabled(true);
				// Re-enabled inputs after ability completion
				
				// Clear action data if it was an attack
				if (StateTag == FGameplayTag::RequestGameplayTag(FName("State.Combat.Attacking")))
				{
					ActionManager->SetCurrentActionData(nullptr);
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