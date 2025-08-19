#include "FocusModeComponent.h"
#include "../Interfaces/IInteractable.h"
#include "../Core/AtlasGameplayTags.h"
#include "../Characters/GameCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "CombatComponent.h"
#include "Engine/OverlapResult.h"

UFocusModeComponent::UFocusModeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UFocusModeComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        CachedPlayerController = Cast<APlayerController>(OwnerPawn->GetController());
        
        if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(OwnerPawn))
        {
            CachedCamera = Character->GetFollowCamera();
        }
    }
    
    if (!CachedCamera)
    {
        CachedCamera = GetOwner()->FindComponentByClass<UCameraComponent>();
    }
}

void UFocusModeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsFocusModeActive)
    {
        return;
    }
    
    TimeSinceFocusStart += DeltaTime;
    
    ScanForTargets();
    UpdateFocusTarget();
    
    if (bDebugDrawFocusInfo)
    {
        DrawDebugInfo();
    }
}

void UFocusModeComponent::StartFocusMode()
{
    if (bIsFocusModeActive)
    {
        return;
    }
    
    bIsFocusModeActive = true;
    TimeSinceFocusStart = 0.0f;
    
    if (UCombatComponent* CombatComp = GetOwner()->FindComponentByClass<UCombatComponent>())
    {
        CombatComp->CombatStateTags.AddTag(FAtlasGameplayTags::Get().Ability_Focus);
    }
    
    OnFocusModeChanged.Broadcast(true);
    
    UE_LOG(LogTemp, Log, TEXT("Focus Mode Started"));
}

void UFocusModeComponent::StopFocusMode()
{
    if (!bIsFocusModeActive)
    {
        return;
    }
    
    bIsFocusModeActive = false;
    
    ClearFocusedTarget();
    PotentialTargets.Empty();
    
    if (UCombatComponent* CombatComp = GetOwner()->FindComponentByClass<UCombatComponent>())
    {
        CombatComp->CombatStateTags.RemoveTag(FAtlasGameplayTags::Get().Ability_Focus);
    }
    
    OnFocusModeChanged.Broadcast(false);
    
    UE_LOG(LogTemp, Log, TEXT("Focus Mode Stopped"));
}

void UFocusModeComponent::ToggleFocusMode()
{
    if (bIsFocusModeActive)
    {
        StopFocusMode();
    }
    else
    {
        StartFocusMode();
    }
}

void UFocusModeComponent::ScanForTargets()
{
    PotentialTargets.Empty();
    
    if (!GetOwner() || !CachedCamera)
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(FocusRange);
    
    bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        OwnerLocation,
        FQuat::Identity,
        ECC_Pawn,
        SphereShape,
        QueryParams
    );
    
    if (!bHasOverlaps)
    {
        return;
    }
    
    for (const FOverlapResult& Result : OverlapResults)
    {
        if (AActor* Actor = Result.GetActor())
        {
            if (IsTargetInteractable(Actor))
            {
                FFocusTarget Target = EvaluateTarget(Actor);
                if (Target.Priority > 0.0f)
                {
                    PotentialTargets.Add(Target);
                }
            }
        }
    }
    
    PotentialTargets.Sort([](const FFocusTarget& A, const FFocusTarget& B)
    {
        return A.Priority > B.Priority;
    });
}

void UFocusModeComponent::UpdateFocusTarget()
{
    if (PotentialTargets.Num() == 0)
    {
        if (CurrentFocusedTarget)
        {
            ClearFocusedTarget();
        }
        return;
    }
    
    AActor* BestTarget = nullptr;
    float BestPriority = -1.0f;
    
    for (const FFocusTarget& Target : PotentialTargets)
    {
        if (Target.Priority > BestPriority)
        {
            BestPriority = Target.Priority;
            BestTarget = Target.Actor;
        }
    }
    
    if (BestTarget != CurrentFocusedTarget)
    {
        SetFocusedTarget(BestTarget);
    }
    
    if (CurrentFocusedTarget)
    {
        for (const FFocusTarget& Target : PotentialTargets)
        {
            if (Target.Actor == CurrentFocusedTarget)
            {
                CurrentTargetInfo = Target;
                break;
            }
        }
    }
}

