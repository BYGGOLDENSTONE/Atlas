#include "AbilityAction.h"
#include "../Abilities/AbilityBase.h"
#include "../Characters/GameCharacterBase.h"
#include "../DataAssets/ActionDataAsset.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UAbilityAction::UAbilityAction()
{
	// Ability action properties set via data asset
	AbilityInstance = nullptr;
}

bool UAbilityAction::CanActivate(AGameCharacterBase* Owner)
{
	if (!Super::CanActivate(Owner))
	{
		return false;
	}

	// Check if ability can be used
	if (AbilityInstance)
	{
		return AbilityInstance->CanExecuteAbility();
	}

	return true;
}

void UAbilityAction::OnActivate(AGameCharacterBase* Owner)
{
	Super::OnActivate(Owner);

	// Get ability class from data asset if not set
	if (!AbilityClass && ActionData && ActionData->AbilityClass)
	{
		AbilityClass = ActionData->AbilityClass;
	}

	// Create ability instance if needed
	if (!AbilityInstance && AbilityClass && Owner)
	{
		CreateAbilityInstance();
	}

	// Execute the ability
	if (AbilityInstance)
	{
		AbilityInstance->TryExecuteAbility();
	}
}

void UAbilityAction::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);

	// Check if ability execution is complete
	if (AbilityInstance && bIsActive)
	{
		// Most abilities are instant or self-managing
		// Check if we should end this action based on ability state
		if (AbilityInstance->GetAbilityState() != EAbilityState::Executing)
		{
			OnRelease();
		}
	}
}

void UAbilityAction::OnRelease()
{
	if (bIsActive)
	{
		bIsActive = false;
		StartCooldown();
		SetActionState(EActionState::Cooldown);
	}
}

void UAbilityAction::OnInterrupted()
{
	if (AbilityInstance && AbilityInstance->GetAbilityState() == EAbilityState::Executing)
	{
		// Let ability handle its own interruption
		AbilityInstance->ForceEndAbility();
	}
	
	OnRelease();
}

void UAbilityAction::SetAbilityClass(TSubclassOf<UAbilityBase> InAbilityClass)
{
	AbilityClass = InAbilityClass;
}

void UAbilityAction::CreateAbilityInstance()
{
	if (!CurrentOwner || !AbilityClass)
	{
		return;
	}

	// Create the ability component
	AbilityInstance = NewObject<UAbilityBase>(CurrentOwner, AbilityClass);
	if (AbilityInstance)
	{
		AbilityInstance->RegisterComponent();
		
		// Configure from action data if available
		if (ActionData && ActionData->IntegrityCost > 0.0f)
		{
			// Pass integrity cost to ability
			AbilityInstance->SetIntegrityCost(ActionData->IntegrityCost);
		}
	}
}

void UAbilityAction::DestroyAbilityInstance()
{
	if (AbilityInstance)
	{
		AbilityInstance->DestroyComponent();
		AbilityInstance = nullptr;
	}
}