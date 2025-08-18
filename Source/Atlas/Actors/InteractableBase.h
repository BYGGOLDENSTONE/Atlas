#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/IInteractable.h"
#include "GameplayTagContainer.h"
#include "InteractableBase.generated.h"

class UStaticMeshComponent;
class UParticleSystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteracted, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCooldownStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCooldownEnded);

UCLASS(Abstract)
class ATLAS_API AInteractableBase : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AInteractableBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* FocusedParticleEffect;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interactable")
    FGameplayTag InteractableTypeTag;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable")
    float CooldownDuration = 5.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable")
    float MaxInteractionDistance = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactable")
    bool bStartOnCooldown = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Interactable State")
    bool bIsOnCooldown = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Interactable State")
    bool bIsFocused = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Interactable State")
    float CooldownTimeRemaining = 0.0f;
    
    UPROPERTY(BlueprintReadOnly, Category = "Interactable State")
    FGameplayTagContainer StateTags;
    
    virtual FGameplayTag GetInteractableType_Implementation() const override;
    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void OnFocused_Implementation(AActor* FocusingActor) override;
    virtual void OnUnfocused_Implementation(AActor* FocusingActor) override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual float GetInteractionDistance_Implementation() const override;
    virtual FVector GetFocusLocation_Implementation() const override;
    virtual bool IsOnCooldown_Implementation() const override;
    virtual float GetCooldownRemaining_Implementation() const override;
    
    UFUNCTION(BlueprintCallable, Category = "Interactable")
    void StartCooldown();
    
    UFUNCTION(BlueprintCallable, Category = "Interactable")
    void EndCooldown();
    
    UFUNCTION(BlueprintCallable, Category = "Interactable")
    void UpdateVisualState();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void OnInteractionSuccessful(AActor* Interactor);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void OnInteractionFailed(AActor* Interactor);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void UpdateFocusVisuals(bool bNewFocusState);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void UpdateCooldownVisuals(float TimeRemaining, float TotalDuration);
    
    UPROPERTY(BlueprintAssignable, Category = "Interactable Events")
    FOnInteracted OnInteracted;
    
    UPROPERTY(BlueprintAssignable, Category = "Interactable Events")
    FOnCooldownStarted OnCooldownStarted;
    
    UPROPERTY(BlueprintAssignable, Category = "Interactable Events")
    FOnCooldownEnded OnCooldownEnded;

protected:
    virtual void ExecuteInteraction(AActor* Interactor);
    
private:
    FTimerHandle CooldownTimerHandle;
};