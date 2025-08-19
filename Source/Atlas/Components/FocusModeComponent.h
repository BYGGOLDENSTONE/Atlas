#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FocusModeComponent.generated.h"

class IInteractable;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetFocused, AActor*, FocusedTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargetUnfocused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusModeChanged, bool, bIsActive);

USTRUCT(BlueprintType)
struct FFocusTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    AActor* Actor = nullptr;
    
    UPROPERTY(BlueprintReadOnly)
    float ScreenDistance = 0.0f;
    
    UPROPERTY(BlueprintReadOnly)
    float WorldDistance = 0.0f;
    
    UPROPERTY(BlueprintReadOnly)
    bool bIsInteractable = false;
    
    UPROPERTY(BlueprintReadOnly)
    bool bIsEnemy = false;
    
    UPROPERTY(BlueprintReadOnly)
    float Priority = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UFocusModeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFocusModeComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Mode")
    float FocusRange = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Mode")
    float ScreenCenterTolerance = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Mode")
    float InteractablePriorityBonus = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Mode")
    bool bDebugDrawFocusInfo = true;
    
    UPROPERTY(BlueprintReadOnly, Category = "Focus Mode State")
    bool bIsFocusModeActive = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Focus Mode State")
    AActor* CurrentFocusedTarget = nullptr;
    
    UPROPERTY(BlueprintReadOnly, Category = "Focus Mode State")
    FFocusTarget CurrentTargetInfo;
    
    UPROPERTY(BlueprintReadOnly, Category = "Focus Mode State")
    TArray<FFocusTarget> PotentialTargets;
    
    UFUNCTION(BlueprintCallable, Category = "Focus Mode")
    void StartFocusMode();
    
    UFUNCTION(BlueprintCallable, Category = "Focus Mode")
    void StopFocusMode();
    
    UFUNCTION(BlueprintCallable, Category = "Focus Mode")
    void ToggleFocusMode();
    
    UFUNCTION(BlueprintCallable, Category = "Focus Mode")
    bool TryInteractWithFocusedTarget();
    
    UFUNCTION(BlueprintCallable, Category = "Focus Mode")
    AActor* GetFocusedTarget() const { return CurrentFocusedTarget; }
    
    UFUNCTION(BlueprintCallable, Category = "Focus Mode")
    bool IsFocusModeActive() const { return bIsFocusModeActive; }
    
    UFUNCTION(BlueprintCallable, Category = "Focus Mode")
    bool IsTargetInteractable(AActor* Target) const;
    
    UPROPERTY(BlueprintAssignable, Category = "Focus Mode Events")
    FOnTargetFocused OnTargetFocused;
    
    UPROPERTY(BlueprintAssignable, Category = "Focus Mode Events")
    FOnTargetUnfocused OnTargetUnfocused;
    
    UPROPERTY(BlueprintAssignable, Category = "Focus Mode Events")
    FOnFocusModeChanged OnFocusModeChanged;

private:
    void ScanForTargets();
    void UpdateFocusTarget();
    void SetFocusedTarget(AActor* NewTarget);
    void ClearFocusedTarget();
    FFocusTarget EvaluateTarget(AActor* Target) const;
    FVector2D GetScreenPosition(const FVector& WorldPosition) const;
    float CalculateScreenDistance(const FVector2D& ScreenPos) const;
    float CalculatePriority(const FFocusTarget& Target) const;
    void DrawDebugInfo() const;
    
    UPROPERTY()
    UCameraComponent* CachedCamera = nullptr;
    
    UPROPERTY()
    APlayerController* CachedPlayerController = nullptr;
    
    float TimeSinceFocusStart = 0.0f;
    bool bWasInteracting = false;
};