// Copyright Epic Games, Inc. All Rights Reserved.

#include "AtlasGameMode.h"
#include "Characters/PlayerCharacter.h"
#include "Core/AtlasPlayerController.h"
#include "Core/AtlasGameState.h"
#include "Components/RunManagerComponent.h"
#include "Debug/Phase3ConsoleCommands.h"
#include "Debug/Phase4ConsoleCommands.h"
#include "Debug/GlobalRunManager.h"
#include "UObject/ConstructorHelpers.h"

AAtlasGameMode::AAtlasGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
	PlayerControllerClass = AAtlasPlayerController::StaticClass();
	GameStateClass = AAtlasGameState::StaticClass();
	
	// Create the RunManagerComponent
	RunManagerComponent = CreateDefaultSubobject<URunManagerComponent>(TEXT("RunManagerComponent"));
}

void AAtlasGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// Ensure RunManagerComponent exists (in case Blueprint didn't create it)
	if (!RunManagerComponent)
	{
		RunManagerComponent = NewObject<URunManagerComponent>(this, TEXT("RunManagerComponent"));
		RunManagerComponent->RegisterComponent();
		UE_LOG(LogTemp, Warning, TEXT("AtlasGameMode: Created RunManagerComponent at runtime (should be created in constructor or Blueprint)"));
	}
	
	// Register with global singleton for console command access
	if (RunManagerComponent)
	{
		FGlobalRunManager::Set(RunManagerComponent);
		UE_LOG(LogTemp, Log, TEXT("AtlasGameMode: Registered RunManager with global singleton"));
	}
	
	// Register Phase 3 console commands
	UPhase3ConsoleCommands::RegisterCommands();
	
	// Register Phase 4 console commands
	UPhase4ConsoleCommands::RegisterCommands();
	
	UE_LOG(LogTemp, Log, TEXT("Atlas GameMode: Phase 3 and Phase 4 console commands registered"));
	UE_LOG(LogTemp, Log, TEXT("Atlas GameMode: RunManager is %s"), RunManagerComponent ? TEXT("READY") : TEXT("NULL"));
}

void AAtlasGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister Phase 3 console commands
	UPhase3ConsoleCommands::UnregisterCommands();
	
	// Unregister Phase 4 console commands
	UPhase4ConsoleCommands::UnregisterCommands();
	
	Super::EndPlay(EndPlayReason);
}
