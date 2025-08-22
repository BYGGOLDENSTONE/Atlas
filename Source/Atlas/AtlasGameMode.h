// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AtlasGameMode.generated.h"

// Forward declarations
class URunManagerComponent;

UCLASS(minimalapi)
class AAtlasGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAtlasGameMode();
	
	/** Called when the game starts */
	virtual void BeginPlay() override;
	
	/** Called when the game ends */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** Get the run manager component */
	UFUNCTION(BlueprintPure, Category = "Atlas")
	URunManagerComponent* GetRunManager() const { return RunManagerComponent; }

protected:
	/** Component that manages room progression and run state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URunManagerComponent* RunManagerComponent;
};



