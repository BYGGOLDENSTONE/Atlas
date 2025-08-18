#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "GameplayTagContainer.h"
#include "ValveInteractable.generated.h"

UENUM(BlueprintType)
enum class EValveEffectType : uint8
{
    Fire UMETA(DisplayName = "Fire"),
    Electric UMETA(DisplayName = "Electric"),
    Poison UMETA(DisplayName = "Poison"),
    Physical UMETA(DisplayName = "Physical")
};

USTRUCT(BlueprintType)
struct FValveArchetypeEffect
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag ArchetypeTag;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RadiusMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAppliesVulnerability = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAppliesStagger = false;
};

UCLASS()
class ATLAS_API AValveInteractable : public AInteractableBase
{
    GENERATED_BODY()

public:
    AValveInteractable();

protected:
    virtual void BeginPlay() override;
    virtual void ExecuteInteraction(AActor* Interactor) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    EValveEffectType EffectType = EValveEffectType::Fire;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    float BaseDamage = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    float AoERadius = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    float EffectDuration = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    bool bDamageOverTime = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    float TickDamageInterval = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    TArray<FValveArchetypeEffect> ArchetypeEffects;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    bool bAffectsAllies = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    bool bAffectsEnemies = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    TSubclassOf<AActor> AoEEffectClass;
    
    UFUNCTION(BlueprintCallable, Category = "Valve")
    void TriggerAoEEffect(AActor* Interactor);
    
    UFUNCTION(BlueprintCallable, Category = "Valve")
    void ApplyEffectToActor(AActor* Target, AActor* Interactor);
    
    UFUNCTION(BlueprintCallable, Category = "Valve")
    TArray<AActor*> GetActorsInRadius() const;
    
    UFUNCTION(BlueprintCallable, Category = "Valve")
    FGameplayTag GetDamageTypeTag() const;
    
    UFUNCTION(BlueprintCallable, Category = "Valve")
    FValveArchetypeEffect GetArchetypeEffect(AActor* Target) const;
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Valve")
    void OnAoETriggered(const TArray<AActor*>& AffectedActors);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Valve")
    void PlayValveAnimation();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Valve")
    void SpawnVisualEffect(EValveEffectType Type, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Valve")
    void ApplyVisualEffectToActor(AActor* Target, AActor* Interactor);

private:
    void StartDamageOverTime();
    void StopDamageOverTime();
    void ApplyTickDamage();
    void StartEffectOverTime();
    void StopEffectOverTime();
    void ApplyTickEffect();
    
    FTimerHandle DamageOverTimeHandle;
    TArray<TWeakObjectPtr<AActor>> ActiveTargets;
    TWeakObjectPtr<AActor> LastInteractor;
};