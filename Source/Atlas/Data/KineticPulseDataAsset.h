#pragma once

#include "CoreMinimal.h"
#include "AbilityDataAsset.h"
#include "../Abilities/KineticPulseAbility.h"
#include "KineticPulseDataAsset.generated.h"

UCLASS()
class ATLAS_API UKineticPulseDataAsset : public UAbilityDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic Pulse")
	FKineticPulseConfig PulseConfig;
};