// DestructibleEnvironmentComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DestructibleEnvironmentComponent.generated.h"

UENUM(BlueprintType)
enum class EDestructibleState : uint8
{
    Intact       UMETA(DisplayName = "Intact"),
    Damaged      UMETA(DisplayName = "Damaged"),
    Critical     UMETA(DisplayName = "Critical"),
    Destroyed    UMETA(DisplayName = "Destroyed")
};

USTRUCT(BlueprintType)
struct FDestructionStage
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HealthThreshold = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* StageMesh = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystem* TransitionEffect = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* TransitionSound = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnablePhysics = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCreateDebris = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DebrisCount = 5;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UDestructibleEnvironmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDestructibleEnvironmentComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Damage handling
    UFUNCTION(BlueprintCallable, Category = "Destructible")
    void ApplyDamage(float DamageAmount, AActor* DamageInstigator);
    
    UFUNCTION(BlueprintCallable, Category = "Destructible")
    void Repair(float RepairAmount);
    
    UFUNCTION(BlueprintCallable, Category = "Destructible")
    void SetHealth(float NewHealth);
    
    // State queries
    UFUNCTION(BlueprintPure, Category = "Destructible")
    float GetCurrentHealth() const { return CurrentHealth; }
    
    UFUNCTION(BlueprintPure, Category = "Destructible")
    float GetMaxHealth() const { return MaxHealth; }
    
    UFUNCTION(BlueprintPure, Category = "Destructible")
    float GetHealthPercent() const;
    
    UFUNCTION(BlueprintPure, Category = "Destructible")
    EDestructibleState GetDestructionState() const { return CurrentState; }
    
    UFUNCTION(BlueprintPure, Category = "Destructible")
    bool IsDestroyed() const { return CurrentState == EDestructibleState::Destroyed; }
    
    // Destruction control
    UFUNCTION(BlueprintCallable, Category = "Destructible")
    void ForceDestroy();
    
    UFUNCTION(BlueprintCallable, Category = "Destructible")
    void ResetToIntact();

protected:
    // State management
    void UpdateDestructionState();
    void TransitionToState(EDestructibleState NewState);
    void ApplyStateEffects(EDestructibleState State);
    
    // Visual updates
    void UpdateVisuals();
    void SpawnDebris();
    void CreateExplosionEffect();
    
    // Damage effects
    void PlayDamageEffect(float DamageAmount);
    void CreateDamageDecal(const FVector& Location);
    
    // Physics
    void EnablePhysicsSimulation();
    void DisablePhysicsSimulation();
    
    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Destructible")
    void OnDamaged(float DamageAmount, AActor* DamageInstigator);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Destructible")
    void OnStateChanged(EDestructibleState OldState, EDestructibleState NewState);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Destructible")
    void OnDestroyed();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Destructible")
    void OnRepaired(float RepairAmount);

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    float MaxHealth = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    bool bCanBeRepaired = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    bool bDestroyActorWhenDestroyed = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    float DestroyDelay = 2.0f;
    
    // Destruction stages
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction Stages")
    TArray<FDestructionStage> DestructionStages;
    
    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    UParticleSystem* DamageEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    UParticleSystem* DestroyEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    USoundBase* DamageSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    USoundBase* DestroySound;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TSubclassOf<UCameraShakeBase> DamageCameraShake;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TSubclassOf<UCameraShakeBase> DestroyCameraShake;
    
    // Debris
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debris")
    TArray<UStaticMesh*> DebrisMeshes;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debris")
    float DebrisLifespan = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debris")
    float DebrisImpulseStrength = 500.0f;
    
    // Damage multipliers
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    TMap<FGameplayTag, float> DamageMultipliers;
    
    // Station integrity impact
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station")
    bool bDamagesStationIntegrity = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station")
    float IntegrityDamageOnDestruction = 5.0f;

private:
    // Runtime state
    UPROPERTY()
    float CurrentHealth;
    
    UPROPERTY()
    EDestructibleState CurrentState;
    
    UPROPERTY()
    int32 CurrentStageIndex;
    
    UPROPERTY()
    class UStaticMeshComponent* MeshComponent;
    
    UPROPERTY()
    TArray<class UParticleSystemComponent*> ActiveEffects;
    
    UPROPERTY()
    TArray<AActor*> SpawnedDebris;
    
    FTimerHandle DestroyTimerHandle;
    
    // Helper functions
    UStaticMeshComponent* GetMeshComponent();
    void CleanupDebris();
    void CleanupEffects();
};