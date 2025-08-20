#include "UniversalAction.h"
#include "../Characters/GameCharacterBase.h"
#include "../Components/CombatComponent.h"
#include "../Components/HealthComponent.h"
#include "../DataAssets/ActionDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UUniversalAction::UUniversalAction()
{
	bIsBlocking = false;
	bIsDashing = false;
	bIsAttacking = false;
	DashTimer = 0.0f;
	AttackTimer = 0.0f;
}

bool UUniversalAction::CanActivate(AGameCharacterBase* Owner)
{
	if (!Super::CanActivate(Owner))
	{
		return false;
	}

	// Add any universal checks here
	return true;
}

void UUniversalAction::OnActivate(AGameCharacterBase* Owner)
{
	Super::OnActivate(Owner);

	if (!ActionData)
	{
		return;
	}

	// Route to appropriate execution based on action tag
	if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Action.Ability.Dash")))
	{
		ExecuteDash();
	}
	else if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Action.Ability.Block")))
	{
		ExecuteBlock();
	}
	else if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Action.Ability.BasicAttack")) ||
	         ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Action.Ability.HeavyAttack")))
	{
		ExecuteMeleeAttack();
	}
	else
	{
		// Generic ability execution
		ExecuteGenericAbility();
	}
}

void UUniversalAction::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);

	// Handle ongoing states
	if (bIsDashing)
	{
		DashTimer -= DeltaTime;
		if (DashTimer <= 0.0f)
		{
			bIsDashing = false;
			OnRelease();
		}
	}
	
	if (bIsAttacking)
	{
		AttackTimer -= DeltaTime;
		if (AttackTimer <= 0.0f)
		{
			bIsAttacking = false;
			OnRelease();
		}
	}
}

void UUniversalAction::OnRelease()
{
	// Handle release for hold actions (like block)
	if (bIsBlocking)
	{
		if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
		{
			CombatComp->EndBlock();
		}
		bIsBlocking = false;
	}

	if (bIsActive)
	{
		bIsActive = false;
		StartCooldown();
		SetActionState(EActionState::Cooldown);
	}
}

void UUniversalAction::OnInterrupted()
{
	bIsBlocking = false;
	bIsDashing = false;
	bIsAttacking = false;
	
	OnRelease();
}

void UUniversalAction::ExecuteDash()
{
	if (!CurrentOwner)
		return;

	ACharacter* Character = Cast<ACharacter>(CurrentOwner);
	if (!Character || !Character->GetCharacterMovement())
		return;

	// Get movement direction
	FVector InputVector = Character->GetCharacterMovement()->GetLastInputVector();
	if (InputVector.IsNearlyZero())
	{
		// Dash forward if no input
		InputVector = Character->GetActorForwardVector();
	}
	
	// Apply dash
	float DashDistance = ActionData ? ActionData->DashDistance : 400.0f;
	FVector DashVelocity = InputVector.GetSafeNormal() * DashDistance * 4.0f; // 4x for velocity
	
	Character->LaunchCharacter(DashVelocity, true, true);
	
	bIsDashing = true;
	DashTimer = ActionData ? ActionData->DashDuration : 0.3f;
	
	// Add invincibility frames if needed
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		CombatComp->AddCombatStateTag(FGameplayTag::RequestGameplayTag("Combat.State.Dashing"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("Executed Dash"));
}

void UUniversalAction::ExecuteBlock()
{
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		if (CombatComp->StartBlock())
		{
			bIsBlocking = true;
			UE_LOG(LogTemp, Log, TEXT("Started Blocking"));
		}
	}
}

void UUniversalAction::ExecuteMeleeAttack()
{
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		// Determine attack type from tag
		FGameplayTag AttackTag = ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Action.Ability.HeavyAttack")) 
			? FGameplayTag::RequestGameplayTag("Attack.Type.Heavy")
			: FGameplayTag::RequestGameplayTag("Attack.Type.Jab");
		
		if (CombatComp->StartAttack(AttackTag))
		{
			bIsAttacking = true;
			AttackTimer = 0.5f; // Default attack duration, could use montage length
			
			// Play animation if available
			if (ActionData && ActionData->ActionMontage && CurrentOwner)
			{
				if (ACharacter* Character = Cast<ACharacter>(CurrentOwner))
				{
					Character->PlayAnimMontage(ActionData->ActionMontage);
				}
			}
			
			UE_LOG(LogTemp, Log, TEXT("Executed Melee Attack: %s"), *AttackTag.ToString());
		}
	}
}

void UUniversalAction::ExecuteGenericAbility()
{
	// Generic ability execution
	// Could spawn effects, apply damage in radius, etc based on ActionData
	
	UE_LOG(LogTemp, Log, TEXT("Executed Generic Ability: %s"), *ActionTag.ToString());
	
	// Most abilities are instant, so release after execution
	OnRelease();
}