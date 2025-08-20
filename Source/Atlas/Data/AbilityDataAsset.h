#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "../Abilities/AbilityBase.h"
#include "AbilityDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FAbilityConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FString AbilityName = "Unnamed Ability";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability", meta = (MultiLine = true))
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float Cooldown = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float ExecutionDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float Range = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	EAbilityRiskTier RiskTier = EAbilityRiskTier::LowRisk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float IntegrityCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FGameplayTagContainer BlockedTags;
};

UCLASS(BlueprintType)
class ATLAS_API UAbilityDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FAbilityConfig AbilityConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Visual")
	UTexture2D* AbilityIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Visual")
	FLinearColor AbilityColor = FLinearColor::White;
};