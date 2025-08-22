#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RunManagerSubsystem.generated.h"

// Forward declarations
class URunManagerComponent;
class URoomDataAsset;

/**
 * Game Instance Subsystem that provides global access to the RunManager.
 * This ensures the RunManager is always available regardless of GameMode setup.
 */
UCLASS()
class ATLAS_API URunManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// UGameInstanceSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	/**
	 * Get the run manager component (creates it if needed)
	 * @return The run manager component
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager")
	URunManagerComponent* GetRunManager();
	
	/**
	 * Static helper to get the subsystem from world context
	 * @param WorldContext Any object with world context
	 * @return The run manager subsystem
	 */
	UFUNCTION(BlueprintPure, Category = "Run Manager", meta = (WorldContext = "WorldContextObject"))
	static URunManagerSubsystem* Get(const UObject* WorldContextObject);

protected:
	/** The actual run manager component */
	UPROPERTY()
	URunManagerComponent* RunManagerComponent;
	
	/** Dummy actor to hold the component */
	UPROPERTY()
	AActor* ComponentHolder;
};