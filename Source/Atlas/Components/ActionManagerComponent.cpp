#include "ActionManagerComponent.h"
#include "../Actions/BaseAction.h"
#include "../DataAssets/ActionDataAsset.h"
#include "../Characters/GameCharacterBase.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

UActionManagerComponent::UActionManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentAction = nullptr;
	OwnerCharacter = nullptr;

	// Initialize default slot names
	InitializeSlots();
}

void UActionManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AGameCharacterBase>(GetOwner());
	
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

	if (!ActionData || !ActionData->ActionClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionManager: Invalid action data or action class"));
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
	UBaseAction* NewAction = CreateActionInstance(ActionData);
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

	UBaseAction* TempAction = ActionSlots[Slot1];
	ActionSlots[Slot1] = ActionSlots[Slot2];
	ActionSlots[Slot2] = TempAction;

	OnActionSlotChanged.Broadcast(Slot1, ActionSlots[Slot1]);
	OnActionSlotChanged.Broadcast(Slot2, ActionSlots[Slot2]);
}

UBaseAction* UActionManagerComponent::GetActionInSlot(FName SlotName) const
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

	UBaseAction* Action = GetActionInSlot(SlotName);
	if (!Action)
	{
		return;
	}

	// Check if we need to interrupt current action
	if (CurrentAction && CurrentAction != Action && CurrentAction->IsActive())
	{
		// For now, interrupt the current action
		// Later we might add priority or queuing system
		InterruptCurrentAction();
	}

	// Try to activate the action
	if (Action->CanActivate(OwnerCharacter))
	{
		CurrentAction = Action;
		Action->OnActivate(OwnerCharacter);
		OnActionActivated.Broadcast(SlotName, Action);
	}
}

void UActionManagerComponent::OnSlotReleased(FName SlotName)
{
	UBaseAction* Action = GetActionInSlot(SlotName);
	if (!Action)
	{
		return;
	}

	// Only process release if this is the current active action
	if (CurrentAction == Action && Action->IsActive())
	{
		Action->OnRelease();
		CurrentAction = nullptr;
	}
}

void UActionManagerComponent::InterruptCurrentAction()
{
	if (CurrentAction && CurrentAction->IsActive())
	{
		CurrentAction->OnInterrupted();
		CurrentAction = nullptr;
	}
}

void UActionManagerComponent::LoadAvailableActions()
{
	// If actions were already set in editor, keep them
	if (AvailableActionDataAssets.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("ActionManager: Using %d pre-configured action data assets"), AvailableActionDataAssets.Num());
		return;
	}
	
	// Manually load known action data assets
	// In production, this would use asset registry to scan the directory
	const TArray<FString> ActionAssetPaths = {
		TEXT("/Game/Atlas/DataAssets/Actions/DA_BasicAttack"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_HeavyAttack"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_Block"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_Dash"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_FocusMode"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_KineticPulse"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_DebrisPull"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_CoolantSpray"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_SystemHack"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_FloorDestabilizer"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_ImpactGauntlet"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_LocalizedEMP"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_SeismicStamp"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_GravityAnchor"),
		TEXT("/Game/Atlas/DataAssets/Actions/DA_AirlockBreach")
	};
	
	for (const FString& AssetPath : ActionAssetPaths)
	{
		UActionDataAsset* ActionData = LoadObject<UActionDataAsset>(nullptr, *AssetPath);
		if (ActionData)
		{
			AvailableActionDataAssets.Add(ActionData);
			UE_LOG(LogTemp, Log, TEXT("ActionManager: Loaded action data asset: %s"), *ActionData->ActionName);
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

UBaseAction* UActionManagerComponent::CreateActionInstance(UActionDataAsset* ActionData)
{
	if (!ActionData || !ActionData->ActionClass)
	{
		return nullptr;
	}

	UBaseAction* NewAction = NewObject<UBaseAction>(this, ActionData->ActionClass);
	if (NewAction)
	{
		NewAction->SetDataAsset(ActionData);
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
			Slot.Value->OnTick(DeltaTime);
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