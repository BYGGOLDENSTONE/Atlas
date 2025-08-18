#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallImpactComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWallImpact, AActor*, ImpactedActor, const FHitResult&, WallHit, float, ImpactForce);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UWallImpactComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWallImpactComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Impact")
    float WallDetectionDistance = 200.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Impact")
    float MinImpactForce = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Impact")
    float WallImpactDamageMultiplier = 1.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Impact")
    float WallImpactStaggerDuration = 2.0f;
    
    UPROPERTY(BlueprintAssignable, Category = "Wall Impact Events")
    FOnWallImpact OnWallImpact;
    
    UFUNCTION(BlueprintCallable, Category = "Wall Impact")
    bool CheckForWallImpact(const FVector& KnockbackDirection, float KnockbackForce, FHitResult& OutWallHit);
    
    UFUNCTION(BlueprintCallable, Category = "Wall Impact")
    void ApplyWallImpactEffects(AActor* Target, const FHitResult& WallHit, float ImpactForce);
    
    UFUNCTION(BlueprintCallable, Category = "Wall Impact")
    float CalculateWallImpactDamage(float BaseDamage);
    
    UFUNCTION(BlueprintCallable, Category = "Wall Impact")
    void ApplyExtendedStagger(AActor* Target);
};