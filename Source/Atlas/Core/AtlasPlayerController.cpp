#include "AtlasPlayerController.h"
#include "AtlasCheatManager.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

AAtlasPlayerController::AAtlasPlayerController()
{
	bShowMouseCursor = false;
	DefaultMouseCursor = EMouseCursor::Default;
	
	// Set our custom cheat manager class
	CheatClass = UAtlasCheatManager::StaticClass();
}

void AAtlasPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);
	SetShowMouseCursor(false);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
	}
}

void AAtlasPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}