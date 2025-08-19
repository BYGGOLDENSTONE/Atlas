#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DebugRenderComponent.generated.h"

UENUM(BlueprintType)
enum class EDebugCategory : uint8
{
    None = 0,
    Combat = 1 << 0,
    Movement = 1 << 1,
    Interactions = 1 << 2,
    AI = 1 << 3,
    Physics = 1 << 4,
    All = 255
};
ENUM_CLASS_FLAGS(EDebugCategory);

USTRUCT(BlueprintType)
struct FDebugDrawRequest
{
    GENERATED_BODY()

    UPROPERTY()
    EDebugCategory Category = EDebugCategory::None;

    UPROPERTY()
    float Duration = 0.0f;

    UPROPERTY()
    FVector Location = FVector::ZeroVector;

    UPROPERTY()
    FVector EndLocation = FVector::ZeroVector;

    UPROPERTY()
    float Radius = 0.0f;

    UPROPERTY()
    FColor Color = FColor::White;

    UPROPERTY()
    FString Text = "";

    UPROPERTY()
    int32 Segments = 12;

    UPROPERTY()
    float Thickness = 1.0f;

    enum EDrawType
    {
        Sphere,
        Line,
        Box,
        Arrow,
        Circle,
        String
    } DrawType = Sphere;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UDebugRenderComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDebugRenderComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (Bitmask, BitmaskEnum = "EDebugCategory"))
    uint8 EnabledCategories = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bGlobalDebugEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DefaultDrawDuration = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void SetCategoryEnabled(EDebugCategory Category, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    bool IsCategoryEnabled(EDebugCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleCategory(EDebugCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void EnableAllCategories();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DisableAllCategories();

    UFUNCTION(BlueprintCallable, Category = "Debug", meta = (CallInEditor = "true"))
    void ToggleGlobalDebug();

    void DrawDebugSphereAtLocation(const FVector& Location, float Radius, EDebugCategory Category, 
        const FColor& Color = FColor::White, float Duration = -1.0f, int32 Segments = 12);

    void DrawDebugLineFromTo(const FVector& Start, const FVector& End, EDebugCategory Category,
        const FColor& Color = FColor::White, float Duration = -1.0f, float Thickness = 1.0f);

    void DrawDebugBoxAtLocation(const FVector& Location, const FVector& Extent, EDebugCategory Category,
        const FColor& Color = FColor::White, float Duration = -1.0f);

    void DrawDebugArrowFromTo(const FVector& Start, const FVector& End, EDebugCategory Category,
        const FColor& Color = FColor::White, float Duration = -1.0f, float ArrowSize = 10.0f, float Thickness = 1.0f);

    void DrawDebugCircleAtLocation(const FVector& Location, float Radius, int32 Segments, EDebugCategory Category,
        const FColor& Color = FColor::White, float Duration = -1.0f, float Thickness = 1.0f,
        const FVector& YAxis = FVector(0, 1, 0), const FVector& ZAxis = FVector(0, 0, 1));

    void DrawDebugTextAtLocation(const FVector& Location, const FString& Text, EDebugCategory Category,
        const FColor& Color = FColor::White, float Duration = -1.0f);

    void DrawCombatHitbox(const FVector& Location, float Radius, bool bIsActiveHitbox = false);
    
    void DrawKnockbackArrow(const FVector& Start, const FVector& Direction, float Force);
    
    void DrawStaggerIndicator(const FVector& Location, float Duration);
    
    void DrawWallImpact(const FVector& ImpactLocation, const FVector& ImpactNormal);
    
    void DrawFocusTarget(AActor* Target, bool bIsPrimary = false);
    
    void DrawInteractableHighlight(AActor* Interactable, const FColor& Color = FColor::Cyan);
    
    void DrawDashTrajectory(const FVector& Start, const FVector& End);

    static UDebugRenderComponent* GetDebugRenderer(AActor* Actor);

private:
    TArray<FDebugDrawRequest> PendingDrawRequests;
    
    void ProcessDrawRequests();
    bool ShouldDrawCategory(EDebugCategory Category) const;
    float GetDrawDuration(float RequestedDuration) const;
};