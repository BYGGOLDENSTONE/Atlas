#pragma once

#include "CoreMinimal.h"
#include "AbilityBase.h"
#include "SystemHackAbility.generated.h"

USTRUCT(BlueprintType)
struct FSystemHackConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Hack")
	float HackRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Hack")
	float HackDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Hack")
	float HackAngle = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Hack")
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Hack")
	bool bCanHackMultiple = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Hack")
	int32 MaxSimultaneousHacks = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Hack")
	bool bOverrideInteractCooldown = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Hack")
	TArray<FName> HackableTags;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API USystemHackAbility : public UAbilityBase
{
	GENERATED_BODY()

public:
	USystemHackAbility();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void ExecuteAbility() override;
	virtual void OnAbilityEnd() override;
	virtual bool CheckAbilitySpecificConditions() const override;

private:
	AActor* FindBestHackTarget() const;
	bool IsValidHackTarget(AActor* Target) const;
	bool HasLineOfSight(AActor* Target) const;
	void StartHacking(AActor* Target);
	void CompleteHack();
	void CancelHack();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Hack")
	class USystemHackDataAsset* HackDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Hack", meta = (EditCondition = "!HackDataAsset"))
	FSystemHackConfig DefaultConfig;

private:
	FSystemHackConfig GetHackConfig() const;

	UPROPERTY()
	AActor* CurrentHackTarget;

	UPROPERTY()
	TArray<AActor*> ActiveHacks;

	float HackProgress;
	bool bIsHacking;
};