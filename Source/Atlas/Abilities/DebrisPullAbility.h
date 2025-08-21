#pragma once

#include "CoreMinimal.h"
#include "AbilityBase.h"
#include "DebrisPullAbility.generated.h"

USTRUCT(BlueprintType)
struct FDebrisPullConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Pull")
	float PullRadius = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Pull")
	float PullForce = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Pull")
	float PullDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Pull")
	float MaxObjectMass = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Pull")
	float MinDistanceFromPlayer = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Pull")
	bool bLiftObjects = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Pull")
	float LiftForce = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris Pull")
	int32 MaxObjectsToPull = 5;
};

USTRUCT()
struct FPulledObject
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* Actor = nullptr;

	UPROPERTY()
	UPrimitiveComponent* Component = nullptr;

	float InitialDistance = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UDebrisPullAbility : public UAbilityBase
{
	GENERATED_BODY()

public:
	UDebrisPullAbility();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void ExecuteAbility() override;
	virtual void OnAbilityEnd() override;
	virtual bool CheckAbilitySpecificConditions() const override;

private:
	void FindDebrisObjects();
	void UpdatePull(float DeltaTime);
	void ApplyPullForce(const FPulledObject& PulledObj, float DeltaTime);
	void ReleaseObjects();
	bool IsValidDebrisObject(AActor* Actor, UPrimitiveComponent* Component) const;

public:
	// DataAsset removed - use ActionDataAsset system instead

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debris Pull", meta = (EditCondition = "!PullDataAsset"))
	FDebrisPullConfig DefaultConfig;

private:
	FDebrisPullConfig GetPullConfig() const;

	UPROPERTY()
	TArray<FPulledObject> PulledObjects;

	float PullTimer;
	bool bIsPulling;
};