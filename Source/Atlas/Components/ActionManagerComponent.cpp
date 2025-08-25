#include "ActionManagerComponent.h"
#include "../Actions/ActionInstance.h"
#include "../Data/ActionDataAsset.h"
#include "../Characters/GameCharacterBase.h"
#include "../Characters/PlayerCharacter.h"
#include "../Components/HealthComponent.h"
#include "../Components/VulnerabilityComponent.h"
#include "../Components/StationIntegrityComponent.h"
#include "../Data/CombatRulesDataAsset.h"
#include "../Data/StationIntegrityDataAsset.h"
#include "../Core/AtlasGameState.h"
#include "GameFramework/Character.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "TimerManager.h"

UActionManagerComponent::UActionManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentAction = nullptr;
	OwnerCharacter = nullptr;
	CurrentActionData = nullptr;

	// Initialize default slot names
	InitializeSlots();
}

void UActionManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AGameCharacterBase>(GetOwner());
	
	// Cache component references
	HealthComponent = GetOwner()->FindComponentByClass<UHealthComponent>();
	ensure(HealthComponent); // Warn if missing but don't crash
	
	VulnerabilityComponent = GetOwner()->FindComponentByClass<UVulnerabilityComponent>();
	if (!VulnerabilityComponent)
	{
		VulnerabilityComponent = NewObject<UVulnerabilityComponent>(GetOwner(), UVulnerabilityComponent::StaticClass(), TEXT("VulnerabilityComponent"));
		VulnerabilityComponent->RegisterComponent();
	}
	
	// Load available actions
	LoadAvailableActions();

	// Apply default slot assignments
	for (const auto& DefaultSlot : DefaultSlotAssignments)
	{
		AssignActionToSlot(DefaultSlot.Key, DefaultSlot.Value);
	}
}

void UActionManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Tick all actions for cooldown updates
	TickActions(DeltaTime);
}

void UActionManagerComponent::InitializeSlots()
{
	// Initialize the 5 universal slots
	ActionSlots.Add(TEXT("Slot1"), nullptr);
	ActionSlots.Add(TEXT("Slot2"), nullptr);
	ActionSlots.Add(TEXT("Slot3"), nullptr);
	ActionSlots.Add(TEXT("Slot4"), nullptr);
	ActionSlots.Add(TEXT("Slot5"), nullptr);
	
	// Set up default combat ability assignments (only for actions that exist)
	DefaultSlotAssignments.Add(TEXT("Slot1"), FGameplayTag::RequestGameplayTag(FName("Action.Combat.BasicAttack")));
	DefaultSlotAssignments.Add(TEXT("Slot2"), FGameplayTag::RequestGameplayTag(FName("Action.Combat.HeavyAttack")));
	// Slots 3-5 will remain empty until more actions are created
	// DefaultSlotAssignments.Add(TEXT("Slot3"), FGameplayTag::RequestGameplayTag(FName("Action.Combat.Dash")));
	// DefaultSlotAssignments.Add(TEXT("Slot4"), FGameplayTag::RequestGameplayTag(FName("Action.Combat.Block")));
	// DefaultSlotAssignments.Add(TEXT("Slot5"), FGameplayTag::RequestGameplayTag(FName("Action.Combat.FocusMode")));
}

bool UActionManagerComponent::AssignActionToSlot(FName SlotName, FGameplayTag ActionTag)
{
	// Find the action data asset with matching tag
	UActionDataAsset* ActionData = GetActionDataByTag(ActionTag);
	if (!ActionData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionManager: No action found with tag %s"), *ActionTag.ToString());
		return false;
	}

	return AssignActionToSlotByDataAsset(SlotName, ActionData);
}

