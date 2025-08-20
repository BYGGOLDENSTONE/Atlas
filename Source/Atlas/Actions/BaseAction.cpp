#include "BaseAction.h"
#include "../Characters/GameCharacterBase.h"
#include "../Components/CombatComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/StationIntegrityComponent.h"
#include "../DataAssets/ActionDataAsset.h"
#include "../Interfaces/ICombatInterface.h"
#include "../Interfaces/IHealthInterface.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"

UBaseAction::UBaseAction()
{
	bIsActive = false;
	CurrentCooldown = 0.0f;
	CurrentState = EActionState::Idle;
	CurrentOwner = nullptr;
	ActionData = nullptr;
	CachedCombatComponent = nullptr;
	CachedHealthComponent = nullptr;
	CachedStationIntegrity = nullptr;
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
		UE_LOG(LogTemp, Warning, TEXT("BaseAction::OnActivate called with null Owner"));
		return;
	}

	CurrentOwner = Owner;
	bIsActive = true;
	SetActionState(EActionState::Active);

	// Cache component references for this activation
	CachedCombatComponent = Owner->GetCombatComponent();
	CachedHealthComponent = Owner->GetHealthComponent();
	
	// Safely get station integrity from game state
	CachedStationIntegrity = nullptr;
	if (UWorld* World = Owner->GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			CachedStationIntegrity = GameState->FindComponentByClass<UStationIntegrityComponent>();
		}
	}

	// Check integrity cost
	if (ActionData && ActionData->IntegrityCost > 0.0f && CachedStationIntegrity)
	{
		CachedStationIntegrity->ApplyIntegrityDamage(ActionData->IntegrityCost, Owner);
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
		
		// Clear cached references
		CachedCombatComponent = nullptr;
		CachedHealthComponent = nullptr;
		CachedStationIntegrity = nullptr;
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
		
		// Clear cached references
		CachedCombatComponent = nullptr;
		CachedHealthComponent = nullptr;
		CachedStationIntegrity = nullptr;
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
	// Try cached reference first, then get from owner
	if (CachedCombatComponent)
	{
		return CachedCombatComponent;
	}
	if (CurrentOwner)
	{
		return CurrentOwner->GetCombatComponent();
	}
	return nullptr;
}

UHealthComponent* UBaseAction::GetOwnerHealthComponent() const
{
	// Try cached reference first, then get from owner
	if (CachedHealthComponent)
	{
		return CachedHealthComponent;
	}
	if (CurrentOwner)
	{
		return CurrentOwner->GetHealthComponent();
	}
	return nullptr;
}

UStationIntegrityComponent* UBaseAction::GetStationIntegrity() const
{
	return CachedStationIntegrity;
}

bool UBaseAction::HasRequiredTags(AGameCharacterBase* Owner) const
{
	if (!ActionData || !Owner)
	{
		return true;
	}

	// Check if owner has required tags using interface
	if (ActionData->RequiredTags.Num() > 0)
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Owner))
		{
			for (const FGameplayTag& RequiredTag : ActionData->RequiredTags)
			{
				if (!CombatInterface->HasCombatStateTag(RequiredTag))
				{
					return false;
				}
			}
		}
	}
	return true;
}

bool UBaseAction::IsBlockedByTags(AGameCharacterBase* Owner) const
{
	if (!ActionData || !Owner)
	{
		return false;
	}

	// Check if owner has any blocking tags using interface
	if (ActionData->BlockedDuringTags.Num() > 0)
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Owner))
		{
			for (const FGameplayTag& BlockedTag : ActionData->BlockedDuringTags)
			{
				if (CombatInterface->HasCombatStateTag(BlockedTag))
				{
					return true;
				}
			}
		}
	}
	return false;
}