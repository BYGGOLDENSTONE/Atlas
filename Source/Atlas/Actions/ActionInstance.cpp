#include "ActionInstance.h"
#include "../Characters/GameCharacterBase.h"
#include "../Components/ActionManagerComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/StationIntegrityComponent.h"
#include "../Data/ActionDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

UActionInstance::UActionInstance()
{
	CurrentState = EActionState::Idle;
	CooldownTimer = 0.0f;
	ActionTimer = 0.0f;
	ActionData = nullptr;
	CurrentOwner = nullptr;
	bIsExecuting = false;
	bIsBlocking = false;
	bIsDashing = false;
}

void UActionInstance::Initialize(UActionDataAsset* InActionData)
{
	ActionData = InActionData;
	SetActionState(EActionState::Idle);
	CooldownTimer = 0.0f;
	ActionTimer = 0.0f;
}

bool UActionInstance::CanExecute(AGameCharacterBase* Owner)
{
	if (!Owner || !ActionData)
	{
		return false;
	}

	// Check if on cooldown
	if (CurrentState == EActionState::Cooldown)
	{
		return false;
	}

	// Check if already executing
	if (CurrentState == EActionState::Active)
	{
		return false;
	}

	// Check integrity cost
	if (UStationIntegrityComponent* Integrity = GetStationIntegrity(Owner))
	{
		if (Integrity->GetCurrentIntegrity() < ActionData->IntegrityCost)
		{
			return false;
		}
	}

	// Check tag requirements
	if (IsBlockedByTags(Owner))
	{
		return false;
	}

	if (!HasRequiredTags(Owner))
	{
		return false;
	}

	return true;
}

void UActionInstance::Execute(AGameCharacterBase* Owner)
{
	if (!CanExecute(Owner))
	{
		return;
	}

	CurrentOwner = Owner;
	SetActionState(EActionState::Active);
	ActionTimer = ActionData->ActionDuration;
	bIsExecuting = true;

	// Consume integrity cost
	if (UStationIntegrityComponent* Integrity = GetStationIntegrity(Owner))
	{
		Integrity->ApplyIntegrityDamage(ActionData->IntegrityCost, Owner);
	}

	// Execute based on action type
	switch (ActionData->ActionType)
	{
		case EActionType::Movement:
			ExecuteMovementAction(Owner);
			break;
		case EActionType::Defense:
			ExecuteDefenseAction(Owner);
			break;
		case EActionType::MeleeAttack:
		case EActionType::RangedAttack:
			ExecuteAttackAction(Owner);
			break;
		default:
			ExecuteUtilityAction(Owner);
			break;
	}

	UE_LOG(LogTemp, Log, TEXT("ActionInstance: Executed %s"), 
		*ActionData->ActionName.ToString());
}

void UActionInstance::Update(float DeltaTime)
{
	// Update cooldown
	if (CurrentState == EActionState::Cooldown)
	{
		CooldownTimer -= DeltaTime;
		if (CooldownTimer <= 0.0f)
		{
			SetActionState(EActionState::Idle);
		}
	}

	// Update action timer
	if (CurrentState == EActionState::Active && ActionTimer > 0.0f)
	{
		ActionTimer -= DeltaTime;
		if (ActionTimer <= 0.0f)
		{
			// Auto-complete action
			Stop();
		}
	}
}

void UActionInstance::Stop()
{
	if (CurrentState != EActionState::Active)
	{
		return;
	}

	// Handle specific action cleanup
	if (bIsBlocking)
	{
		bIsBlocking = false;
		// Remove blocking state from ActionManager
		if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent(CurrentOwner))
		{
			ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag("State.Combat.Blocking"));
		}
	}

	if (bIsDashing)
	{
		bIsDashing = false;
		// Remove dashing state
		if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent(CurrentOwner))
		{
			ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag("State.Combat.Dashing"));
		}
	}

	bIsExecuting = false;
	CurrentOwner = nullptr;
	StartCooldown();
}

void UActionInstance::Interrupt()
{
	if (CurrentState == EActionState::Active)
	{
		bIsExecuting = false;
		bIsBlocking = false;
		bIsDashing = false;
		CurrentOwner = nullptr;
		SetActionState(EActionState::Idle);
	}
}

FGameplayTag UActionInstance::GetActionTag() const
{
	return ActionData ? ActionData->ActionTag : FGameplayTag();
}

