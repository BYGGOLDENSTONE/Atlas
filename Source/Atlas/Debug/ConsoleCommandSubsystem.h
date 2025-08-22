#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ConsoleCommandSubsystem.generated.h"

/**
 * Subsystem that ensures console commands are always registered
 * regardless of GameMode configuration
 */
UCLASS()
class ATLAS_API UConsoleCommandSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// UGameInstanceSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
private:
	bool bCommandsRegistered;
};