bool UActionManagerComponent::AssignActionToSlotByDataAsset(FName SlotName, UActionDataAsset* ActionData)
{
	if (!ActionSlots.Contains(SlotName))
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionManager: Invalid slot name %s"), *SlotName.ToString());
		return false;
	}

	if (!ActionData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionManager: Invalid action data"));
		return false;
	}

	// Clear existing action in slot
	if (ActionSlots[SlotName])
	{
		if (CurrentAction == ActionSlots[SlotName])
		{
			InterruptCurrentAction();
		}
		ActionSlots[SlotName] = nullptr;
	}

	// Create new action instance
	UActionInstance* NewAction = CreateActionInstance(ActionData);
	if (!NewAction)
	{
		UE_LOG(LogTemp, Error, TEXT("ActionManager: Failed to create action instance"));
		return false;
	}

	// Assign to slot
	ActionSlots[SlotName] = NewAction;
	OnActionSlotChanged.Broadcast(SlotName, NewAction);

	UE_LOG(LogTemp, Log, TEXT("ActionManager: Assigned %s to %s"), *ActionData->ActionName.ToString(), *SlotName.ToString());
	return true;
}

void UActionManagerComponent::ClearSlot(FName SlotName)
{
	if (!ActionSlots.Contains(SlotName))
	{
		return;
	}

	if (ActionSlots[SlotName])
	{
		if (CurrentAction == ActionSlots[SlotName])
		{
			InterruptCurrentAction();
		}
		ActionSlots[SlotName] = nullptr;
		OnActionSlotChanged.Broadcast(SlotName, nullptr);
	}
}

void UActionManagerComponent::SwapSlots(FName Slot1, FName Slot2)
{
	if (!ActionSlots.Contains(Slot1) || !ActionSlots.Contains(Slot2))
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionManager: Invalid slot names for swap"));
		return;
	}

	UActionInstance* TempAction = ActionSlots[Slot1];
	ActionSlots[Slot1] = ActionSlots[Slot2];
	ActionSlots[Slot2] = TempAction;

	OnActionSlotChanged.Broadcast(Slot1, ActionSlots[Slot1]);
	OnActionSlotChanged.Broadcast(Slot2, ActionSlots[Slot2]);
}

UActionInstance* UActionManagerComponent::GetActionInSlot(FName SlotName) const
{
	if (ActionSlots.Contains(SlotName))
	{
		return ActionSlots[SlotName];
	}
	return nullptr;
}

TArray<FName> UActionManagerComponent::GetAllSlotNames() const
{
	TArray<FName> SlotNames;
	ActionSlots.GetKeys(SlotNames);
	return SlotNames;
}

void UActionManagerComponent::OnSlotPressed(FName SlotName)
{
	if (!OwnerCharacter)
	{
		return;
	}

	// Check if ability inputs are enabled on PlayerCharacter
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(OwnerCharacter))
	{
		if (!PlayerChar->AreAbilityInputsEnabled())
		{
			UE_LOG(LogTemp, Error, TEXT("INPUT BLOCKED: Ability inputs disabled, cannot use %s"), *SlotName.ToString());
			return;
		}
	}

	// Check if we're in an attack state and not in a combo window
	bool bIsAttacking = IsAttacking();
	UE_LOG(LogTemp, Warning, TEXT("OnSlotPressed %s - IsAttacking: %s, ComboWindow: %s"), 
		*SlotName.ToString(), 
		bIsAttacking ? TEXT("TRUE") : TEXT("FALSE"),
		bComboWindowActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
		
	if (bIsAttacking && !bComboWindowActive)
	{
		// Block all action inputs while attacking (except during combo windows)
		UE_LOG(LogTemp, Error, TEXT("INPUT BLOCKED: Cannot use %s while attacking (not in combo window)"), *SlotName.ToString());
		return;
	}

	// If combo window is active, buffer the input
	if (bComboWindowActive)
	{
		BufferedSlot = SlotName;
		BufferedInputTime = GetWorld()->GetTimeSeconds();
		UE_LOG(LogTemp, Log, TEXT("Buffered input: %s during combo window %s"), *SlotName.ToString(), *CurrentComboWindow.ToString());
		return;
	}

	UActionInstance* Action = GetActionInSlot(SlotName);
	if (!Action)
	{
		return;
	}

	// Check if this is the same action already active
	if (CurrentAction && CurrentAction == Action && CurrentAction->IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("Same action already active, ignoring press for %s"), *SlotName.ToString());
		return;
	}
	
	// Check if we need to interrupt a different action
	if (CurrentAction && CurrentAction != Action && CurrentAction->IsActive())
	{
		// For now, interrupt the current action
		// Later we might add priority or queuing system
		InterruptCurrentAction();
	}

	// Try to activate the action
	if (Action->CanExecute(OwnerCharacter))
	{
		CurrentAction = Action;
		// Set the current action data for damage calculations
		SetCurrentActionData(Action->GetActionData());
		Action->Execute(OwnerCharacter);
		OnActionActivated.Broadcast(SlotName, Action);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Action cannot activate: %s"), *SlotName.ToString());
	}
}

