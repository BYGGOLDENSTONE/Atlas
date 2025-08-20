#include "BlockAction.h"
#include "../Characters/GameCharacterBase.h"
#include "../Components/CombatComponent.h"
#include "../Components/HealthComponent.h"
#include "../DataAssets/ActionDataAsset.h"
#include "../Core/AtlasGameplayTags.h"

UBlockAction::UBlockAction()
{
	ActionTag = FGameplayTag::RequestGameplayTag(FName("Action.Block"));
	bIsBlocking = false;
	BlockHoldTime = 0.0f;
}

bool UBlockAction::CanActivate(AGameCharacterBase* Owner)
{
	if (!Super::CanActivate(Owner))
	{
		return false;
	}

	// Check if already blocking
	if (bIsBlocking)
	{
		return false;
	}

	// Check state restrictions
	if (!CheckBlockRestrictions())
	{
		return false;
	}

	return true;
}

void UBlockAction::OnActivate(AGameCharacterBase* Owner)
{
	Super::OnActivate(Owner);
	StartBlock();
}

void UBlockAction::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);
	
	if (bIsBlocking)
	{
		BlockHoldTime += DeltaTime;
		
		// Could add stamina drain or other mechanics here
		// For now, block can be held indefinitely
	}
}

void UBlockAction::OnRelease()
{
	if (bIsBlocking)
	{
		EndBlock();
	}
	Super::OnRelease();
}

void UBlockAction::OnInterrupted()
{
	if (bIsBlocking)
	{
		EndBlock();
	}
	Super::OnInterrupted();
}

void UBlockAction::StartBlock()
{
	if (!CurrentOwner)
	{
		return;
	}

	// Use CombatComponent to start blocking
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		if (CombatComp->StartBlock())
		{
			bIsBlocking = true;
			BlockHoldTime = 0.0f;
			
			UE_LOG(LogTemp, Log, TEXT("BlockAction: Started blocking"));
		}
		else
		{
			// Failed to start block
			bIsActive = false;
			UE_LOG(LogTemp, Warning, TEXT("BlockAction: Failed to start block"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BlockAction: No CombatComponent found"));
		bIsActive = false;
	}
}

void UBlockAction::EndBlock()
{
	if (!bIsBlocking)
	{
		return;
	}

	bIsBlocking = false;
	bIsActive = false;

	// End block in combat component
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		CombatComp->EndBlock();
		UE_LOG(LogTemp, Log, TEXT("BlockAction: Ended blocking after %.2f seconds"), BlockHoldTime);
	}

	BlockHoldTime = 0.0f;

	// Blocks typically don't have cooldowns, but we can add one if needed
	if (ActionData && ActionData->Cooldown > 0.0f)
	{
		StartCooldown();
	}
}

bool UBlockAction::CheckBlockRestrictions() const
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

	// Check if currently attacking
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		if (CombatComp->IsAttacking())
		{
			// Can't block while attacking
			return false;
		}
	}

	return true;
}