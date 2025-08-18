#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoftLockComponent.generated.h"

class UCameraComponent;
class UCombatComponent;
class AEnemyCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoftLockEngaged, AActor*, LockedTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSoftLockDisengaged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API USoftLockComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USoftLockComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soft Lock")
    float DetectionRange = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soft Lock")
    float BreakAwayAngle = 90.0f; // Degrees player must turn to break lock (increased for less aggressive breaking)
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soft Lock")
    float ReengageAngle = 30.0f; // Degrees within which lock can reengage (decreased to avoid unwanted locks)
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soft Lock")
    float CameraInterpSpeed = 1.5f; // How smoothly camera tracks (decreased for gentler movement)
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soft Lock")
    float MinLockDistance = 200.0f; // Too close, no lock needed
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soft Lock")
    bool bEnabled = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Soft Lock")
    bool bDebugDraw = false;
    
    // State
    UPROPERTY(BlueprintReadOnly, Category = "Soft Lock State")
    bool bIsLocked = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Soft Lock State")
    AActor* CurrentTarget = nullptr;
    
    UPROPERTY(BlueprintReadOnly, Category = "Soft Lock State")
    float CurrentTargetDistance = 0.0f;
    
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Soft Lock Events")
    FOnSoftLockEngaged OnSoftLockEngaged;
    
    UPROPERTY(BlueprintAssignable, Category = "Soft Lock Events")
    FOnSoftLockDisengaged OnSoftLockDisengaged;
    
    // Public methods
    UFUNCTION(BlueprintCallable, Category = "Soft Lock")
    void EnableSoftLock() { bEnabled = true; }
    
    UFUNCTION(BlueprintCallable, Category = "Soft Lock")
    void DisableSoftLock() { bEnabled = false; DisengageLock(); }
    
    UFUNCTION(BlueprintCallable, Category = "Soft Lock")
    void ForceBreakLock() { DisengageLock(); }
    
    UFUNCTION(BlueprintCallable, Category = "Soft Lock")
    bool IsLocked() const { return bIsLocked; }
    
    UFUNCTION(BlueprintCallable, Category = "Soft Lock")
    AActor* GetLockedTarget() const { return CurrentTarget; }

private:
    void ScanForEnemies();
    AActor* FindBestTarget();
    void EngageLock(AActor* Target);
    void DisengageLock();
    void UpdateCameraRotation();
    bool ShouldBreakLock();
    bool IsTargetValid(AActor* Target) const;
    float GetAngleToTarget(AActor* Target) const;
    void DrawDebugInfo() const;
    
    UPROPERTY()
    UCameraComponent* CachedCamera = nullptr;
    
    UPROPERTY()
    APlayerController* CachedPlayerController = nullptr;
    
    UPROPERTY()
    UCombatComponent* CachedCombatComponent = nullptr;
    
    FRotator LastPlayerInputRotation;
    bool bPlayerIsRotatingCamera = false;
    float TimeSinceLockEngaged = 0.0f;
    float LastDisengageTime = 0.0f;
};