void UActionManagerComponent::OnSlotReleased(FName SlotName)
{
	UActionInstance* Action = GetActionInSlot(SlotName);
	if (!Action)
	{
		return;
	}

	// Only process release if this is the current active action
	if (CurrentAction == Action && Action->IsActive())
	{
		Action->Stop();
		CurrentAction = nullptr;
	}
}

void UActionManagerComponent::InterruptCurrentAction()
{
	if (CurrentAction && CurrentAction->IsActive())
	{
		CurrentAction->Interrupt();
		CurrentAction = nullptr;
	}
}

void UActionManagerComponent::LoadAvailableActions()
{
	// If actions were already set in editor, keep them but still try to load defaults
	if (AvailableActionDataAssets.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("ActionManager: Using %d pre-configured action data assets"), AvailableActionDataAssets.Num());
		// Don't return early - continue to load missing default actions
	}
	
	// Load action data assets from the actual Content/Dataassets/Actions/ folder
	const TArray<FString> ActionAssetPaths = {
		// Currently available actions
		TEXT("/Game/Dataassets/Actions/DA_BasicAttack"),
		TEXT("/Game/Dataassets/Actions/DA_HeavyAttack")
	};
	
	for (const FString& AssetPath : ActionAssetPaths)
	{
		UActionDataAsset* ActionData = LoadObject<UActionDataAsset>(nullptr, *AssetPath);
		if (ActionData)
		{
			AvailableActionDataAssets.Add(ActionData);
			UE_LOG(LogTemp, Log, TEXT("ActionManager: Loaded action data asset: %s"), *ActionData->ActionName.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ActionManager: Failed to load action data asset at path: %s"), *AssetPath);
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("ActionManager: Loaded %d action data assets"), AvailableActionDataAssets.Num());
}

UActionDataAsset* UActionManagerComponent::GetActionDataByTag(FGameplayTag ActionTag) const
{
	for (UActionDataAsset* ActionData : AvailableActionDataAssets)
	{
		if (ActionData && ActionData->ActionTag == ActionTag)
		{
			return ActionData;
		}
	}
	return nullptr;
}

UActionInstance* UActionManagerComponent::CreateActionInstance(UActionDataAsset* ActionData)
{
	if (!ActionData)
	{
		return nullptr;
	}

	UActionInstance* NewAction = NewObject<UActionInstance>(this, UActionInstance::StaticClass());
	if (NewAction)
	{
		NewAction->Initialize(ActionData);
	}
	return NewAction;
}

void UActionManagerComponent::TickActions(float DeltaTime)
{
	// Tick all actions for cooldown and other updates
	for (const auto& Slot : ActionSlots)
	{
		if (Slot.Value)
		{
			Slot.Value->Update(DeltaTime);
		}
	}
}

// Console Commands
void UActionManagerComponent::ExecuteAssignCommand(const FString& SlotName, const FString& ActionTagString)
{
	FGameplayTag ActionTag = FGameplayTag::RequestGameplayTag(FName(*ActionTagString));
	if (!ActionTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionManager: Invalid action tag %s"), *ActionTagString);
		return;
	}

	AssignActionToSlot(FName(*SlotName), ActionTag);
}

void UActionManagerComponent::ExecuteClearSlotCommand(const FString& SlotName)
{
	ClearSlot(FName(*SlotName));
	UE_LOG(LogTemp, Log, TEXT("ActionManager: Cleared %s"), *SlotName);
}

void UActionManagerComponent::ExecuteSwapSlotsCommand(const FString& Slot1, const FString& Slot2)
{
	SwapSlots(FName(*Slot1), FName(*Slot2));
	UE_LOG(LogTemp, Log, TEXT("ActionManager: Swapped %s and %s"), *Slot1, *Slot2);
}

void UActionManagerComponent::ExecuteListActionsCommand()
{
	UE_LOG(LogTemp, Log, TEXT("=== Available Actions ==="));
	for (const UActionDataAsset* ActionData : AvailableActionDataAssets)
	{
		if (ActionData)
		{
			UE_LOG(LogTemp, Log, TEXT("  %s - %s"), 
				*ActionData->ActionTag.ToString(), 
				*ActionData->ActionName.ToString());
		}
	}
}

void UActionManagerComponent::SetComboWindowActive(bool bActive, FName WindowName)
{
	bComboWindowActive = bActive;
	CurrentComboWindow = WindowName;
	
	if (!bActive)
	{
		// Clear buffered input if window is closing without executing
		if (BufferedSlot == NAME_None)
		{
			BufferedSlot = NAME_None;
			BufferedInputTime = 0.0f;
		}
	}
}

void UActionManagerComponent::ExecuteBufferedAction()
{
	if (BufferedSlot != NAME_None)
	{
		FName SlotToExecute = BufferedSlot;
		BufferedSlot = NAME_None;
		BufferedInputTime = 0.0f;
		
		// Execute the buffered action
		UE_LOG(LogTemp, Log, TEXT("Executing buffered action: %s"), *SlotToExecute.ToString());
		OnSlotPressed(SlotToExecute);
	}
}

void UActionManagerComponent::ExecuteShowSlotsCommand()
{
	UE_LOG(LogTemp, Log, TEXT("=== Current Slot Assignments ==="));
	for (const auto& Slot : ActionSlots)
	{
		if (Slot.Value)
		{
			UE_LOG(LogTemp, Log, TEXT("  %s: %s"), 
				*Slot.Key.ToString(), 
				*Slot.Value->GetActionTag().ToString());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("  %s: [Empty]"), *Slot.Key.ToString());
		}
	}
}

void UActionManagerComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	// Clean up actions
	for (auto& Slot : ActionSlots)
	{
		if (Slot.Value)
		{
			Slot.Value->ConditionalBeginDestroy();
		}
	}
	ActionSlots.Empty();
	
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

// ========================================
// COMBAT STATE MANAGEMENT
// ========================================

bool UActionManagerComponent::IsInCombat() const
{
	// Check if we're actively attacking, blocking, or recently damaged
	if (IsAttacking() || IsBlocking() || (HealthComponent && HealthComponent->IsStaggered()))
	{
		return true;
	}
	
	// Check if we've been in combat within the last 3 seconds
	float TimeSinceLastAction = GetTimeSinceLastCombatAction();
	return TimeSinceLastAction < 3.0f;
}

bool UActionManagerComponent::IsAttacking() const
{
	return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.Attacking")));
}

bool UActionManagerComponent::IsBlocking() const
{
	return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.Blocking")));
}

bool UActionManagerComponent::IsVulnerable() const
{
	if (VulnerabilityComponent)
	{
		return VulnerabilityComponent->IsVulnerable();
	}
	return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("State.Status.Vulnerable")));
}

bool UActionManagerComponent::HasIFrames() const
{
	if (VulnerabilityComponent)
	{
		return VulnerabilityComponent->HasIFrames();
	}
	return HasCombatStateTag(FGameplayTag::RequestGameplayTag(FName("State.Status.Invulnerable")));
}

float UActionManagerComponent::GetTimeSinceLastCombatAction() const
{
	if (LastCombatActionTime <= 0.0f)
	{
		return 999.0f; // Never been in combat
	}
	
	return GetWorld() ? GetWorld()->GetTimeSeconds() - LastCombatActionTime : 999.0f;
}

void UActionManagerComponent::AddCombatStateTag(const FGameplayTag& Tag)
{
	CombatStateTags.AddTag(Tag);
	LastCombatActionTime = GetWorld()->GetTimeSeconds();
}

void UActionManagerComponent::RemoveCombatStateTag(const FGameplayTag& Tag)
{
	CombatStateTags.RemoveTag(Tag);
}

bool UActionManagerComponent::HasCombatStateTag(const FGameplayTag& Tag) const
{
	return CombatStateTags.HasTag(Tag);
}

bool UActionManagerComponent::StartBlock()
{
	if ((HealthComponent && HealthComponent->IsStaggered()) || IsAttacking())
	{
		return false;
	}

	AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.Blocking")));
	OnBlockStarted.Broadcast(true);
	return true;
}

