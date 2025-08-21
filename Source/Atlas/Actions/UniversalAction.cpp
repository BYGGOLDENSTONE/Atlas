#include "UniversalAction.h"
#include "../Characters/GameCharacterBase.h"
#include "../Components/ActionManagerComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/ActionManagerComponent.h"
#include "../Components/StationIntegrityComponent.h"
#include "../Core/AtlasGameplayTags.h"
#include "../Data/ActionDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"

UUniversalAction::UUniversalAction()
{
	bIsBlocking = false;
	bIsDashing = false;
	bIsAttacking = false;
	bIsChanneling = false;
	ActionTimer = 0.0f;
	ChannelProgress = 0.0f;
	
	// Don't initialize executor map here - gameplay tags aren't ready yet
	// It will be initialized on first use
}

bool UUniversalAction::CanActivate(AGameCharacterBase* Owner)
{
	if (!Super::CanActivate(Owner))
	{
		return false;
	}

	// Prevent activating any action while attacking (except during combo windows)
	if (Owner && ActionData)
	{
		// Check if this is an attack action
		if (ActionData->ActionType == EActionType::MeleeAttack || 
		    ActionData->ActionType == EActionType::RangedAttack)
		{
			// Don't allow attack if already attacking (unless in combo window)
			if (UActionManagerComponent* ActionManager = Owner->GetActionManagerComponent())
			{
				if (ActionManager->IsAttacking())
				{
					// Check if we're in a combo window
					if (UActionManagerComponent* ActionManager = Owner->FindComponentByClass<UActionManagerComponent>())
					{
						if (!ActionManager->IsComboWindowActive())
						{
							UE_LOG(LogTemp, Warning, TEXT("Cannot attack while already attacking (not in combo window)"));
							return false;
						}
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Cannot attack while already attacking"));
						return false;
					}
				}
			}
		}
		
		// Also check if we're currently in any blocking state
		if (UActionManagerComponent* ActionManager = Owner->GetCombatComponent())
		{
			// Can't start most actions while staggered
			if (ActionManager->HasCombatStateTag(FGameplayTag::RequestGameplayTag("State.Staggered")))
			{
				return false;
			}
		}
	}

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
			StationIntegrity->ApplyIntegrityDamage(ActionData->IntegrityCost);
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
				if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent())
				{
					ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag("Combat.State.Dashing"));
				}
			}
			if (bIsAttacking)
			{
				bIsAttacking = false;
				if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent())
				{
					ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag("Combat.State.Attacking"));
				}
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
		if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent())
		{
			ActionManager->EndBlock();
		}
		bIsBlocking = false;
	}
	
	// Clean up attack state if still active
	if (bIsAttacking)
	{
		bIsAttacking = false;
		if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent())
		{
			ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag("Combat.State.Attacking"));
		}
	}
	
	// Clean up dash state if still active
	if (bIsDashing)
	{
		bIsDashing = false;
		if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent())
		{
			ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag("Combat.State.Dashing"));
		}
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
		UE_LOG(LogTemp, Warning, TEXT("Action cannot be interrupted: %s"), *ActionTag.ToString());
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Action interrupted: %s"), *ActionTag.ToString());
	
	// Clean up combat states properly
	if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent())
	{
		if (bIsAttacking)
		{
			ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag("Combat.State.Attacking"));
			ActionManager->SetCurrentActionData(nullptr);
		}
		if (bIsBlocking)
		{
			ActionManager->EndBlock();
		}
		if (bIsDashing)
		{
			ActionManager->RemoveCombatStateTag(FGameplayTag::RequestGameplayTag("Combat.State.Dashing"));
		}
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
	if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent())
	{
		ActionManager->AddCombatStateTag(FGameplayTag::RequestGameplayTag("Combat.State.Dashing"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("Executed Dash"));
}

void UUniversalAction::ExecuteBlock()
{
	if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent())
	{
		if (ActionManager->StartBlock())
		{
			bIsBlocking = true;
			UE_LOG(LogTemp, Log, TEXT("Started Blocking"));
		}
	}
}

