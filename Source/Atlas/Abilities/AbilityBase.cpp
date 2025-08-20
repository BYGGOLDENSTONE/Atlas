#include "AbilityBase.h"
#include "GameFramework/Character.h"
#include "../Components/CombatComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/StationIntegrityComponent.h"
#include "../Core/AtlasGameState.h"
#include "Engine/World.h"

UAbilityBase::UAbilityBase()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentState = EAbilityState::Ready;
	CooldownTimer = 0.0f;
	ExecutionTimer = 0.0f;
}

void UAbilityBase::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		CombatComponent = OwnerCharacter->FindComponentByClass<UCombatComponent>();
		HealthComponent = OwnerCharacter->FindComponentByClass<UHealthComponent>();
	}
}

void UAbilityBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentState == EAbilityState::Cooldown)
	{
		UpdateCooldown(DeltaTime);
	}
	else if (CurrentState == EAbilityState::Executing)
	{
		ExecutionTimer -= DeltaTime;
		if (ExecutionTimer <= 0.0f)
		{
			OnAbilityEnd();
		}
	}
}

bool UAbilityBase::TryExecuteAbility()
{
	if (!CanExecuteAbility())
	{
		return false;
	}

	// Check integrity cost for medium/high risk abilities
	if (IntegrityCost > 0.0f)
	{
		if (UStationIntegrityComponent* IntegrityComp = GetStationIntegrity())
		{
			// Just apply the damage without checking if we can afford it
			// The integrity component will handle the failure state
			IntegrityComp->ApplyIntegrityDamage(IntegrityCost, GetOwner());
		}
	}

	SetAbilityState(EAbilityState::Executing);
	ExecutionTimer = ExecutionDuration;
	ExecuteAbility();
	OnAbilityExecuted.Broadcast();

	return true;
}

bool UAbilityBase::CanExecuteAbility() const
{
	if (CurrentState != EAbilityState::Ready)
	{
		return false;
	}

	if (!OwnerCharacter || !OwnerCharacter->CanJump())
	{
		return false;
	}

	// Check combat state restrictions
	if (CombatComponent)
	{
		if (CombatComponent->IsBlocking() || CombatComponent->IsAttacking())
		{
			return false;
		}
	}

	// Check health state restrictions
	if (HealthComponent)
	{
		if (HealthComponent->IsStaggered() || HealthComponent->IsDead())
		{
			return false;
		}
	}

	// Check required/blocked tags
	// TODO: Implement tag checking when GameCharacterBase has tag support
	// For now, skip tag checks since base ACharacter doesn't have gameplay tags

	return CheckAbilitySpecificConditions();
}

float UAbilityBase::GetCooldownPercent() const
{
	if (Cooldown <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(CooldownTimer / Cooldown, 0.0f, 1.0f);
}

void UAbilityBase::ExecuteAbility()
{
	// Override in derived classes
}

void UAbilityBase::OnAbilityEnd()
{
	StartCooldown();
}

void UAbilityBase::StartCooldown()
{
	if (Cooldown > 0.0f)
	{
		CooldownTimer = Cooldown;
		SetAbilityState(EAbilityState::Cooldown);
	}
	else
	{
		SetAbilityState(EAbilityState::Ready);
	}
}

void UAbilityBase::UpdateCooldown(float DeltaTime)
{
	CooldownTimer -= DeltaTime;
	OnAbilityCooldownUpdate.Broadcast(CooldownTimer);

	if (CooldownTimer <= 0.0f)
	{
		CooldownTimer = 0.0f;
		SetAbilityState(EAbilityState::Ready);
	}
}

void UAbilityBase::SetAbilityState(EAbilityState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnAbilityStateChanged.Broadcast(NewState);
	}
}

UStationIntegrityComponent* UAbilityBase::GetStationIntegrity() const
{
	if (UWorld* World = GetWorld())
	{
		if (AAtlasGameState* GameState = World->GetGameState<AAtlasGameState>())
		{
			return GameState->GetStationIntegrityComponent();
		}
	}
	return nullptr;
}