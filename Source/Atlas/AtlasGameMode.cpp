// Copyright Epic Games, Inc. All Rights Reserved.

#include "AtlasGameMode.h"
#include "Characters/PlayerCharacter.h"
#include "Core/AtlasPlayerController.h"
#include "Core/AtlasGameState.h"
#include "UObject/ConstructorHelpers.h"

AAtlasGameMode::AAtlasGameMode()
{
	DefaultPawnClass = APlayerCharacter::StaticClass();
	PlayerControllerClass = AAtlasPlayerController::StaticClass();
	GameStateClass = AAtlasGameState::StaticClass();
}
