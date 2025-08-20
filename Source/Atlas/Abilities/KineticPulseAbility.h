#pragma once

#include "CoreMinimal.h"
#include "AbilityBase.h"
#include "KineticPulseAbility.generated.h"

USTRUCT(BlueprintType)
struct FKineticPulseConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinetic Pulse")
	float PulseRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinetic Pulse")
	float PulseForce = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinetic Pulse")
	float PulseDamage = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinetic Pulse")
	float PulsePoiseDamage = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinetic Pulse")
	float UpwardForceMultiplier = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinetic Pulse")
	bool bAffectPhysicsObjects = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kinetic Pulse")
	float PhysicsImpulseMultiplier = 2.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UKineticPulseAbility : public UAbilityBase
{
	GENERATED_BODY()

public:
	UKineticPulseAbility();

protected:
	virtual void BeginPlay() override;
	virtual void ExecuteAbility() override;
	virtual bool CheckAbilitySpecificConditions() const override;

private:
	void ApplyPulseToActor(AActor* Target, const FVector& PulseOrigin);
	void ApplyPulseToPhysicsActor(AActor* Target, const FVector& PulseOrigin);
	FVector CalculatePulseDirection(const FVector& TargetLocation, const FVector& PulseOrigin) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic Pulse")
	class UKineticPulseDataAsset* PulseDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic Pulse", meta = (EditCondition = "!PulseDataAsset"))
	FKineticPulseConfig DefaultConfig;

private:
	FKineticPulseConfig GetPulseConfig() const;

	UPROPERTY()
	class UDamageCalculator* DamageCalculator;
};