#include "DebugRenderComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UDebugRenderComponent::UDebugRenderComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
#if !UE_BUILD_SHIPPING
    bGlobalDebugEnabled = true;
    EnabledCategories = (uint8)EDebugCategory::All;
#endif
}

void UDebugRenderComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UDebugRenderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ProcessDrawRequests();
}

void UDebugRenderComponent::SetCategoryEnabled(EDebugCategory Category, bool bEnabled)
{
    if (bEnabled)
    {
        EnabledCategories |= (uint8)Category;
    }
    else
    {
        EnabledCategories &= ~(uint8)Category;
    }
}

bool UDebugRenderComponent::IsCategoryEnabled(EDebugCategory Category) const
{
    return (EnabledCategories & (uint8)Category) != 0;
}

void UDebugRenderComponent::ToggleCategory(EDebugCategory Category)
{
    SetCategoryEnabled(Category, !IsCategoryEnabled(Category));
}

void UDebugRenderComponent::EnableAllCategories()
{
    EnabledCategories = (uint8)EDebugCategory::All;
}

void UDebugRenderComponent::DisableAllCategories()
{
    EnabledCategories = (uint8)EDebugCategory::None;
}

void UDebugRenderComponent::ToggleGlobalDebug()
{
    bGlobalDebugEnabled = !bGlobalDebugEnabled;
    UE_LOG(LogTemp, Warning, TEXT("Global Debug Rendering: %s"), bGlobalDebugEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UDebugRenderComponent::DrawDebugSphereAtLocation(const FVector& Location, float Radius, EDebugCategory Category,
    const FColor& Color, float Duration, int32 Segments)
{
    if (!ShouldDrawCategory(Category)) return;
    
    ::DrawDebugSphere(GetWorld(), Location, Radius, Segments, Color, false, GetDrawDuration(Duration));
}

void UDebugRenderComponent::DrawDebugLineFromTo(const FVector& Start, const FVector& End, EDebugCategory Category,
    const FColor& Color, float Duration, float Thickness)
{
    if (!ShouldDrawCategory(Category)) return;
    
    ::DrawDebugLine(GetWorld(), Start, End, Color, false, GetDrawDuration(Duration), 0, Thickness);
}

void UDebugRenderComponent::DrawDebugBoxAtLocation(const FVector& Location, const FVector& Extent, EDebugCategory Category,
    const FColor& Color, float Duration)
{
    if (!ShouldDrawCategory(Category)) return;
    
    ::DrawDebugBox(GetWorld(), Location, Extent, Color, false, GetDrawDuration(Duration));
}

void UDebugRenderComponent::DrawDebugArrowFromTo(const FVector& Start, const FVector& End, EDebugCategory Category,
    const FColor& Color, float Duration, float ArrowSize, float Thickness)
{
    if (!ShouldDrawCategory(Category)) return;
    
    ::DrawDebugDirectionalArrow(GetWorld(), Start, End, ArrowSize, Color, false, GetDrawDuration(Duration), 0, Thickness);
}

void UDebugRenderComponent::DrawDebugCircleAtLocation(const FVector& Location, float Radius, int32 Segments, EDebugCategory Category,
    const FColor& Color, float Duration, float Thickness, const FVector& YAxis, const FVector& ZAxis)
{
    if (!ShouldDrawCategory(Category)) return;
    
    ::DrawDebugCircle(GetWorld(), Location, Radius, Segments, Color, false, GetDrawDuration(Duration), 0, Thickness, YAxis, ZAxis);
}

void UDebugRenderComponent::DrawDebugTextAtLocation(const FVector& Location, const FString& Text, EDebugCategory Category,
    const FColor& Color, float Duration)
{
    if (!ShouldDrawCategory(Category)) return;
    
    ::DrawDebugString(GetWorld(), Location, Text, nullptr, Color, GetDrawDuration(Duration), true);
}

void UDebugRenderComponent::DrawCombatHitbox(const FVector& Location, float Radius, bool bIsActiveHitbox)
{
    if (!ShouldDrawCategory(EDebugCategory::Combat)) return;
    
    FColor Color = bIsActiveHitbox ? FColor::Red : FColor::Orange;
    DrawDebugSphereAtLocation(Location, Radius, EDebugCategory::Combat, Color, 0.1f, 12);
    
    if (bIsActiveHitbox)
    {
        DrawDebugSphereAtLocation(Location, Radius * 1.1f, EDebugCategory::Combat, FColor::Green, 0.1f, 12);
    }
}

void UDebugRenderComponent::DrawKnockbackArrow(const FVector& Start, const FVector& Direction, float Force)
{
    if (!ShouldDrawCategory(EDebugCategory::Combat)) return;
    
    FVector End = Start + (Direction * Force * 0.5f);
    DrawDebugArrowFromTo(Start, End, EDebugCategory::Combat, FColor::Yellow, 2.0f, Force * 0.1f, 3.0f);
    
    FString ForceText = FString::Printf(TEXT("Knockback: %.0f"), Force);
    DrawDebugTextAtLocation(Start + FVector(0, 0, 100), ForceText, EDebugCategory::Combat, FColor::Yellow, 2.0f);
}

void UDebugRenderComponent::DrawStaggerIndicator(const FVector& Location, float Duration)
{
    if (!ShouldDrawCategory(EDebugCategory::Combat)) return;
    
    DrawDebugSphereAtLocation(Location + FVector(0, 0, 100), 30, EDebugCategory::Combat, FColor::Yellow, Duration, 16);
    DrawDebugTextAtLocation(Location + FVector(0, 0, 150), TEXT("STAGGERED"), EDebugCategory::Combat, FColor::Yellow, Duration);
}

void UDebugRenderComponent::DrawWallImpact(const FVector& ImpactLocation, const FVector& ImpactNormal)
{
    if (!ShouldDrawCategory(EDebugCategory::Physics)) return;
    
    DrawDebugSphereAtLocation(ImpactLocation, 75.0f, EDebugCategory::Physics, FColor::Red, 5.0f, 24);
    DrawDebugArrowFromTo(ImpactLocation, ImpactLocation + ImpactNormal * 200, EDebugCategory::Physics, FColor::Red, 5.0f, 30, 8.0f);
    DrawDebugTextAtLocation(ImpactLocation + FVector(0, 0, 100), TEXT("WALL IMPACT"), EDebugCategory::Physics, FColor::Red, 5.0f);
}

void UDebugRenderComponent::DrawFocusTarget(AActor* Target, bool bIsPrimary)
{
    if (!ShouldDrawCategory(EDebugCategory::Interactions) || !Target) return;
    
    FVector Location = Target->GetActorLocation();
    FColor Color = bIsPrimary ? FColor::Green : FColor::Cyan;
    
    DrawDebugBoxAtLocation(Location, FVector(50, 50, 50), EDebugCategory::Interactions, Color, 0.0f);
    
    if (bIsPrimary)
    {
        DrawDebugLineFromTo(GetOwner()->GetActorLocation(), Location, EDebugCategory::Interactions, Color, 0.0f, 2.0f);
    }
}

void UDebugRenderComponent::DrawInteractableHighlight(AActor* Interactable, const FColor& Color)
{
    if (!ShouldDrawCategory(EDebugCategory::Interactions) || !Interactable) return;
    
    FVector Location = Interactable->GetActorLocation();
    DrawDebugBoxAtLocation(Location, FVector(50, 50, 100), EDebugCategory::Interactions, Color, 0.0f);
    DrawDebugTextAtLocation(Location + FVector(0, 0, 120), TEXT("INTERACTABLE"), EDebugCategory::Interactions, Color, 0.0f);
}

void UDebugRenderComponent::DrawDashTrajectory(const FVector& Start, const FVector& End)
{
    if (!ShouldDrawCategory(EDebugCategory::Movement)) return;
    
    DrawDebugLineFromTo(Start, End, EDebugCategory::Movement, FColor::Yellow, 2.0f, 2.0f);
    DrawDebugSphereAtLocation(End, 30, EDebugCategory::Movement, FColor::Yellow, 2.0f, 8);
}

UDebugRenderComponent* UDebugRenderComponent::GetDebugRenderer(AActor* Actor)
{
    if (!Actor) return nullptr;
    
    UDebugRenderComponent* DebugComp = Actor->FindComponentByClass<UDebugRenderComponent>();
    if (!DebugComp)
    {
        DebugComp = NewObject<UDebugRenderComponent>(Actor, UDebugRenderComponent::StaticClass(), TEXT("DebugRenderComponent"));
        DebugComp->RegisterComponent();
    }
    
    return DebugComp;
}

void UDebugRenderComponent::ProcessDrawRequests()
{
    PendingDrawRequests.Empty();
}

bool UDebugRenderComponent::ShouldDrawCategory(EDebugCategory Category) const
{
#if UE_BUILD_SHIPPING
    return false;
#else
    return bGlobalDebugEnabled && IsCategoryEnabled(Category);
#endif
}

float UDebugRenderComponent::GetDrawDuration(float RequestedDuration) const
{
    return RequestedDuration < 0 ? DefaultDrawDuration : RequestedDuration;
}