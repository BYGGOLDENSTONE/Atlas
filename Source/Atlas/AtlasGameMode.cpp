// Copyright Epic Games, Inc. All Rights Reserved.

#include "AtlasGameMode.h"
#include "Characters/PlayerCharacter.h"
#include "Core/AtlasPlayerController.h"
#include "Core/AtlasGameState.h"
#include "Debug/Phase3ConsoleCommands.h"
#include "Debug/Phase4ConsoleCommands.h"
#include "UObject/ConstructorHelpers.h"

AAtlasGameMode::AAtlasGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
	PlayerControllerClass = AAtlasPlayerController::StaticClass();
	GameStateClass = AAtlasGameState::StaticClass();
}

void AAtlasGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// Register Phase 3 console commands
	UPhase3ConsoleCommands::RegisterCommands();
	
	// Register Phase 4 console commands
	UPhase4ConsoleCommands::RegisterCommands();
	
	UE_LOG(LogTemp, Log, TEXT("Atlas GameMode: Phase 3 and Phase 4 console commands registered"));
}

void AAtlasGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister Phase 3 console commands
	UPhase3ConsoleCommands::UnregisterCommands();
	
	// Unregister Phase 4 console commands
	UPhase4ConsoleCommands::UnregisterCommands();
	
	Super::EndPlay(EndPlayReason);
}
