#include "MeleeAttackAction.h"
#include "../Characters/GameCharacterBase.h"
#include "../Components/CombatComponent.h"
#include "../Components/HealthComponent.h"
#include "../DataAssets/ActionDataAsset.h"
#include "../Core/AtlasGameplayTags.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

UMeleeAttackAction::UMeleeAttackAction()
{
	ActionTag = FGameplayTag::RequestGameplayTag(FName("Action.Attack.Basic"));
	bAttackInProgress = false;
	CurrentMontage = nullptr;
}

bool UMeleeAttackAction::CanActivate(AGameCharacterBase* Owner)
{
	if (!Super::CanActivate(Owner))
	{
		return false;
	}

	// Check if already attacking
	if (bAttackInProgress)
	{
		return false;
	}

	// Check state restrictions
	if (!CheckAttackRestrictions())
	{
		return false;
	}

	return true;
}

void UMeleeAttackAction::OnActivate(AGameCharacterBase* Owner)
{
	Super::OnActivate(Owner);
	StartAttack();
}

void UMeleeAttackAction::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);
	
	// Attack is animation-driven, no tick logic needed
}

void UMeleeAttackAction::OnRelease()
{
	// Melee attacks don't respond to release (they're single press)
}

void UMeleeAttackAction::OnInterrupted()
{
	if (bAttackInProgress)
	{
		EndAttack();
	}
	Super::OnInterrupted();
}

void UMeleeAttackAction::StartAttack()
{
	if (!CurrentOwner)
	{
		EndAttack();
		return;
	}

	ACharacter* Character = Cast<ACharacter>(CurrentOwner);
	if (!Character)
	{
		EndAttack();
		return;
	}

	// Get the attack tag from data asset or use default
	CurrentAttackTag = GetAttackTag();

	// Use CombatComponent to start the attack (maintains existing pipeline)
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		if (CombatComp->StartAttack(CurrentAttackTag))
		{
			bAttackInProgress = true;
			
			// Get and play the attack montage
			CurrentMontage = GetAttackMontage();
			if (CurrentMontage)
			{
				if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
				{
					// Play montage and bind to end event
					AnimInstance->Montage_Play(CurrentMontage);
					
					// Bind to montage end
					FOnMontageEnded MontageEndedDelegate;
					MontageEndedDelegate.BindUObject(this, &UMeleeAttackAction::OnMontageEnded);
					AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, CurrentMontage);
				}
			}
			else
			{
				// No montage, end attack immediately
				EndAttack();
			}
		}
		else
		{
			// Failed to start attack through combat component
			EndAttack();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MeleeAttackAction: No CombatComponent found"));
		EndAttack();
	}
}

void UMeleeAttackAction::EndAttack()
{
	bAttackInProgress = false;
	bIsActive = false;

	// End attack in combat component
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		CombatComp->EndAttack();
	}

	// Stop montage if still playing
	if (CurrentOwner && CurrentMontage)
	{
		ACharacter* Character = Cast<ACharacter>(CurrentOwner);
		if (Character && Character->GetMesh())
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
			{
				if (AnimInstance->Montage_IsPlaying(CurrentMontage))
				{
					AnimInstance->Montage_Stop(0.2f, CurrentMontage);
				}
			}
		}
	}

	CurrentMontage = nullptr;
	CurrentAttackTag = FGameplayTag::EmptyTag;

	// Start cooldown
	StartCooldown();
}

FGameplayTag UMeleeAttackAction::GetAttackTag() const
{
	// Check if data asset specifies a custom attack tag
	if (ActionData)
	{
		// Check custom parameters for attack type
		if (ActionData->CustomStringParameters.Contains("AttackType"))
		{
			FString AttackType = ActionData->CustomStringParameters["AttackType"];
			if (AttackType == "Heavy")
			{
				return FGameplayTag::RequestGameplayTag(FName("Attack.Heavy"));
			}
		}
	}

	// Default to basic attack
	return FGameplayTag::RequestGameplayTag(FName("Attack.Light"));
}

UAnimMontage* UMeleeAttackAction::GetAttackMontage() const
{
	// Use the montage from the action data asset
	if (ActionData && ActionData->ActionMontage)
	{
		return ActionData->ActionMontage;
	}

	// Or get from combat component based on attack tag
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		return CombatComp->GetAttackMontage(CurrentAttackTag);
	}

	return nullptr;
}

bool UMeleeAttackAction::CheckAttackRestrictions() const
{
	if (!CurrentOwner)
	{
		return false;
	}

	// Check if stunned or staggered
	if (UHealthComponent* HealthComp = GetOwnerHealthComponent())
	{
		if (HealthComp->IsStaggered())
		{
			return false;
		}
	}

	// Check if already in another attack
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		if (CombatComp->IsAttacking())
		{
			return false;
		}
	}

	return true;
}

void UMeleeAttackAction::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == CurrentMontage)
	{
		EndAttack();
	}
}