void UActionInstance::SetActionState(EActionState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnActionStateChanged.Broadcast(NewState);
	}
}

void UActionInstance::StartCooldown()
{
	if (ActionData && ActionData->Cooldown > 0.0f)
	{
		CooldownTimer = ActionData->Cooldown;
		SetActionState(EActionState::Cooldown);
	}
	else
	{
		SetActionState(EActionState::Idle);
	}
}

void UActionInstance::ExecuteMovementAction(AGameCharacterBase* Owner)
{
	// Play movement animation montage if available
	if (ActionData && ActionData->ActionMontage && Owner)
	{
		Owner->PlayAnimMontage(ActionData->ActionMontage, ActionData->MontagePlayRate);
	}

	// Handle dash
	if (ActionData->ActionTag.MatchesTag(FGameplayTag::RequestGameplayTag("Action.Combat.Dash")))
	{
		bIsDashing = true;
		// Note: State.Combat.Dashing tag is managed by animation notifies

		// Apply dash movement
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			FVector DashDirection = Owner->GetActorForwardVector();
			FVector DashVelocity = DashDirection * ActionData->DashDistance / ActionData->DashDuration;
			
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->AddImpulse(DashVelocity, true);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("ActionInstance: Executing Dash - Distance: %.1f"), ActionData->DashDistance);
	}
}

void UActionInstance::ExecuteDefenseAction(AGameCharacterBase* Owner)
{
	// Play defense animation montage if available
	if (ActionData && ActionData->ActionMontage && Owner)
	{
		Owner->PlayAnimMontage(ActionData->ActionMontage, ActionData->MontagePlayRate);
	}

	// Handle block
	if (ActionData->ActionTag.MatchesTag(FGameplayTag::RequestGameplayTag("Action.Combat.Block")))
	{
		bIsBlocking = true;
		// Note: State.Combat.Blocking tag is managed by animation notifies

		UE_LOG(LogTemp, Log, TEXT("ActionInstance: Executing Block - Reduction: %.1f%%"), ActionData->DamageReduction);
	}
}

void UActionInstance::ExecuteAttackAction(AGameCharacterBase* Owner)
{
	// Play attack animation montage if available
	if (ActionData && ActionData->ActionMontage && Owner)
	{
		Owner->PlayAnimMontage(ActionData->ActionMontage, ActionData->MontagePlayRate);
		UE_LOG(LogTemp, Log, TEXT("ActionInstance: Executing Attack - Damage: %.1f"), 
			ActionData->MeleeDamage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionInstance: No montage available for attack"));
	}

	// Note: Actual damage is applied by AttackNotify during the animation
	// when it detects hits and calls ProcessHitFromAnimation
}

void UActionInstance::ExecuteUtilityAction(AGameCharacterBase* Owner)
{
	// Handle utility actions (focus mode, etc.)
	UE_LOG(LogTemp, Log, TEXT("ActionInstance: Executing Utility Action"));
}

UActionManagerComponent* UActionInstance::GetOwnerActionManagerComponent(AGameCharacterBase* Owner) const
{
	return Owner ? Owner->GetActionManagerComponent() : nullptr;
}

UHealthComponent* UActionInstance::GetOwnerHealthComponent(AGameCharacterBase* Owner) const
{
	return Owner ? Owner->GetHealthComponent() : nullptr;
}

UStationIntegrityComponent* UActionInstance::GetStationIntegrity(AGameCharacterBase* Owner) const
{
	return Owner ? Owner->GetStationIntegrityComponent() : nullptr;
}

bool UActionInstance::HasRequiredTags(AGameCharacterBase* Owner) const
{
	if (!ActionData || !Owner)
		return true;

	// Check required tags through ActionManager
	if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent(Owner))
	{
		for (const FGameplayTag& RequiredTag : ActionData->RequiredTags.GetGameplayTagArray())
		{
			if (!ActionManager->HasCombatStateTag(RequiredTag))
			{
				return false;
			}
		}
	}

	return true;
}

bool UActionInstance::IsBlockedByTags(AGameCharacterBase* Owner) const
{
	if (!ActionData || !Owner)
		return false;

	// Check blocked tags through ActionManager
	if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent(Owner))
	{
		for (const FGameplayTag& BlockedTag : ActionData->BlockedDuringTags.GetGameplayTagArray())
		{
			if (ActionManager->HasCombatStateTag(BlockedTag))
			{
				return true;
			}
		}
	}

	return false;
}