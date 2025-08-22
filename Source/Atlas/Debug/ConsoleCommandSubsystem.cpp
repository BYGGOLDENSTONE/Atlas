#include "ConsoleCommandSubsystem.h"
#include "Phase3ConsoleCommands.h"
#include "Phase4ConsoleCommands.h"
#include "Engine/Engine.h"

void UConsoleCommandSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Register all console commands
	if (!bCommandsRegistered)
	{
		UE_LOG(LogTemp, Log, TEXT("ConsoleCommandSubsystem: Registering Atlas console commands"));
		
		// Register Phase 3 commands
		UPhase3ConsoleCommands::RegisterCommands();
		
		// Register Phase 4 commands
		UPhase4ConsoleCommands::RegisterCommands();
		
		bCommandsRegistered = true;
		
		UE_LOG(LogTemp, Log, TEXT("ConsoleCommandSubsystem: All Atlas console commands registered successfully"));
		
		// List some registered commands for verification
		UE_LOG(LogTemp, Log, TEXT("Available commands:"));
		UE_LOG(LogTemp, Log, TEXT("  - Atlas.Phase3.StartRun"));
		UE_LOG(LogTemp, Log, TEXT("  - Atlas.Phase3.ListRewards"));
		UE_LOG(LogTemp, Log, TEXT("  - Atlas.Phase3.ShowSlots"));
		UE_LOG(LogTemp, Log, TEXT("  - Atlas.Phase4.TestAll"));
		UE_LOG(LogTemp, Log, TEXT("  - Atlas.ShowIntegrityStatus"));
	}
}

void UConsoleCommandSubsystem::Deinitialize()
{
	if (bCommandsRegistered)
	{
		// Unregister commands
		UPhase3ConsoleCommands::UnregisterCommands();
		UPhase4ConsoleCommands::UnregisterCommands();
		
		bCommandsRegistered = false;
		
		UE_LOG(LogTemp, Log, TEXT("ConsoleCommandSubsystem: Console commands unregistered"));
	}
	
	Super::Deinitialize();
}