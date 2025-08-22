// ElectricalSurgeHazard.h
#pragma once

#include "CoreMinimal.h"
#include "EnvironmentalHazardComponent.h"
#include "ElectricalSurgeHazard.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UElectricalSurgeHazard : public UEnvironmentalHazardComponent
{
    GENERATED_BODY()

public:
    UElectricalSurgeHazard();

    // Electrical specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical")
    float StunDuration = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical")
    float ChainRadius = 200.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical")
    int32 MaxChainTargets = 3;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical")
    float ChainDamageMultiplier = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical")
    float ElectricityJumpDelay = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Effects")
    UParticleSystem* ChainLightningEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Effects")
    USoundCue* ElectricityJumpSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Electrical|Effects")
    TSubclassOf<UCameraShakeBase> ElectricalShake;

protected:
    virtual void ApplyHazardEffect(AActor* Actor, float DeltaTime) override;
    virtual void UpdateHazardVisuals(float DeltaTime) override;
    
    UFUNCTION(BlueprintCallable, Category = "Electrical")
    void ChainElectricity(AActor* Source);
    
    UFUNCTION(BlueprintCallable, Category = "Electrical")
    void SpawnElectricalArc(const FVector& Start, const FVector& End);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Electrical")
    void OnElectricityChained(AActor* Source, AActor* Target, float Damage);

private:
    TArray<AActor*> ChainedActors;
    float LastChainTime;
};