#pragma once

#include "CoreMinimal.h"
#include "AbilityDataAsset.h"
#include "../Abilities/SystemHackAbility.h"
#include "SystemHackDataAsset.generated.h"

UCLASS()
class ATLAS_API USystemHackDataAsset : public UAbilityDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Hack")
	FSystemHackConfig HackConfig;
};