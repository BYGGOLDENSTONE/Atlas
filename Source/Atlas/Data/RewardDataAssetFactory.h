#pragma once

#include "CoreMinimal.h"
#include "Atlas/Data/RewardDataAsset.h"
#include "Engine/DeveloperSettings.h"
#include "RewardDataAssetFactory.generated.h"

/**
 * Factory class that programmatically defines all reward DataAsset configurations
 * These serve as blueprints for creating actual DataAssets in the editor
 */
UCLASS(Config=Game, DefaultConfig, meta = (DisplayName = "Reward DataAsset Factory"))
class ATLAS_API URewardDataAssetFactory : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ========================================
	// FACTORY METHODS
	// ========================================
	
	/** Create all reward DataAsset definitions for Phase 3 */
	static void CreateAllRewardDefinitions();
	
	/** Defense Category Rewards */
	static URewardDataAsset* CreateImprovedBlockReward();
	static URewardDataAsset* CreateParryMasterReward();
	static URewardDataAsset* CreateCounterStrikeReward();
	static URewardDataAsset* CreateIronSkinReward();
	static URewardDataAsset* CreateLastStandReward();
	
	/** Offense Category Rewards */
	static URewardDataAsset* CreateSharpBladeReward();
	static URewardDataAsset* CreateHeavyImpactReward();
	static URewardDataAsset* CreateBleedingStrikesReward();
	static URewardDataAsset* CreateExecutionerReward();
	static URewardDataAsset* CreateRapidStrikesReward();
	
	/** Passive Stats Rewards */
	static URewardDataAsset* CreateVitalityReward();
	static URewardDataAsset* CreateSwiftnessReward();
	static URewardDataAsset* CreateHeavyweightReward();
	static URewardDataAsset* CreateRegenerationReward();
	static URewardDataAsset* CreateFortitudeReward();
	
	/** Passive Abilities Rewards */
	static URewardDataAsset* CreateSecondWindReward();
	static URewardDataAsset* CreateVampirismReward();
	static URewardDataAsset* CreateBerserkerReward();
	static URewardDataAsset* CreateMomentumReward();
	static URewardDataAsset* CreateStationShieldReward();
	
	/** Interactables Rewards */
	static URewardDataAsset* CreateExplosiveValvesReward();
	static URewardDataAsset* CreateGravityWellsReward();
	static URewardDataAsset* CreateTurretHackReward();
	static URewardDataAsset* CreateEmergencyVentReward();
	static URewardDataAsset* CreatePowerSurgeReward();
	
private:
	/** Helper to create a new reward DataAsset */
	static URewardDataAsset* CreateRewardBase(const FString& Name, ERewardCategory Category);
};

/**
 * Configuration struct for reward DataAsset definitions
 * Used to store reward configurations that can be referenced when creating DataAssets
 */
USTRUCT(BlueprintType)
struct FRewardDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString AssetName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag RewardTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText RewardName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERewardCategory Category;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SlotCost = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxStackLevel = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<float> StackMultipliers = {1.0f, 1.5f, 2.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, float> StatModifiers;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag PassiveAbilityTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, float> PassiveParameters;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag InteractableType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InteractionRange = 500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InteractableCooldown = 5.0f;
	
	FRewardDefinition() {}
};

/**
 * Storage class for all reward definitions
 * This can be used in the editor to view all reward configurations
 */
UCLASS(BlueprintType)
class ATLAS_API URewardDataAssetStorage : public UObject
{
	GENERATED_BODY()
	
public:
	/** All reward definitions for Phase 3 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	TArray<FRewardDefinition> DefenseRewards;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Offense")
	TArray<FRewardDefinition> OffenseRewards;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Passive Stats")
	TArray<FRewardDefinition> PassiveStatRewards;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Passive Abilities")
	TArray<FRewardDefinition> PassiveAbilityRewards;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactables")
	TArray<FRewardDefinition> InteractableRewards;
	
	/** Initialize all reward definitions */
	void InitializeDefinitions();
};