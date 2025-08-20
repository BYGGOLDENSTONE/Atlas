#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "StationIntegrityComponent.generated.h"

class UStationIntegrityDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnIntegrityChanged, float, CurrentIntegrity, float, MaxIntegrity, float, IntegrityDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIntegrityDamaged, float, DamageAmount, AActor*, DamageInstigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntegrityThresholdReached, float, ThresholdPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIntegrityCritical);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIntegrityFailed);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UStationIntegrityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStationIntegrityComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station Integrity")
    UStationIntegrityDataAsset* IntegrityDataAsset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station Integrity", meta = (ClampMin = "1.0"))
    float MaxIntegrity = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Station Integrity")
    float CurrentIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station Integrity")
    float CriticalThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station Integrity")
    float FailureThreshold = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Station Integrity")
    bool bIsIntegrityCritical = false;

    UPROPERTY(BlueprintReadOnly, Category = "Station Integrity")
    bool bIsIntegrityFailed = false;

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    void ApplyIntegrityDamage(float DamageAmount, AActor* DamageInstigator = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    void RestoreIntegrity(float RestoreAmount);

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    void SetIntegrity(float NewIntegrity);

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    void SetIntegrityPercent(float Percent);

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    float GetIntegrityPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    float GetCurrentIntegrity() const { return CurrentIntegrity; }

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    float GetMaxIntegrity() const { return MaxIntegrity; }

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    bool IsIntegrityCritical() const { return bIsIntegrityCritical; }

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    bool IsIntegrityFailed() const { return bIsIntegrityFailed; }

    UFUNCTION(BlueprintCallable, Category = "Station Integrity")
    void ApplyAbilityIntegrityCost(const FGameplayTag& AbilityTag, AActor* Instigator = nullptr);

    UPROPERTY(BlueprintAssignable, Category = "Station Integrity Events")
    FOnIntegrityChanged OnIntegrityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Station Integrity Events")
    FOnIntegrityDamaged OnIntegrityDamaged;

    UPROPERTY(BlueprintAssignable, Category = "Station Integrity Events")
    FOnIntegrityThresholdReached OnIntegrityThresholdReached;

    UPROPERTY(BlueprintAssignable, Category = "Station Integrity Events")
    FOnIntegrityCritical OnIntegrityCritical;

    UPROPERTY(BlueprintAssignable, Category = "Station Integrity Events")
    FOnIntegrityFailed OnIntegrityFailed;

private:
    void CheckIntegrityThresholds();
    void HandleIntegrityFailure();
    void BroadcastIntegrityChange(float IntegrityDelta);
};