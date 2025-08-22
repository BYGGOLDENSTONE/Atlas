#pragma once

#include "CoreMinimal.h"

// Forward declaration
class URunManagerComponent;

/**
 * Global singleton access to RunManager for console commands
 * This is a simple, foolproof approach that always works
 */
class ATLAS_API FGlobalRunManager
{
public:
	/** Get or create the global RunManager instance */
	static URunManagerComponent* Get();
	
	/** Set the RunManager (called from GameMode or wherever it's created) */
	static void Set(URunManagerComponent* InRunManager);
	
	/** Create a new RunManager if none exists */
	static URunManagerComponent* CreateNew();

private:
	static URunManagerComponent* Instance;
};