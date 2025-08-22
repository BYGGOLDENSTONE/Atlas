// ToxicLeakHazard.h
#pragma once

#include "CoreMinimal.h"
#include "EnvironmentalHazardComponent.h"
#include "ToxicLeakHazard.generated.h"

USTRUCT(BlueprintType)
struct FPoisonDOTData
{
    GENERATED_BODY()
    
    UPROPERTY()
    AActor* Target = nullptr;
    
    UPROPERTY()
    float Duration = 5.0f;
    
    UPROPERTY()
    float DamagePerSecond = 5.0f;
    
    UPROPERTY()
    float TimeRemaining = 0.0f;
    
    FPoisonDOTData()
    {
        Target = nullptr;
        Duration = 5.0f;
        DamagePerSecond = 5.0f;
        TimeRemaining = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UToxicLeakHazard : public UEnvironmentalHazardComponent
{
    GENERATED_BODY()

public:
    UToxicLeakHazard();

    // Toxic specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxic")
    float VisionImpairment = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxic")
    float MovementSlowPercent = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxic")
    float DOTDuration = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxic")
    float DOTDamagePerSecond = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxic")
    float ToxicCloudSpreadRadius = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxic")
    float ToxicCloudSpreadRate = 50.0f; // Units per second
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxic|Effects")
    UParticleSystem* ToxicCloudEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxic|Effects")
    USoundCue* CoughingSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toxic|Effects")
    UMaterialInterface* ToxicScreenOverlay;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void ApplyHazardEffect(AActor* Actor, float DeltaTime) override;
    virtual void UpdateHazardVisuals(float DeltaTime) override;
    virtual void OnActorEnterHazard_Implementation(AActor* Actor) override;
    virtual void OnActorExitHazard_Implementation(AActor* Actor) override;
    
    UFUNCTION(BlueprintCallable, Category = "Toxic")
    void ApplyPoisonDOT(AActor* Target);
    
    UFUNCTION(BlueprintCallable, Category = "Toxic")
    void ApplyToxicScreenEffect(APlayerController* PC, float Intensity);
    
    UFUNCTION(BlueprintCallable, Category = "Toxic")
    void RemoveToxicScreenEffect(APlayerController* PC);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Toxic")
    void OnPoisonApplied(AActor* Target, float PoisonDuration, float PoisonDamage);

private:
    void UpdatePoisonDOTs(float DeltaTime);
    void ExpandToxicCloud(float DeltaTime);
    
    TArray<FPoisonDOTData> ActiveDOTs;
    TMap<AActor*, float> OriginalMovementSpeeds;
    float CurrentCloudRadius;
    
    UPROPERTY()
    TArray<UParticleSystemComponent*> ToxicCloudParticles;
};