void UActionManagerComponent::EndBlock()
{
	RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.Blocking")));
	OnBlockEnded.Broadcast();
}

void UActionManagerComponent::SetParryState(bool bParrying)
{
	bIsParrying = bParrying;
	
	if (bParrying)
	{
		// Record parry start time for window tracking
		ParryStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		AddCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Action.Combat.Parry")));
		UE_LOG(LogTemp, Log, TEXT("Parry state activated"));
	}
	else
	{
		RemoveCombatStateTag(FGameplayTag::RequestGameplayTag(FName("Action.Combat.Parry")));
		ParryStartTime = 0.0f;
		UE_LOG(LogTemp, Log, TEXT("Parry state deactivated"));
	}
}

bool UActionManagerComponent::IsParrying() const
{
	return bIsParrying;
}

void UActionManagerComponent::OnParrySuccess(AActor* Attacker, bool bPerfectParry)
{
	if (!Attacker)
		return;
	
	if (bPerfectParry)
	{
		// Apply vulnerability to attacker on perfect parry
		if (AGameCharacterBase* AttackerChar = Cast<AGameCharacterBase>(Attacker))
		{
			if (UVulnerabilityComponent* AttackerVuln = AttackerChar->FindComponentByClass<UVulnerabilityComponent>())
			{
				// Apply Stunned tier vulnerability
				AttackerVuln->ApplyVulnerabilityTier(EVulnerabilityTier::Stunned);
				UE_LOG(LogTemp, Log, TEXT("Perfect parry! Attacker stunned."));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Late parry - 50%% damage reduction"));
	}
}

bool UActionManagerComponent::IsInParryWindow(bool& bIsPerfectWindow, bool& bIsLateWindow) const
{
	if (!bIsParrying || ParryStartTime <= 0.0f)
	{
		bIsPerfectWindow = false;
		bIsLateWindow = false;
		return false;
	}
	
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	float TimeInParry = CurrentTime - ParryStartTime;
	
	bIsPerfectWindow = TimeInParry <= PerfectParryWindow;
	bIsLateWindow = !bIsPerfectWindow && TimeInParry <= (PerfectParryWindow + LateParryWindow);
	
	return bIsPerfectWindow || bIsLateWindow;
}

float UActionManagerComponent::GetParryDamageReduction(bool bPerfectParry) const
{
	return bPerfectParry ? 1.0f : 0.5f;  // 100% reduction for perfect, 50% for late
}

void UActionManagerComponent::ApplyVulnerabilityWithIFrames(int32 Charges, bool bGrantIFrames)
{
	if (VulnerabilityComponent)
	{
		float Duration = CombatRules ? CombatRules->CombatRules.VulnerabilityDuration : 1.0f;
		VulnerabilityComponent->ApplyVulnerability(Charges, Duration);
		
		if (bGrantIFrames && VulnerabilityComponent->bEnableIFrames)
		{
			VulnerabilityComponent->StartIFrames();
		}
		
		OnVulnerabilityApplied.Broadcast();
	}
}

void UActionManagerComponent::SetCurrentActionData(UActionDataAsset* ActionData)
{
	CurrentActionData = ActionData;
}

void UActionManagerComponent::ProcessHitFromAnimation(AGameCharacterBase* HitCharacter)
{
	// This is called by animation notifies when a hit is detected
	if (!HitCharacter || !CurrentActionData)
	{
		return;
	}
	
	// Use ActionDataAsset for damage calculation
	if (UHealthComponent* TargetHealth = HitCharacter->FindComponentByClass<UHealthComponent>())
	{
		// Check target's state for damage modifiers
		bool bIsTargetBlocking = false;
		bool bIsTargetVulnerable = false;
		bool bIsTargetParrying = false;
		bool bIsPerfectParry = false;
		bool bIsLateParry = false;
		
		if (UActionManagerComponent* TargetActionManager = HitCharacter->FindComponentByClass<UActionManagerComponent>())
		{
			bIsTargetBlocking = TargetActionManager->IsBlocking();
			bIsTargetVulnerable = TargetActionManager->IsVulnerable();
			bIsTargetParrying = TargetActionManager->IsParrying();
			
			// Check parry window timing
			if (bIsTargetParrying)
			{
				TargetActionManager->IsInParryWindow(bIsPerfectParry, bIsLateParry);
				
				if (bIsPerfectParry || bIsLateParry)
				{
					// Successful parry - trigger parry success event
					TargetActionManager->OnParrySuccess(GetOwner(), bIsPerfectParry);
				}
			}
		}
		
		// Calculate damage
		float FinalDamage = CalculateFinalDamage(CurrentActionData->MeleeDamage, bIsTargetBlocking, bIsTargetVulnerable);
		
		// Apply parry damage reduction if parrying
		if (bIsTargetParrying && (bIsPerfectParry || bIsLateParry))
		{
			float ParryReduction = bIsPerfectParry ? 1.0f : 0.5f;  // 100% or 50% reduction
			FinalDamage *= (1.0f - ParryReduction);
			
			// Check if attack is unblockable (Soul Attack)
			if (CurrentActionData && CurrentActionData->bIsUnblockable)
			{
				// Unblockable attacks bypass parry
				FinalDamage = CurrentActionData->MeleeDamage;
				UE_LOG(LogTemp, Warning, TEXT("Unblockable attack bypasses parry!"));
			}
		}
		
		TargetHealth->TakeDamage(FinalDamage, GetOwner());
		
		// Apply poise damage
		if (CurrentActionData->PoiseDamage > 0.0f)
		{
			TargetHealth->TakePoiseDamage(CurrentActionData->PoiseDamage);
		}
		
		// Apply knockback
		if (CurrentActionData->KnockbackForce > 0.0f)
		{
			FVector KnockbackDirection = (HitCharacter->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
			ApplyKnockback(HitCharacter, KnockbackDirection, CurrentActionData->KnockbackForce, CurrentActionData->bCausesRagdoll);
		}
		
		// Apply station integrity cost for high-risk abilities
		// Check if this is a soul attack that bypasses station damage
		bool bShouldDamageStation = true;
		if (CurrentActionData->ActionType == EActionType::Special && CurrentActionData->bBypassesStationDamage)
		{
			bShouldDamageStation = false;
			UE_LOG(LogTemp, Log, TEXT("Soul Attack bypasses station integrity damage"));
		}
		
		AAtlasGameState* GameState = AAtlasGameState::GetAtlasGameState(GetWorld());
		if (bShouldDamageStation && GameState && GameState->StationIntegrityComponent && CurrentActionData->IntegrityCost > 0.0f)
		{
			UStationIntegrityComponent* IntegrityComp = GameState->StationIntegrityComponent;
			IntegrityComp->ApplyAbilityIntegrityCost(CurrentActionData->ActionTag, GetOwner());
			
			if (IntegrityComp->GetIntegrityPercent() <= 50.0f)
			{
				UE_LOG(LogTemp, Error, TEXT("WARNING: Station integrity critical!"));
			}
		}
	}
}

float UActionManagerComponent::CalculateFinalDamage(float BaseDamage, bool bIsBlocking, bool bIsVulnerable) const
{
	float FinalDamage = BaseDamage;
	
	// Apply vulnerability multiplier (8x damage as per CLAUDE.md)
	if (bIsVulnerable)
	{
		FinalDamage *= 8.0f;
	}
	
	// Apply block damage reduction (40% reduction as per CLAUDE.md)
	if (bIsBlocking)
	{
		FinalDamage *= 0.6f; // 60% damage goes through (40% reduction)
	}
	
	return FinalDamage;
}

void UActionManagerComponent::ApplyKnockback(AGameCharacterBase* Target, const FVector& Direction, float Force, bool bCauseRagdoll)
{
	if (!Target)
	{
		return;
	}
	
	ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	if (!TargetCharacter)
	{
		return;
	}
	
	// Add slight upward force for better knockback feel
	FVector KnockbackDir = Direction;
	KnockbackDir.Z = 0.3f;
	KnockbackDir.Normalize();
	
	if (bCauseRagdoll)
	{
		// Apply ragdoll physics
		if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
		{
			Mesh->SetSimulatePhysics(true);
			Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Mesh->AddImpulse(KnockbackDir * Force * 100.0f); // Scale for physics impulse
		}
	}
	else
	{
		// Standard knockback using character movement
		TargetCharacter->LaunchCharacter(KnockbackDir * Force, true, true);
	}
}