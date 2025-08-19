#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "GameplayTagContainer.h"
#include "ValveInteractable.generated.h"

UENUM(BlueprintType)
enum class EValveType : uint8
{
    Vulnerability UMETA(DisplayName = "Vulnerability"),
    Stagger UMETA(DisplayName = "Stagger")
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
    EValveType ValveType = EValveType::Vulnerability;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    float AoERadius = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    float StaggerPoiseDamage = 30.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    int32 VulnerabilityCharges = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Valve")
    TSubclassOf<AActor> AoEEffectClass;
    
    UFUNCTION(BlueprintCallable, Category = "Valve")
    void TriggerAoEEffect(AActor* Interactor);
    
    UFUNCTION(BlueprintCallable, Category = "Valve")
    void ApplyEffectToActor(AActor* Target);
    
    UFUNCTION(BlueprintCallable, Category = "Valve")
    TArray<AActor*> GetActorsInRadius() const;
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Valve")
    void OnAoETriggered(const TArray<AActor*>& AffectedActors);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Valve")
    void PlayValveAnimation();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Valve")
    void SpawnVisualEffect(EValveType Type, float Radius);

private:
    TWeakObjectPtr<AActor> LastInteractor;
};