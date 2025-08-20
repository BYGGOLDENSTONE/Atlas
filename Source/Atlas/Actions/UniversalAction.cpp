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
	bIsChanneling = false;
	ActionTimer = 0.0f;
	ChannelProgress = 0.0f;
	
	// Initialize the action executor map
	InitializeExecutorMap();
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

	// Execute action based on type
	ExecuteActionByType();
	
	// Apply station integrity cost if configured
	if (ActionData && ActionData->IntegrityCost > 0.0f)
	{
		if (UStationIntegrityComponent* StationIntegrity = GetStationIntegrity())
		{
			StationIntegrity->ApplyDamage(ActionData->IntegrityCost);
		}
	}
}

void UUniversalAction::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);

	// Handle universal action timer
	if (ActionTimer > 0.0f)
	{
		ActionTimer -= DeltaTime;
		
		// Check for action completion based on type
		if (ActionTimer <= 0.0f)
		{
			if (bIsDashing)
			{
				bIsDashing = false;
				if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
				{
					CombatComp->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag("Combat.State.Dashing"));
				}
			}
			if (bIsAttacking)
			{
				bIsAttacking = false;
			}
			if (bIsChanneling)
			{
				bIsChanneling = false;
			}
			
			// Check if we should auto-release
			if (!ActionData || ActionData->bAutoReleaseOnComplete)
			{
				OnRelease();
			}
		}
	}
	
	// Update channel progress for channeled abilities
	if (bIsChanneling && ActionData && ActionData->ChargeTime > 0.0f)
	{
		ChannelProgress = FMath::Clamp((ActionData->ChargeTime - ActionTimer) / ActionData->ChargeTime, 0.0f, 1.0f);
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
	
	// Handle toggle actions
	if (ActionData && ActionData->bIsToggleAction)
	{
		// Toggle actions don't auto-release, they wait for another press
		return;
	}

	// Call base implementation to handle common release logic
	Super::OnRelease();
}

void UUniversalAction::OnInterrupted()
{
	// Check if this action can be interrupted
	if (ActionData && !ActionData->bCanBeInterrupted)
	{
		// Action cannot be interrupted, ignore the request
		return;
	}
	
	bIsBlocking = false;
	bIsDashing = false;
	bIsAttacking = false;
	bIsChanneling = false;
	ActionTimer = 0.0f;
	ChannelProgress = 0.0f;
	
	// Call base implementation to handle interruption
	Super::OnInterrupted();
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
	ActionTimer = ActionData ? ActionData->DashDuration : 0.3f;
	
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
			// Use data-driven attack duration
			if (ActionData)
			{
				ActionTimer = ActionData->ActionDuration;
			}
			else
			{
				ActionTimer = 0.5f; // Fallback default
			}
			
			// Play animation if available
			if (ActionData && ActionData->ActionMontage && CurrentOwner)
			{
				if (ACharacter* Character = Cast<ACharacter>(CurrentOwner))
				{
					// Play montage with configured play rate and section
					float PlayRate = ActionData->MontagePlayRate;
					FName SectionName = ActionData->MontageSectionName;
					
					if (SectionName != NAME_None)
					{
						Character->PlayAnimMontage(ActionData->ActionMontage, PlayRate, SectionName);
					}
					else
					{
						Character->PlayAnimMontage(ActionData->ActionMontage, PlayRate);
					}
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
	
	// Check if this is a channeled ability
	if (ActionData && ActionData->ChargeTime > 0.0f)
	{
		bIsChanneling = true;
		ActionTimer = ActionData->ChargeTime;
		ChannelProgress = 0.0f;
	}
	else
	{
		// Instant ability, release after execution
		OnRelease();
	}
}

void UUniversalAction::ExecuteFocusMode()
{
	if (!CurrentOwner)
		return;
		
	// Toggle focus mode through a component or state
	if (UCombatComponent* CombatComp = GetOwnerCombatComponent())
	{
		FGameplayTag FocusTag = FGameplayTag::RequestGameplayTag("Combat.State.FocusMode");
		if (CombatComp->HasCombatStateTag(FocusTag))
		{
			CombatComp->RemoveCombatStateTag(FocusTag);
			UE_LOG(LogTemp, Log, TEXT("Exited Focus Mode"));
		}
		else
		{
			CombatComp->AddCombatStateTag(FocusTag);
			UE_LOG(LogTemp, Log, TEXT("Entered Focus Mode"));
		}
	}
	
	OnRelease();
}

void UUniversalAction::ExecuteAreaEffect()
{
	if (!CurrentOwner || !ActionData)
		return;
		
	// Spawn area effect at owner location
	FVector EffectLocation = CurrentOwner->GetActorLocation();
	float Radius = ActionData->EffectRadius;
	float Damage = ActionData->EffectDamage;
	
	// Draw debug sphere for visualization
	if (CurrentOwner->GetWorld())
	{
		DrawDebugSphere(CurrentOwner->GetWorld(), EffectLocation, Radius, 32, FColor::Red, false, 2.0f);
	}
	
	// Apply damage to all actors in radius
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CurrentOwner);
	
	if (CurrentOwner->GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		EffectLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		QueryParams))
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			if (AActor* HitActor = Result.GetActor())
			{
				// Apply damage or effects based on ActionData
				UE_LOG(LogTemp, Log, TEXT("Area effect hit: %s"), *HitActor->GetName());
			}
		}
	}
	
	// Check for duration-based effects
	if (ActionData->EffectDuration > 0.0f)
	{
		ActionTimer = ActionData->EffectDuration;
		bIsChanneling = true;
	}
	else
	{
		OnRelease();
	}
}

