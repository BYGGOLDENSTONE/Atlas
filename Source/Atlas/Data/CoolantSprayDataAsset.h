#pragma once

#include "CoreMinimal.h"
#include "AbilityDataAsset.h"
#include "../Abilities/CoolantSprayAbility.h"
#include "CoolantSprayDataAsset.generated.h"

UCLASS()
class ATLAS_API UCoolantSprayDataAsset : public UAbilityDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coolant Spray")
	FCoolantSprayConfig SprayConfig;
};