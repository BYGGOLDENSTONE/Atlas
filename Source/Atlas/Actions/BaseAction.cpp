#include "BaseAction.h"
#include "../Characters/GameCharacterBase.h"
#include "../Components/CombatComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/StationIntegrityComponent.h"
#include "../DataAssets/ActionDataAsset.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"

UBaseAction::UBaseAction()
{
	bIsActive = false;
	CurrentCooldown = 0.0f;
	CurrentState = EActionState::Idle;
	CurrentOwner = nullptr;
	ActionData = nullptr;
}

bool UBaseAction::CanActivate(AGameCharacterBase* Owner)
{
	if (!Owner)
	{
		return false;
	}

	// Check cooldown
	if (CurrentCooldown > 0.0f)
	{
		return false;
	}

	// Check if already active
	if (bIsActive)
	{
		return false;
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

void UBaseAction::OnActivate(AGameCharacterBase* Owner)
{
	if (!Owner)
	{
		return;
	}

	CurrentOwner = Owner;
	bIsActive = true;
	SetActionState(EActionState::Active);

	// Check integrity cost
	if (ActionData && ActionData->IntegrityCost > 0.0f)
	{
		if (UStationIntegrityComponent* IntegrityComp = GetStationIntegrity())
		{
			IntegrityComp->ApplyIntegrityDamage(ActionData->IntegrityCost, Owner);
		}
	}
}

void UBaseAction::OnTick(float DeltaTime)
{
	// Update cooldown if not active
	if (!bIsActive && CurrentCooldown > 0.0f)
	{
		UpdateCooldown(DeltaTime);
	}
}

void UBaseAction::OnRelease()
{
	if (bIsActive)
	{
		bIsActive = false;
		StartCooldown();
		SetActionState(EActionState::Idle);
	}
}

void UBaseAction::OnInterrupted()
{
	if (bIsActive)
	{
		bIsActive = false;
		// Interrupted actions typically have reduced cooldown
		if (ActionData)
		{
			CurrentCooldown = ActionData->Cooldown * 0.5f;
		}
		SetActionState(EActionState::Cooldown);
	}
}

void UBaseAction::SetDataAsset(UActionDataAsset* InDataAsset)
{
	ActionData = InDataAsset;
	if (ActionData)
	{
		ActionTag = ActionData->ActionTag;
	}
}

void UBaseAction::UpdateCooldown(float DeltaTime)
{
	if (CurrentCooldown > 0.0f)
	{
		CurrentCooldown = FMath::Max(0.0f, CurrentCooldown - DeltaTime);
		if (CurrentCooldown == 0.0f)
		{
			SetActionState(EActionState::Idle);
		}
	}
}

void UBaseAction::StartCooldown()
{
	if (ActionData)
	{
		CurrentCooldown = ActionData->Cooldown;
		if (CurrentCooldown > 0.0f)
		{
			SetActionState(EActionState::Cooldown);
		}
	}
}

void UBaseAction::SetActionState(EActionState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnActionStateChanged.Broadcast(NewState);
	}
}

UCombatComponent* UBaseAction::GetOwnerCombatComponent() const
{
	if (CurrentOwner)
	{
		return CurrentOwner->FindComponentByClass<UCombatComponent>();
	}
	return nullptr;
}

UHealthComponent* UBaseAction::GetOwnerHealthComponent() const
{
	if (CurrentOwner)
	{
		return CurrentOwner->FindComponentByClass<UHealthComponent>();
	}
	return nullptr;
}

UStationIntegrityComponent* UBaseAction::GetStationIntegrity() const
{
	if (CurrentOwner && CurrentOwner->GetWorld())
	{
		if (AGameStateBase* GameState = CurrentOwner->GetWorld()->GetGameState())
		{
			return GameState->FindComponentByClass<UStationIntegrityComponent>();
		}
	}
	return nullptr;
}

bool UBaseAction::HasRequiredTags(AGameCharacterBase* Owner) const
{
	if (!ActionData || !Owner)
	{
		return true;
	}

	// For now, just return true - we'll implement tag checking when we have the character tag system
	return true;
}

bool UBaseAction::IsBlockedByTags(AGameCharacterBase* Owner) const
{
	if (!ActionData || !Owner)
	{
		return false;
	}

	// For now, just return false - we'll implement tag checking when we have the character tag system
	return false;
}