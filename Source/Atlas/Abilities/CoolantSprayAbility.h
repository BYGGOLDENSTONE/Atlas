#pragma once

#include "CoreMinimal.h"
#include "AbilityBase.h"
#include "CoolantSprayAbility.generated.h"

USTRUCT(BlueprintType)
struct FCoolantSprayConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant Spray")
	float SprayRange = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant Spray")
	float SprayRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant Spray")
	float HazardDuration = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant Spray")
	float SlipForce = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant Spray")
	float FrictionMultiplier = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant Spray")
	float SlipPoiseDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant Spray")
	float TickRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant Spray")
	bool bAffectsOwner = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UCoolantSprayAbility : public UAbilityBase
{
	GENERATED_BODY()

public:
	UCoolantSprayAbility();

protected:
	virtual void ExecuteAbility() override;
	virtual bool CheckAbilitySpecificConditions() const override;

private:
	FVector CalculateSprayLocation() const;
	void SpawnHazardZone(const FVector& Location);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coolant Spray")
	class UCoolantSprayDataAsset* SprayDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coolant Spray", meta = (EditCondition = "!SprayDataAsset"))
	FCoolantSprayConfig DefaultConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coolant Spray")
	TSubclassOf<class ACoolantHazard> HazardActorClass;

private:
	FCoolantSprayConfig GetSprayConfig() const;
};