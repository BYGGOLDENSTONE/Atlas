#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StationIntegrityDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FIntegrityThreshold
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float ThresholdPercent = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString ThresholdName = "Critical";

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag ThresholdTag;

    FIntegrityThreshold()
    {
        ThresholdPercent = 50.0f;
        ThresholdName = "Critical";
    }
};

USTRUCT(BlueprintType)
struct FAbilityIntegrityCost
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag AbilityTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float IntegrityCost = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString AbilityName = "Unknown Ability";

    FAbilityIntegrityCost()
    {
        IntegrityCost = 10.0f;
        AbilityName = "Unknown Ability";
    }
};

UCLASS()
class ATLAS_API UStationIntegrityDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station Integrity", meta = (ClampMin = "1.0"))
    float MaxIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station Integrity")
    TArray<FIntegrityThreshold> IntegrityThresholds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Costs")
    TArray<FAbilityIntegrityCost> HighRiskAbilities;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Costs")
    TArray<FAbilityIntegrityCost> MediumRiskAbilities;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Costs")
    TArray<FAbilityIntegrityCost> LowRiskAbilities;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Tags")
    FGameplayTag HighRiskTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Tags")
    FGameplayTag MediumRiskTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Tags")
    FGameplayTag LowRiskTag;

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    float GetIntegrityCostForAbility(const FGameplayTag& AbilityTag) const;

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    bool IsHighRiskAbility(const FGameplayTag& AbilityTag) const;

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    bool IsMediumRiskAbility(const FGameplayTag& AbilityTag) const;

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    bool IsLowRiskAbility(const FGameplayTag& AbilityTag) const;
};