void UUniversalAction::ExecuteRangedAttack()
{
	if (!CurrentOwner || !ActionData)
		return;
		
	// Spawn projectile or perform hitscan
	FVector StartLocation = CurrentOwner->GetActorLocation() + FVector(0, 0, 50);
	FVector ForwardVector = CurrentOwner->GetActorForwardVector();
	
	// Simple line trace for now
	FHitResult HitResult;
	FVector EndLocation = StartLocation + (ForwardVector * ActionData->ProjectileRange);
	
	if (CurrentOwner->GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Pawn))
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			UE_LOG(LogTemp, Log, TEXT("Ranged attack hit: %s"), *HitActor->GetName());
			// Apply damage based on ActionData->ProjectileDamage
		}
	}
	
	// Draw debug line
	if (CurrentOwner->GetWorld())
	{
		DrawDebugLine(CurrentOwner->GetWorld(), StartLocation, EndLocation, FColor::Yellow, false, 1.0f);
	}
	
	OnRelease();
}

void UUniversalAction::ExecuteUtility()
{
	if (!CurrentOwner || !ActionData)
		return;
		
	// Handle utility actions based on specific tags
	if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Action.Ability.SystemHack")))
	{
		// System hack specific logic
		UE_LOG(LogTemp, Log, TEXT("Executing System Hack"));
	}
	else if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Action.Ability.GravityAnchor")))
	{
		// Gravity anchor specific logic
		UE_LOG(LogTemp, Log, TEXT("Executing Gravity Anchor"));
	}
	
	// Check for charge time
	if (ActionData->ChargeTime > 0.0f)
	{
		bIsChanneling = true;
		ActionTimer = ActionData->ChargeTime;
	}
	else
	{
		OnRelease();
	}
}

void UUniversalAction::InitializeExecutorMap()
{
	// Map action tags to their execution functions
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.Dash"), &UUniversalAction::ExecuteDash);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.Block"), &UUniversalAction::ExecuteBlock);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.BasicAttack"), &UUniversalAction::ExecuteMeleeAttack);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.HeavyAttack"), &UUniversalAction::ExecuteMeleeAttack);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.FocusMode"), &UUniversalAction::ExecuteFocusMode);
	
	// Map area effect abilities
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.KineticPulse"), &UUniversalAction::ExecuteAreaEffect);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.SeismicStamp"), &UUniversalAction::ExecuteAreaEffect);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.LocalizedEMP"), &UUniversalAction::ExecuteAreaEffect);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.FloorDestabilizer"), &UUniversalAction::ExecuteAreaEffect);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.AirlockBreach"), &UUniversalAction::ExecuteAreaEffect);
	
	// Map ranged abilities
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.DebrisPull"), &UUniversalAction::ExecuteRangedAttack);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.ImpactGauntlet"), &UUniversalAction::ExecuteRangedAttack);
	
	// Map utility abilities
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.CoolantSpray"), &UUniversalAction::ExecuteUtility);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.SystemHack"), &UUniversalAction::ExecuteUtility);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Ability.GravityAnchor"), &UUniversalAction::ExecuteUtility);
}

void UUniversalAction::ExecuteActionByType()
{
	// First check if we have a specific executor for this tag
	if (ActionExecutor* Executor = ActionExecutorMap.Find(ActionTag))
	{
		(this->**Executor)();
		return;
	}
	
	// Fall back to action type-based execution
	if (!ActionData)
	{
		ExecuteGenericAbility();
		return;
	}
	
	switch (ActionData->ActionType)
	{
		case EActionType::Movement:
			ExecuteDash();
			break;
		case EActionType::Defense:
			ExecuteBlock();
			break;
		case EActionType::MeleeAttack:
			ExecuteMeleeAttack();
			break;
		case EActionType::RangedAttack:
			ExecuteRangedAttack();
			break;
		case EActionType::AreaEffect:
			ExecuteAreaEffect();
			break;
		case EActionType::Utility:
			ExecuteUtility();
			break;
		case EActionType::Special:
		default:
			ExecuteGenericAbility();
			break;
	}
}

void UUniversalAction::BeginDestroy()
{
	// Clear the executor map
	ActionExecutorMap.Empty();
	
	Super::BeginDestroy();
}