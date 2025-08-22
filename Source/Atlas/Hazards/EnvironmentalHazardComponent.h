// EnvironmentalHazardComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Engine/DataAsset.h"
#include "EnvironmentalHazardComponent.generated.h"

class UDamageType;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EHazardType : uint8
{
    None           UMETA(DisplayName = "None"),
    Electrical     UMETA(DisplayName = "Electrical"),
    Toxic          UMETA(DisplayName = "Toxic"),
    Fire           UMETA(DisplayName = "Fire"),
    Gravity        UMETA(DisplayName = "Gravity"),
    Radiation      UMETA(DisplayName = "Radiation")
};

USTRUCT(BlueprintType)
struct FHazardEffectData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystem* HazardEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* HazardSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* HazardDecal;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor HazardColor;
    
    FHazardEffectData()
    {
        HazardEffect = nullptr;
        HazardSound = nullptr;
        HazardDecal = nullptr;
        HazardColor = FLinearColor::White;
    }
};

UCLASS(Abstract, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UEnvironmentalHazardComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UEnvironmentalHazardComponent();

    // Hazard configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
    EHazardType HazardType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
    float HazardRadius = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
    float DamagePerSecond = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
    float ActivationDelay = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
    bool bPermanent = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard", meta = (EditCondition = "!bPermanent"))
    float Duration = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
    bool bStartActive = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
    bool bAffectsPlayers = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
    bool bAffectsEnemies = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
    bool bDamagesStationIntegrity = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard", meta = (EditCondition = "bDamagesStationIntegrity"))
    float IntegrityDamagePerSecond = 1.0f;
    
    // Visual/Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FHazardEffectData EffectData;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSubclassOf<UDamageType> DamageTypeClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bShowWarningIndicator = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float WarningIndicatorTime = 1.0f;
    
    // Activation
    UFUNCTION(BlueprintCallable, Category = "Hazard")
    virtual void ActivateHazard();
    
    UFUNCTION(BlueprintCallable, Category = "Hazard")
    virtual void DeactivateHazard();
    
    UFUNCTION(BlueprintCallable, Category = "Hazard")
    void ToggleHazard();
    
    UFUNCTION(BlueprintPure, Category = "Hazard")
    bool IsHazardActive() const { return bIsActive; }
    
    UFUNCTION(BlueprintPure, Category = "Hazard")
    float GetRemainingDuration() const;
    
    UFUNCTION(BlueprintPure, Category = "Hazard")
    TArray<AActor*> GetAffectedActors() const { return AffectedActors; }
    
    // Events
    UFUNCTION(BlueprintNativeEvent, Category = "Hazard")
    void OnActorEnterHazard(AActor* Actor);
    
    UFUNCTION(BlueprintNativeEvent, Category = "Hazard")
    void OnActorExitHazard(AActor* Actor);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Hazard")
    void OnHazardActivated();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Hazard")
    void OnHazardDeactivated();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Hazard")
    void OnActorDamagedByHazard(AActor* Actor, float Damage);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    // Override in derived classes
    virtual void ApplyHazardEffect(AActor* Actor, float DeltaTime);
    virtual void UpdateHazardVisuals(float DeltaTime);
    virtual bool ShouldAffectActor(AActor* Actor) const;
    
    // Helper functions
    void ShowWarningIndicator();
    void HideWarningIndicator();
    void SpawnHazardEffects();
    void DestroyHazardEffects();
    void UpdateActiveActors();
    void DamageStationIntegrity(float DeltaTime);
    void StartHazard();
    
protected:
    UPROPERTY(BlueprintReadOnly, Category = "Hazard")
    TArray<AActor*> AffectedActors;
    
    UPROPERTY()
    class UParticleSystemComponent* ActiveHazardEffect;
    
    UPROPERTY()
    class UAudioComponent* ActiveHazardSound;
    
    UPROPERTY()
    class UDecalComponent* ActiveHazardDecal;
    
    UPROPERTY()
    class USphereComponent* HazardTriggerSphere;
    
    bool bIsActive;
    float CurrentDuration;
    float ActivationTimer;
    bool bWarningShown;
    
private:
    FTimerHandle DeactivationTimerHandle;
    FTimerHandle WarningTimerHandle;
    
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    
    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};