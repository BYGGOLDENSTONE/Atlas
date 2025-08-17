#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameplayTagContainer.h"
#include "HitboxComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitboxActivated, const FGameplayTag&, AttackTag, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitboxDeactivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitDetected, AActor*, HitActor, const FHitResult&, HitResult);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent, DisplayName="Hitbox Component"))
class ATLAS_API UHitboxComponent : public USphereComponent
{
    GENERATED_BODY()

public:
    UHitboxComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FGameplayTag AttackTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    bool bDebugDraw = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FColor DebugColor = FColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    float DebugDrawDuration = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Hitbox")
    bool bHitboxActive = false;

    UFUNCTION(BlueprintCallable, Category = "Hitbox")
    void ActivateHitbox(const FGameplayTag& InAttackTag, float Duration = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Hitbox")
    void DeactivateHitbox();

    UFUNCTION(BlueprintCallable, Category = "Hitbox")
    void ResetHitActors();

    UFUNCTION(BlueprintCallable, Category = "Hitbox")
    bool HasAlreadyHit(AActor* Actor) const;

    UPROPERTY(BlueprintAssignable, Category = "Hitbox Events")
    FOnHitboxActivated OnHitboxActivated;

    UPROPERTY(BlueprintAssignable, Category = "Hitbox Events")
    FOnHitboxDeactivated OnHitboxDeactivated;

    UPROPERTY(BlueprintAssignable, Category = "Hitbox Events")
    FOnHitDetected OnHitDetected;

private:
    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UPROPERTY()
    TArray<AActor*> AlreadyHitActors;
    
    FTimerHandle DeactivationTimer;

    void ProcessHit(AActor* HitActor, const FHitResult& HitResult);
    void DrawDebugHitbox();
};