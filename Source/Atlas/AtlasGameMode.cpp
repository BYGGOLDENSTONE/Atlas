// Copyright Epic Games, Inc. All Rights Reserved.

#include "AtlasGameMode.h"
#include "Characters/PlayerCharacter.h"
#include "Core/AtlasPlayerController.h"
#include "Core/AtlasGameState.h"
#include "Components/RunManagerComponent.h"
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
	
	UE_LOG(LogTemp, Log, TEXT("Atlas GameMode: RunManager is %s"), RunManagerComponent ? TEXT("READY") : TEXT("NULL"));
}

void AAtlasGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}