void UUniversalAction::ExecuteMeleeAttack()
{
	if (!ActionData || !CurrentOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecuteMeleeAttack: Missing ActionData or Owner"));
		return;
	}
	
	// Melee attacks should generally not be interruptible
	// This should be set in the DataAsset, but let's log it
	if (ActionData->bCanBeInterrupted)
	{
		UE_LOG(LogTemp, Warning, TEXT("WARNING: Melee attack %s is marked as interruptible! This allows spam!"), *ActionTag.ToString());
	}

	// Check if we can attack
	if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent())
	{
		// First check if already attacking - this should have been caught earlier but double check
		bool bIsCurrentlyAttacking = ActionManager->IsAttacking();
		UE_LOG(LogTemp, Warning, TEXT("ExecuteMeleeAttack - IsAttacking: %s"), 
			bIsCurrentlyAttacking ? TEXT("TRUE") : TEXT("FALSE"));
			
		if (bIsCurrentlyAttacking)
		{
			UE_LOG(LogTemp, Error, TEXT("ExecuteMeleeAttack BLOCKED: Already attacking"));
			return;
		}
		
		if (UHealthComponent* HealthComp = GetOwnerHealthComponent())
		{
			if (HealthComp->IsStaggered())
			{
				UE_LOG(LogTemp, Warning, TEXT("ExecuteMeleeAttack blocked: Staggered"));
				return;
			}
		}

		// Set attacking state IMMEDIATELY to prevent spam
		// The animation's CombatStateNotify at frame 0 will re-set this (harmless)
		// The animation's CombatStateNotify at end will clear it
		ActionManager->AddCombatStateTag(FGameplayTag::RequestGameplayTag("Combat.State.Attacking"));
		bIsAttacking = true;
		
		// ActionTimer is only used as a safety timeout, animation controls actual duration
		ActionTimer = ActionData->ActionDuration > 0.0f ? ActionData->ActionDuration : 2.0f; // Generous timeout
		
		// Store attack data in CombatComponent so ProcessHitFromAnimation can use it
		// This allows the animation notifies to access damage values
		ActionManager->SetCurrentActionData(ActionData);
		
		// Play animation montage - the montage should have AttackNotifyState 
		// which will handle hit detection timing automatically
		if (ActionData->ActionMontage)
		{
			if (ACharacter* Character = Cast<ACharacter>(CurrentOwner))
			{
				float PlayRate = ActionData->MontagePlayRate > 0.0f ? ActionData->MontagePlayRate : 1.0f;
				FName SectionName = ActionData->MontageSectionName;
				
				if (SectionName != NAME_None)
				{
					Character->PlayAnimMontage(ActionData->ActionMontage, PlayRate, SectionName);
				}
				else
				{
					Character->PlayAnimMontage(ActionData->ActionMontage, PlayRate);
				}
				
				// Playing attack montage with AnimNotifies for hit detection
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No ActionMontage configured for %s - attack won't have hit detection!"), *ActionTag.ToString());
		}
		
		// Melee Attack Started
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
	if (UActionManagerComponent* ActionManager = GetOwnerActionManagerComponent())
	{
		FGameplayTag FocusTag = FGameplayTag::RequestGameplayTag("Combat.State.FocusMode");
		if (ActionManager->HasCombatStateTag(FocusTag))
		{
			ActionManager->RemoveCombatStateTag(FocusTag);
			UE_LOG(LogTemp, Log, TEXT("Exited Focus Mode"));
		}
		else
		{
			ActionManager->AddCombatStateTag(FocusTag);
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
	if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Action.SystemHack")))
	{
		// System hack specific logic
		UE_LOG(LogTemp, Log, TEXT("Executing System Hack"));
	}
	else if (ActionTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Action.GravityAnchor")))
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
	// Map action tags to their execution functions - using correct tag names
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Dash"), &UUniversalAction::ExecuteDash);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.Block"), &UUniversalAction::ExecuteBlock);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.BasicAttack"), &UUniversalAction::ExecuteMeleeAttack);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.HeavyAttack"), &UUniversalAction::ExecuteMeleeAttack);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.FocusMode"), &UUniversalAction::ExecuteFocusMode);
	
	// Map area effect abilities
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.KineticPulse"), &UUniversalAction::ExecuteAreaEffect);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.SeismicStamp"), &UUniversalAction::ExecuteAreaEffect);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.LocalizedEMP"), &UUniversalAction::ExecuteAreaEffect);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.FloorDestabilizer"), &UUniversalAction::ExecuteAreaEffect);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.AirlockBreach"), &UUniversalAction::ExecuteAreaEffect);
	
	// Map ranged abilities
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.DebrisPull"), &UUniversalAction::ExecuteRangedAttack);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.ImpactGauntlet"), &UUniversalAction::ExecuteRangedAttack);
	
	// Map utility abilities
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.CoolantSpray"), &UUniversalAction::ExecuteUtility);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.SystemHack"), &UUniversalAction::ExecuteUtility);
	ActionExecutorMap.Add(FGameplayTag::RequestGameplayTag("Action.GravityAnchor"), &UUniversalAction::ExecuteUtility);
}

void UUniversalAction::ExecuteActionByType()
{
	// Initialize map on first use (lazy initialization to avoid constructor issues)
	if (ActionExecutorMap.Num() == 0)
	{
		InitializeExecutorMap();
	}
	
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