void UFocusModeComponent::SetFocusedTarget(AActor* NewTarget)
{
    if (CurrentFocusedTarget == NewTarget)
    {
        return;
    }
    
    if (CurrentFocusedTarget)
    {
        if (CurrentFocusedTarget->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
        {
            IInteractable::Execute_OnUnfocused(CurrentFocusedTarget, GetOwner());
        }
    }
    
    CurrentFocusedTarget = NewTarget;
    
    if (CurrentFocusedTarget)
    {
        if (CurrentFocusedTarget->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
        {
            IInteractable::Execute_OnFocused(CurrentFocusedTarget, GetOwner());
        }
        
        OnTargetFocused.Broadcast(CurrentFocusedTarget);
        
        UE_LOG(LogTemp, Log, TEXT("Focused on: %s"), *CurrentFocusedTarget->GetName());
    }
}

void UFocusModeComponent::ClearFocusedTarget()
{
    if (CurrentFocusedTarget)
    {
        if (CurrentFocusedTarget->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
        {
            IInteractable::Execute_OnUnfocused(CurrentFocusedTarget, GetOwner());
        }
        
        OnTargetUnfocused.Broadcast();
        
        UE_LOG(LogTemp, Log, TEXT("Unfocused from: %s"), *CurrentFocusedTarget->GetName());
    }
    
    CurrentFocusedTarget = nullptr;
    CurrentTargetInfo = FFocusTarget();
}

FFocusTarget UFocusModeComponent::EvaluateTarget(AActor* Target) const
{
    FFocusTarget Result;
    Result.Actor = Target;
    
    if (!Target || !CachedCamera || !CachedPlayerController)
    {
        return Result;
    }
    
    FVector TargetLocation = Target->GetActorLocation();
    
    if (Target->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        TargetLocation = IInteractable::Execute_GetFocusLocation(Target);
        Result.bIsInteractable = true;
    }
    
    Result.bIsEnemy = false; // Focus mode is only for interactables
    
    Result.WorldDistance = FVector::Dist(GetOwner()->GetActorLocation(), TargetLocation);
    
    if (Result.WorldDistance > FocusRange)
    {
        Result.Priority = 0.0f;
        return Result;
    }
    
    FVector2D ScreenPos = GetScreenPosition(TargetLocation);
    Result.ScreenDistance = CalculateScreenDistance(ScreenPos);
    
    FVector CameraLocation = CachedCamera->GetComponentLocation();
    FVector DirectionToTarget = (TargetLocation - CameraLocation).GetSafeNormal();
    FVector CameraForward = CachedCamera->GetForwardVector();
    
    float DotProduct = FVector::DotProduct(CameraForward, DirectionToTarget);
    
    if (DotProduct < 0.3f)
    {
        Result.Priority = 0.0f;
        return Result;
    }
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);
    
    bool bHasLineOfSight = !GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CameraLocation,
        TargetLocation,
        ECC_Visibility,
        QueryParams
    );
    
    if (!bHasLineOfSight)
    {
        Result.Priority = 0.0f;
        return Result;
    }
    
    Result.Priority = CalculatePriority(Result);
    
    return Result;
}

FVector2D UFocusModeComponent::GetScreenPosition(const FVector& WorldPosition) const
{
    if (!CachedPlayerController)
    {
        return FVector2D::ZeroVector;
    }
    
    FVector2D ScreenPosition;
    CachedPlayerController->ProjectWorldLocationToScreen(WorldPosition, ScreenPosition);
    
    return ScreenPosition;
}

float UFocusModeComponent::CalculateScreenDistance(const FVector2D& ScreenPos) const
{
    if (!CachedPlayerController)
    {
        return 999999.0f;
    }
    
    int32 ViewportSizeX, ViewportSizeY;
    CachedPlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
    
    FVector2D ScreenCenter(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);
    
    return FVector2D::Distance(ScreenPos, ScreenCenter);
}

float UFocusModeComponent::CalculatePriority(const FFocusTarget& Target) const
{
    float Priority = 100.0f;
    
    float ScreenDistanceFactor = FMath::Clamp(1.0f - (Target.ScreenDistance / ScreenCenterTolerance), 0.0f, 1.0f);
    Priority *= ScreenDistanceFactor;
    
    float DistanceFactor = FMath::Clamp(1.0f - (Target.WorldDistance / FocusRange), 0.0f, 1.0f);
    Priority *= (0.5f + DistanceFactor * 0.5f);
    
    if (Target.bIsInteractable)
    {
        Priority *= InteractablePriorityBonus;
    }
    
    return Priority;
}


bool UFocusModeComponent::TryInteractWithFocusedTarget()
{
    
    if (!CurrentFocusedTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("No focused target"));
        return false;
    }
    
    if (!CurrentTargetInfo.bIsInteractable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target %s is not interactable"), *CurrentFocusedTarget->GetName());
        return false;
    }
    
    if (!CurrentFocusedTarget->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Target %s doesn't implement IInteractable interface"), *CurrentFocusedTarget->GetName());
        return false;
    }
    
    if (!IInteractable::Execute_CanInteract(CurrentFocusedTarget, GetOwner()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot interact with %s (on cooldown or not ready)"), *CurrentFocusedTarget->GetName());
        return false;
    }
    
    // Use focus range for interaction, not individual interaction distance
    // This allows interaction from anywhere within focus range (1000 units)
    if (CurrentTargetInfo.WorldDistance > FocusRange)
    {
        UE_LOG(LogTemp, Warning, TEXT("Too far to interact with %s (Distance: %.0f, Max: %.0f)"), 
            *CurrentFocusedTarget->GetName(), CurrentTargetInfo.WorldDistance, FocusRange);
        return false;
    }
    
    IInteractable::Execute_Interact(CurrentFocusedTarget, GetOwner());
    bWasInteracting = true;
    
    return true;
}

bool UFocusModeComponent::IsTargetInteractable(AActor* Target) const
{
    if (!Target)
    {
        return false;
    }
    
    bool bImplementsInterface = Target->GetClass()->ImplementsInterface(UInteractable::StaticClass());
    
    if (bImplementsInterface)
    {
        UE_LOG(LogTemp, Verbose, TEXT("Target %s implements IInteractable"), *Target->GetName());
    }
    
    return bImplementsInterface;
}

void UFocusModeComponent::DrawDebugInfo() const
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    DrawDebugSphere(GetWorld(), OwnerLocation, FocusRange, 32, FColor::Blue, false, -1.0f, 0, 2.0f);
    
    for (const FFocusTarget& Target : PotentialTargets)
    {
        if (Target.Actor)
        {
            FColor Color = Target.Actor == CurrentFocusedTarget ? FColor::Green : FColor::Yellow;
            
            if (Target.bIsInteractable)
            {
                Color = Target.Actor == CurrentFocusedTarget ? FColor::Cyan : FColor::Blue;
            }
            
            DrawDebugLine(GetWorld(), OwnerLocation, Target.Actor->GetActorLocation(), Color, false, -1.0f, 0, 2.0f);
            
            FString DebugText = FString::Printf(TEXT("%s\nPriority: %.1f\nDist: %.0f"), 
                *Target.Actor->GetName(), Target.Priority, Target.WorldDistance);
            DrawDebugString(GetWorld(), Target.Actor->GetActorLocation() + FVector(0, 0, 100), DebugText, nullptr, Color, 0.0f, true);
        }
    }
    
    if (CurrentFocusedTarget)
    {
        DrawDebugBox(GetWorld(), CurrentFocusedTarget->GetActorLocation(), FVector(50, 50, 50), FColor::Green, false, -1.0f, 0, 5.0f);
    }
}

