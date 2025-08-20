#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "AtlasCheatManager.generated.h"

/**
 * Cheat manager for Atlas game, handles console commands for the action system
 */
UCLASS()
class ATLAS_API UAtlasCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	// Action System Commands
	UFUNCTION(Exec, Category = "Atlas|Actions")
	void Atlas_AssignAction(const FString& SlotName, const FString& ActionTag);

	UFUNCTION(Exec, Category = "Atlas|Actions")
	void Atlas_ClearSlot(const FString& SlotName);

	UFUNCTION(Exec, Category = "Atlas|Actions")
	void Atlas_SwapSlots(const FString& Slot1, const FString& Slot2);

	UFUNCTION(Exec, Category = "Atlas|Actions")
	void Atlas_ListActions();

	UFUNCTION(Exec, Category = "Atlas|Actions")
	void Atlas_ShowSlots();

	UFUNCTION(Exec, Category = "Atlas|Actions")
	void Atlas_ResetSlots();

	// Helper function to get action manager
	class UActionManagerComponent* GetActionManager() const;
};