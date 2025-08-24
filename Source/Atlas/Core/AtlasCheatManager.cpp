#include "AtlasCheatManager.h"
#include "../Components/ActionManagerComponent.h"
#include "../Components/RunManagerComponent.h"
#include "../Characters/PlayerCharacter.h"
#include "../Actions/BaseAction.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
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
		UE_LOG(LogTemp, Log, TEXT("Action.Combat.Dash - Dash movement ability"));
		UE_LOG(LogTemp, Log, TEXT("Action.Combat.BasicAttack - Basic melee attack"));
		UE_LOG(LogTemp, Log, TEXT("Action.Combat.HeavyAttack - Heavy melee attack"));
		UE_LOG(LogTemp, Log, TEXT("Action.Combat.Block - Defensive block"));
		UE_LOG(LogTemp, Log, TEXT("Action.KineticPulse - Force push ability"));
		UE_LOG(LogTemp, Log, TEXT("Action.DebrisPull - Magnetic pull ability"));
		UE_LOG(LogTemp, Log, TEXT("Action.CoolantSpray - Area hazard ability"));
		UE_LOG(LogTemp, Log, TEXT("Action.SystemHack - Remote hack ability"));
		
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
		ActionManager->AssignActionToSlot(TEXT("Slot1"), FGameplayTag::RequestGameplayTag(FName("Action.Combat.BasicAttack")));
		ActionManager->AssignActionToSlot(TEXT("Slot2"), FGameplayTag::RequestGameplayTag(FName("Action.Combat.Block")));
		ActionManager->AssignActionToSlot(TEXT("Slot3"), FGameplayTag::RequestGameplayTag(FName("Action.Combat.HeavyAttack")));
		ActionManager->AssignActionToSlot(TEXT("Slot4"), FGameplayTag::RequestGameplayTag(FName("Action.KineticPulse")));
		ActionManager->AssignActionToSlot(TEXT("Slot5"), FGameplayTag::RequestGameplayTag(FName("Action.Combat.Dash")));
		
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

// ========================================
// ROOM TESTING COMMANDS
// ========================================

void UAtlasCheatManager::Atlas_Room_GoTo(const FString& RoomName)
{
	if (URunManagerComponent* RunManager = GetRunManager())
	{
		RunManager->GoToRoom(RoomName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find RunManagerComponent. Make sure your GameMode has one."));
	}
}

void UAtlasCheatManager::Atlas_Room_CompleteTest()
{
	if (URunManagerComponent* RunManager = GetRunManager())
	{
		RunManager->CompleteRoomTest();
	}
}

void UAtlasCheatManager::Atlas_Room_ResetAll()
{
	if (URunManagerComponent* RunManager = GetRunManager())
	{
		RunManager->ResetAllRooms();
	}
}

void UAtlasCheatManager::Atlas_Room_Debug()
{
	if (URunManagerComponent* RunManager = GetRunManager())
	{
		RunManager->DebugRooms();
	}
}

void UAtlasCheatManager::Atlas_Room_TestSequence()
{
	if (URunManagerComponent* RunManager = GetRunManager())
	{
		RunManager->TestRoomSequence();
	}
}

URunManagerComponent* UAtlasCheatManager::GetRunManager() const
{
	if (UWorld* World = GetWorld())
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode())
		{
			// Try to find RunManagerComponent on the GameMode
			if (URunManagerComponent* RunManager = GameMode->FindComponentByClass<URunManagerComponent>())
			{
				return RunManager;
			}
			
			// If not on GameMode, try GameState
			if (AGameStateBase* GameState = World->GetGameState())
			{
				if (URunManagerComponent* RunManager = GameState->FindComponentByClass<URunManagerComponent>())
				{
					return RunManager;
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Could not find RunManagerComponent. Make sure it's attached to your GameMode or GameState."));
	return nullptr;
}