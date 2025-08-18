#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "VentInteractable.generated.h"

class AProjectile;

UCLASS()
class ATLAS_API AVentInteractable : public AInteractableBase
{
    GENERATED_BODY()

public:
    AVentInteractable();

protected:
    virtual void BeginPlay() override;
    virtual void ExecuteInteraction(AActor* Interactor) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent")
    float LaunchSpeed = 1500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent")
    float LaunchRange = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent Physics")
    float Mass = 50.0f;
    
    // Note: GravityScale not directly supported in UE5, use Mass to affect physics behavior
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent Physics")
    bool bShouldBounce = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent Physics")
    float Bounciness = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent")
    float StaggerPoiseDamage = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent Launch")
    FVector LaunchDirection = FVector(1.0f, 0.0f, 0.5f);
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent Launch")
    bool bUseLocalDirection = true;
    
    UPROPERTY(BlueprintReadOnly, Category = "Vent State")
    bool bHasBeenTriggered = false;
    
    UFUNCTION(BlueprintCallable, Category = "Vent")
    void LaunchSelf(AActor* Interactor);
    
    UFUNCTION(BlueprintCallable, Category = "Vent")
    FVector GetPredeterminedLaunchDirection() const;
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Vent")
    void OnVentLaunched(const FVector& LaunchVelocity);
    
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Vent")
    void PlayVentAnimation();

private:
    void ApplyStaggerToTarget(AActor* HitActor);
    void SetupPhysics();
    FTimerHandle LaunchTimerHandle;
    bool bIsFlying = false;
};