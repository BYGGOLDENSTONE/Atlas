#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallImpactComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWallImpact, AActor*, ImpactedActor, const FHitResult&, WallHit, float, ImpactForce);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFloorImpact, AActor*, ImpactedActor, const FHitResult&, FloorHit);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UWallImpactComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWallImpactComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Impact")
    float CollisionSphereRadius = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Impact")
    float MinImpactForce = 150.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Impact")
    float WallImpactStaggerDuration = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Impact")
    float FloorRagdollDuration = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Impact")
    float WallSurfaceAngleThreshold = 0.3f;  // Normal.Z < 0.3 = Wall
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Impact")
    float FloorSurfaceAngleThreshold = 0.7f;  // Normal.Z > 0.7 = Floor
    
    UPROPERTY(BlueprintAssignable, Category = "Wall Impact Events")
    FOnWallImpact OnWallImpact;
    
    UPROPERTY(BlueprintAssignable, Category = "Wall Impact Events")
    FOnFloorImpact OnFloorImpact;
    
    UFUNCTION(BlueprintCallable, Category = "Wall Impact")
    void StartKnockbackTracking(AActor* Target, float KnockbackForce);
    
    UFUNCTION(BlueprintCallable, Category = "Wall Impact")
    void StopKnockbackTracking();
    
    UFUNCTION(BlueprintCallable, Category = "Wall Impact")
    void ApplyWallImpactEffects(AActor* Target, const FHitResult& WallHit);
    
    UFUNCTION(BlueprintCallable, Category = "Wall Impact")
    void ApplyFloorImpactEffects(AActor* Target, const FHitResult& FloorHit);
    
protected:
    UFUNCTION()
    void OnTargetHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
    
    bool IsWallHit(const FHitResult& Hit) const;
    bool IsFloorHit(const FHitResult& Hit) const;
    
private:
    UPROPERTY()
    class USphereComponent* CollisionSphere;
    
    UPROPERTY()
    AActor* TrackedTarget;
    
    float CurrentKnockbackForce;
    bool bIsTracking;
    FTimerHandle TrackingTimeoutHandle;
};