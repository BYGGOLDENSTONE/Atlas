// LowGravityHazard.h
#pragma once

#include "CoreMinimal.h"
#include "EnvironmentalHazardComponent.h"
#include "LowGravityHazard.generated.h"

USTRUCT(BlueprintType)
struct FOriginalMovementValues
{
    GENERATED_BODY()
    
    UPROPERTY()
    float GravityScale = 1.0f;
    
    UPROPERTY()
    float JumpZVelocity = 420.0f;
    
    UPROPERTY()
    float AirControl = 0.05f;
    
    UPROPERTY()
    float FallingLateralFriction = 0.0f;
    
    FOriginalMovementValues()
    {
        GravityScale = 1.0f;
        JumpZVelocity = 420.0f;
        AirControl = 0.05f;
        FallingLateralFriction = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API ULowGravityHazard : public UEnvironmentalHazardComponent
{
    GENERATED_BODY()

public:
    ULowGravityHazard();

    // Gravity specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float GravityScale = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity", meta = (ClampMin = "1.0", ClampMax = "5.0"))
    float JumpBoostMultiplier = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AirControlBoost = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    float FloatingForce = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    bool bAffectsPhysicsObjects = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    float PhysicsObjectGravityScale = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity|Effects")
    UParticleSystem* AntiGravityParticles;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity|Effects")
    UParticleSystem* FloatingDebrisEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity|Effects")
    USoundCue* AntiGravityHumSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity|Effects")
    UMaterialInterface* GravityDistortionMaterial;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void ApplyHazardEffect(AActor* Actor, float DeltaTime) override;
    virtual void UpdateHazardVisuals(float DeltaTime) override;
    virtual void OnActorEnterHazard_Implementation(AActor* Actor) override;
    virtual void OnActorExitHazard_Implementation(AActor* Actor) override;
    
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void ApplyAntiGravityToPhysicsObject(UPrimitiveComponent* Component);
    
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void RestoreGravityToPhysicsObject(UPrimitiveComponent* Component);
    
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void SpawnAntiGravityParticles(AActor* Target);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Gravity")
    void OnActorEnteredLowGravity(AActor* Actor);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Gravity")
    void OnActorExitedLowGravity(AActor* Actor);

private:
    void UpdateFloatingObjects(float DeltaTime);
    void CreateFloatingDebris();
    
    TMap<class AGameCharacterBase*, FOriginalMovementValues> OriginalCharacterValues;
    TMap<UPrimitiveComponent*, float> OriginalPhysicsGravity;
    TArray<UPrimitiveComponent*> FloatingObjects;
    
    UPROPERTY()
    TArray<UParticleSystemComponent*> ActiveAntiGravityEffects;
    
    UPROPERTY()
    UAudioComponent* AmbientHumSound;
    
    float DebrisSpawnTimer;
};