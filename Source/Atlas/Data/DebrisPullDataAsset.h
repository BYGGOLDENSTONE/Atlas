#pragma once

#include "CoreMinimal.h"
#include "AbilityDataAsset.h"
#include "../Abilities/DebrisPullAbility.h"
#include "DebrisPullDataAsset.generated.h"

UCLASS()
class ATLAS_API UDebrisPullDataAsset : public UAbilityDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debris Pull")
	FDebrisPullConfig PullConfig;
};