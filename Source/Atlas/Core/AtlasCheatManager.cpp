#include "AtlasCheatManager.h"
#include "../Components/ActionManagerComponent.h"
#include "../Characters/PlayerCharacter.h"
#include "../Actions/BaseAction.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"

void UAtlasCheatManager::Atlas_AssignAction(const FString& SlotName, const FString& ActionTag)
{
	if (UActionManagerComponent* ActionManager = GetActionManager())
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*ActionTag));
		if (!Tag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid action tag: %s"), *ActionTag);
			UE_LOG(LogTemp, Warning, TEXT("Usage: Atlas.AssignAction Slot1 Action.Dash"));
			return;
		}

		if (ActionManager->AssignActionToSlot(FName(*SlotName), Tag))
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully assigned %s to %s"), *ActionTag, *SlotName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to assign %s to %s"), *ActionTag, *SlotName);
		}
	}
}

void UAtlasCheatManager::Atlas_ClearSlot(const FString& SlotName)
{
	if (UActionManagerComponent* ActionManager = GetActionManager())
	{
		ActionManager->ClearSlot(FName(*SlotName));
		UE_LOG(LogTemp, Log, TEXT("Cleared slot: %s"), *SlotName);
	}
}

void UAtlasCheatManager::Atlas_SwapSlots(const FString& Slot1, const FString& Slot2)
{
	if (UActionManagerComponent* ActionManager = GetActionManager())
	{
		ActionManager->SwapSlots(FName(*Slot1), FName(*Slot2));
		UE_LOG(LogTemp, Log, TEXT("Swapped %s and %s"), *Slot1, *Slot2);
	}
}

void UAtlasCheatManager::Atlas_ListActions()
{
	if (UActionManagerComponent* ActionManager = GetActionManager())
	{
		UE_LOG(LogTemp, Log, TEXT("=== Available Actions ==="));
		UE_LOG(LogTemp, Log, TEXT("Action.Dash - Dash movement ability"));
		UE_LOG(LogTemp, Log, TEXT("Action.Attack.Basic - Basic melee attack"));
		UE_LOG(LogTemp, Log, TEXT("Action.Attack.Heavy - Heavy melee attack"));
		UE_LOG(LogTemp, Log, TEXT("Action.Block - Defensive block"));
		UE_LOG(LogTemp, Log, TEXT("Action.Ability.KineticPulse - Force push ability"));
		UE_LOG(LogTemp, Log, TEXT("Action.Ability.DebrisPull - Magnetic pull ability"));
		UE_LOG(LogTemp, Log, TEXT("Action.Ability.CoolantSpray - Area hazard ability"));
		UE_LOG(LogTemp, Log, TEXT("Action.Ability.SystemHack - Remote hack ability"));
		
		// Also list from available data assets
		ActionManager->ExecuteListActionsCommand();
	}
}

void UAtlasCheatManager::Atlas_ShowSlots()
{
	if (UActionManagerComponent* ActionManager = GetActionManager())
	{
		UE_LOG(LogTemp, Log, TEXT("=== Current Slot Assignments ==="));
		
		TArray<FName> SlotNames = ActionManager->GetAllSlotNames();
		for (const FName& SlotName : SlotNames)
		{
			if (UBaseAction* Action = ActionManager->GetActionInSlot(SlotName))
			{
				FString Status = Action->GetCooldownRemaining() > 0.0f ? 
					FString::Printf(TEXT(" (Cooldown: %.1fs)"), Action->GetCooldownRemaining()) : 
					TEXT(" (Ready)");
				
				UE_LOG(LogTemp, Log, TEXT("  %s: %s%s"), 
					*SlotName.ToString(), 
					*Action->GetActionTag().ToString(),
					*Status);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("  %s: [Empty]"), *SlotName.ToString());
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("=== Input Mapping ==="));
		UE_LOG(LogTemp, Log, TEXT("  Slot1: Left Mouse Button"));
		UE_LOG(LogTemp, Log, TEXT("  Slot2: Right Mouse Button"));
		UE_LOG(LogTemp, Log, TEXT("  Slot3: E Key"));
		UE_LOG(LogTemp, Log, TEXT("  Slot4: R Key"));
		UE_LOG(LogTemp, Log, TEXT("  Slot5: Space Bar"));
	}
}

void UAtlasCheatManager::Atlas_ResetSlots()
{
	if (UActionManagerComponent* ActionManager = GetActionManager())
	{
		// Clear all slots
		TArray<FName> SlotNames = ActionManager->GetAllSlotNames();
		for (const FName& SlotName : SlotNames)
		{
			ActionManager->ClearSlot(SlotName);
		}
		
		// Set default configuration
		ActionManager->AssignActionToSlot(TEXT("Slot1"), FGameplayTag::RequestGameplayTag(FName("Action.Attack.Basic")));
		ActionManager->AssignActionToSlot(TEXT("Slot2"), FGameplayTag::RequestGameplayTag(FName("Action.Block")));
		ActionManager->AssignActionToSlot(TEXT("Slot3"), FGameplayTag::RequestGameplayTag(FName("Action.Attack.Heavy")));
		ActionManager->AssignActionToSlot(TEXT("Slot4"), FGameplayTag::RequestGameplayTag(FName("Action.Ability.KineticPulse")));
		ActionManager->AssignActionToSlot(TEXT("Slot5"), FGameplayTag::RequestGameplayTag(FName("Action.Dash")));
		
		UE_LOG(LogTemp, Log, TEXT("Reset slots to default configuration"));
		Atlas_ShowSlots();
	}
}

UActionManagerComponent* UAtlasCheatManager::GetActionManager() const
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Pawn))
			{
				return PlayerChar->FindComponentByClass<UActionManagerComponent>();
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Could not find ActionManagerComponent"));
	return nullptr;
}