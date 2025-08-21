#pragma once

#include "CoreMinimal.h"
#include "Atlas/Data/RoomDataAsset.h"
#include "Engine/DeveloperSettings.h"
#include "RoomDataAssetFactory.generated.h"

/**
 * Factory class that programmatically defines all room DataAsset configurations
 * These serve as blueprints for creating actual DataAssets in the editor
 */
UCLASS(Config=Game, DefaultConfig, meta = (DisplayName = "Room DataAsset Factory"))
class ATLAS_API URoomDataAssetFactory : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ========================================
	// FACTORY METHODS
	// ========================================
	
	/** Create all room DataAsset definitions for Phase 3 */
	static void CreateAllRoomDefinitions();
	
	/** Individual Room Creation Methods */
	static URoomDataAsset* CreateEngineeringBayRoom();    // Room A
	static URoomDataAsset* CreateMedicalWardRoom();        // Room B
	static URoomDataAsset* CreateWeaponsLabRoom();         // Room C
	static URoomDataAsset* CreateCommandCenterRoom();      // Room D
	static URoomDataAsset* CreateMaintenanceShaftRoom();   // Room E
	
private:
	/** Helper to create a new room DataAsset */
	static URoomDataAsset* CreateRoomBase(const FString& Name, ERewardCategory Theme);
	
	/** Helper to add reward choices to a room */
	static void AddRewardToPool(URoomDataAsset* Room, const FString& RewardAssetPath, 
		float Weight = 1.0f, int32 MinLevel = 1, int32 MaxAppear = 999);
};

/**
 * Configuration struct for room DataAsset definitions
 * Used to store room configurations that can be referenced when creating DataAssets
 */
USTRUCT(BlueprintType)
struct FRoomDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString AssetName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RoomID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText RoomName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERewardCategory RoomTheme;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERoomDifficulty Difficulty;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERoomHazard EnvironmentalHazard;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> AppearOnLevels;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RoomSelectionWeight;
	
	// Enemy Configuration
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString EnemyBlueprintPath;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText EnemyName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText EnemyDescription;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 EnemyBasePower;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag EnemyAIPreset;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FGameplayTag> EnemyAbilities;
	
	// Reward Pool References
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FString> RewardPoolAssetPaths;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<float> RewardWeights;
	
	// Environmental Setup
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString RoomLevelPath;
	
	FRoomDefinition() 
	{
		Difficulty = ERoomDifficulty::Medium;
		EnvironmentalHazard = ERoomHazard::None;
		RoomSelectionWeight = 1.0f;
		EnemyBasePower = 3;
	}
};

/**
 * Storage class for all room definitions
 * This can be used in the editor to view all room configurations
 */
UCLASS(BlueprintType)
class ATLAS_API URoomDataAssetStorage : public UObject
{
	GENERATED_BODY()
	
public:
	/** All room definitions for Phase 3 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rooms")
	TArray<FRoomDefinition> AllRooms;
	
	/** Initialize all room definitions */
	void InitializeDefinitions();
	
	/** Get room definition by ID */
	UFUNCTION(BlueprintPure, Category = "Rooms")
	FRoomDefinition GetRoomByID(FName RoomID) const;
	
	/** Get rooms valid for a specific level */
	UFUNCTION(BlueprintPure, Category = "Rooms")
	TArray<FRoomDefinition> GetRoomsForLevel(int32 Level) const;
};