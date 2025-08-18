#include "SoftLockComponent.h"
#include "CombatComponent.h"
#include "../Characters/EnemyCharacter.h"
#include "../Characters/GameCharacterBase.h"
#include "HealthComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

USoftLockComponent::USoftLockComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void USoftLockComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache references
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
    
    CachedCombatComponent = GetOwner()->FindComponentByClass<UCombatComponent>();
    
    if (CachedPlayerController)
    {
        LastPlayerInputRotation = CachedPlayerController->GetControlRotation();
    }
}

void USoftLockComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnabled || !CachedPlayerController)
    {
        if (bIsLocked)
        {
            DisengageLock();
        }
        return;
    }
    
    // Get current rotation and calculate player input
    FRotator CurrentRotation = CachedPlayerController->GetControlRotation();
    float YawDelta = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, LastPlayerInputRotation.Yaw);
    float InputSpeed = FMath::Abs(YawDelta) / DeltaTime;
    
    // Detect if player is actively rotating camera (threshold in degrees per second)
    bPlayerIsRotatingCamera = InputSpeed > 30.0f;
    
    if (bIsLocked)
    {
        TimeSinceLockEngaged += DeltaTime;
        
        // Check if we should break the lock
        if (ShouldBreakLock())
        {
            DisengageLock();
        }
        else if (CurrentTarget)
        {
            // Check if player is trying to look away from target
            float AngleToTarget = GetAngleToTarget(CurrentTarget);
            
            // If player is rotating camera AND moving away from target, don't fight them
            if (bPlayerIsRotatingCamera && AngleToTarget > 15.0f)
            {
                // Player is intentionally looking away, don't update camera
            }
            else if (!bPlayerIsRotatingCamera || AngleToTarget < 10.0f)
            {
                // Either no input, or player is looking at target - assist them
                UpdateCameraRotation();
            }
        }
    }
    else
    {
        // Not locked, scan for potential targets
        ScanForEnemies();
    }
    
    LastPlayerInputRotation = CurrentRotation;
    
    if (bDebugDraw)
    {
        DrawDebugInfo();
    }
}

void USoftLockComponent::ScanForEnemies()
{
    // Don't auto-lock if player is actively rotating camera
    if (bPlayerIsRotatingCamera)
    {
        return;
    }
    
    // Add a cooldown after disengaging to prevent immediate re-lock
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastDisengageTime < 0.5f) // 0.5 second cooldown
    {
        return;
    }
    
    AActor* BestTarget = FindBestTarget();
    if (BestTarget)
    {
        // Check if target is within reengage angle AND we're not fighting player input
        float AngleToTarget = GetAngleToTarget(BestTarget);
        if (AngleToTarget <= ReengageAngle && !bPlayerIsRotatingCamera)
        {
            EngageLock(BestTarget);
        }
    }
}

AActor* USoftLockComponent::FindBestTarget()
{
    if (!GetOwner())
    {
        return nullptr;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    AActor* BestTarget = nullptr;
    float BestScore = -1.0f;
    
    // Find all enemies in range
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(DetectionRange);
    
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
        return nullptr;
    }
    
    for (const FOverlapResult& Result : OverlapResults)
    {
        AActor* PotentialTarget = Result.GetActor();
        if (!IsTargetValid(PotentialTarget))
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, PotentialTarget->GetActorLocation());
        
        // Skip if too close
        if (Distance < MinLockDistance)
        {
            continue;
        }
        
        // Calculate score based on distance and angle
        float AngleToTarget = GetAngleToTarget(PotentialTarget);
        float AngleScore = 1.0f - (AngleToTarget / 180.0f); // Closer to center = higher score
        float DistanceScore = 1.0f - (Distance / DetectionRange); // Closer = higher score
        
        float TotalScore = (AngleScore * 2.0f) + DistanceScore; // Prioritize angle over distance
        
        if (TotalScore > BestScore)
        {
            BestScore = TotalScore;
            BestTarget = PotentialTarget;
        }
    }
    
    return BestTarget;
}

void USoftLockComponent::EngageLock(AActor* Target)
{
    if (!Target || bIsLocked)
    {
        return;
    }
    
    CurrentTarget = Target;
    bIsLocked = true;
    TimeSinceLockEngaged = 0.0f;
    CurrentTargetDistance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    
    OnSoftLockEngaged.Broadcast(Target);
    
    if (GEngine && bDebugDraw)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
            FString::Printf(TEXT("Soft Lock Engaged: %s"), *Target->GetName()));
    }
}

void USoftLockComponent::DisengageLock()
{
    if (!bIsLocked)
    {
        return;
    }
    
    AActor* PreviousTarget = CurrentTarget;
    CurrentTarget = nullptr;
    bIsLocked = false;
    TimeSinceLockEngaged = 0.0f;
    LastDisengageTime = GetWorld()->GetTimeSeconds(); // Track when we disengaged
    
    OnSoftLockDisengaged.Broadcast();
    
    if (GEngine && bDebugDraw && PreviousTarget)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
            FString::Printf(TEXT("Soft Lock Disengaged: %s"), *PreviousTarget->GetName()));
    }
}

void USoftLockComponent::UpdateCameraRotation()
{
    if (!CurrentTarget || !CachedPlayerController || !CachedCamera)
    {
        return;
    }
    
    // Get target location (aim for chest height)
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    if (ACharacter* TargetChar = Cast<ACharacter>(CurrentTarget))
    {
        TargetLocation.Z += 50.0f; // Chest height
    }
    
    FVector CameraLocation = CachedCamera->GetComponentLocation();
    FVector LookDirection = (TargetLocation - CameraLocation).GetSafeNormal();
    FRotator DesiredRotation = LookDirection.Rotation();
    
    // Get current camera rotation
    FRotator CurrentRotation = CachedPlayerController->GetControlRotation();
    
    // Calculate angle difference (properly handles both positive and negative)
    float YawDiff = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, DesiredRotation.Yaw);
    float PitchDiff = FMath::FindDeltaAngleDegrees(CurrentRotation.Pitch, DesiredRotation.Pitch);
    
    // Store absolute values for comparisons
    float AbsYawDiff = FMath::Abs(YawDiff);
    float AbsPitchDiff = FMath::Abs(PitchDiff);
    
    // If we're already looking at the target (within deadzone), don't adjust
    float DeadZoneAngle = 5.0f; // Degrees
    if (AbsYawDiff < DeadZoneAngle && AbsPitchDiff < DeadZoneAngle)
    {
        return; // Already looking at target, no adjustment needed
    }
    
    // Calculate distance-based interpolation speed
    CurrentTargetDistance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (CurrentTargetDistance / DetectionRange), 0.3f, 1.0f);
    
    // Check if player input is moving TOWARD or AWAY from target
    bool bPlayerMovingTowardTarget = false;
    if (bPlayerIsRotatingCamera)
    {
        // Get the direction of player input
        float InputDirection = FMath::FindDeltaAngleDegrees(LastPlayerInputRotation.Yaw, CurrentRotation.Yaw);
        
        // If input and correction are in same direction, player is helping
        bPlayerMovingTowardTarget = (FMath::Sign(InputDirection) == FMath::Sign(YawDiff));
    }
    
    // Adjust speed based on whether player is cooperating or resisting
    float InterpSpeedMultiplier = 1.0f;
    if (bPlayerIsRotatingCamera)
    {
        if (bPlayerMovingTowardTarget)
        {
            // Player is helping, increase speed slightly
            InterpSpeedMultiplier = 1.2f;
        }
        else
        {
            // Player is resisting, back off significantly
            InterpSpeedMultiplier = 0.1f;
        }
    }
    
    float AdjustedInterpSpeed = CameraInterpSpeed * DistanceFactor * InterpSpeedMultiplier;
    
    // Apply different speeds for different angle ranges
    if (AbsYawDiff > 45.0f)
    {
        // Far from target, use slower speed to avoid jarring movement
        AdjustedInterpSpeed *= 0.5f;
    }
    else if (AbsYawDiff < 15.0f)
    {
        // Close to target, use faster speed for precision
        AdjustedInterpSpeed *= 1.5f;
    }
    
    // Smoothly interpolate to desired rotation
    FRotator NewRotation;
    
    // Only adjust yaw if difference is significant
    if (AbsYawDiff > DeadZoneAngle)
    {
        // If player is actively resisting, don't force it
        if (bPlayerIsRotatingCamera && !bPlayerMovingTowardTarget && AbsYawDiff > 20.0f)
        {
            NewRotation.Yaw = CurrentRotation.Yaw; // Don't fight the player
        }
        else
        {
            NewRotation.Yaw = FMath::FInterpTo(CurrentRotation.Yaw, DesiredRotation.Yaw, 
                GetWorld()->GetDeltaSeconds(), AdjustedInterpSpeed);
        }
    }
    else
    {
        NewRotation.Yaw = CurrentRotation.Yaw;
    }
    
    // Very gentle pitch adjustment
    if (AbsPitchDiff > DeadZoneAngle * 2.0f) // Larger deadzone for pitch
    {
        NewRotation.Pitch = FMath::FInterpTo(CurrentRotation.Pitch, DesiredRotation.Pitch, 
            GetWorld()->GetDeltaSeconds(), AdjustedInterpSpeed * 0.3f); // Much slower pitch
    }
    else
    {
        NewRotation.Pitch = CurrentRotation.Pitch;
    }
    
    NewRotation.Roll = 0.0f;
    
    // Apply the rotation
    CachedPlayerController->SetControlRotation(NewRotation);
    
    // Debug info
    if (bDebugDraw && GEngine)
    {
        FString DebugText = FString::Printf(
            TEXT("YawDiff: %.1f | PlayerInput: %s | MovingToward: %s | Speed: %.2f"),
            YawDiff,
            bPlayerIsRotatingCamera ? TEXT("YES") : TEXT("NO"),
            bPlayerMovingTowardTarget ? TEXT("YES") : TEXT("NO"),
            AdjustedInterpSpeed
        );
        GEngine->AddOnScreenDebugMessage(100, 0.0f, FColor::Yellow, DebugText);
    }
}

bool USoftLockComponent::ShouldBreakLock()
{
    if (!CurrentTarget)
    {
        return true;
    }
    
    // Check if target is still valid
    if (!IsTargetValid(CurrentTarget))
    {
        return true;
    }
    
    // Check distance
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    if (Distance > DetectionRange * 1.2f) // 20% buffer to prevent flickering
    {
        return true;
    }
    
    // Check if player has rotated away from target
    float AngleToTarget = GetAngleToTarget(CurrentTarget);
    if (AngleToTarget > BreakAwayAngle)
    {
        return true;
    }
    
    return false;
}

bool USoftLockComponent::IsTargetValid(AActor* Target) const
{
    if (!Target)
    {
        return false;
    }
    
    // Check if it's an enemy
    AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Target);
    if (!Enemy)
    {
        return false;
    }
    
    // Check if enemy is alive
    UHealthComponent* HealthComp = Enemy->GetHealthComponent();
    if (!HealthComp || HealthComp->IsDead())
    {
        return false;
    }
    
    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);
    
    bool bHasLineOfSight = !GetWorld()->LineTraceSingleByChannel(
        HitResult,
        GetOwner()->GetActorLocation() + FVector(0, 0, 50), // Slightly elevated
        Target->GetActorLocation() + FVector(0, 0, 50),
        ECC_Visibility,
        QueryParams
    );
    
    return bHasLineOfSight;
}

float USoftLockComponent::GetAngleToTarget(AActor* Target) const
{
    if (!Target || !CachedPlayerController)
    {
        return 180.0f;
    }
    
    FVector CameraLocation = CachedPlayerController->PlayerCameraManager->GetCameraLocation();
    FVector CameraForward = CachedPlayerController->GetControlRotation().Vector();
    FVector ToTarget = (Target->GetActorLocation() - CameraLocation).GetSafeNormal();
    
    float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
    float Angle = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
    
    return FMath::RadiansToDegrees(Angle);
}

void USoftLockComponent::DrawDebugInfo() const
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Draw detection sphere
    DrawDebugSphere(GetWorld(), OwnerLocation, DetectionRange, 24, FColor::Blue, false, -1.0f, 0, 1.0f);
    
    // Draw lock state
    if (bIsLocked && CurrentTarget)
    {
        // Line to target
        DrawDebugLine(GetWorld(), OwnerLocation, CurrentTarget->GetActorLocation(), 
            FColor::Green, false, -1.0f, 0, 3.0f);
        
        // Box around target
        DrawDebugBox(GetWorld(), CurrentTarget->GetActorLocation(), 
            FVector(30, 30, 60), FColor::Green, false, -1.0f, 0, 3.0f);
        
        // Info text
        FString InfoText = FString::Printf(TEXT("Locked: %s\nDist: %.0f\nAngle: %.1f°"), 
            *CurrentTarget->GetName(), CurrentTargetDistance, GetAngleToTarget(CurrentTarget));
        DrawDebugString(GetWorld(), CurrentTarget->GetActorLocation() + FVector(0, 0, 120), 
            InfoText, nullptr, FColor::Green, 0.0f, true);
    }
    
    // Draw break away angle cone
    if (CachedPlayerController)
    {
        FVector CameraForward = CachedPlayerController->GetControlRotation().Vector();
        FVector CameraLocation = CachedPlayerController->PlayerCameraManager->GetCameraLocation();
        
        // Draw cone for break away angle
        float ConeLength = 500.0f;
        DrawDebugCone(GetWorld(), CameraLocation, CameraForward, ConeLength, 
            FMath::DegreesToRadians(BreakAwayAngle), FMath::DegreesToRadians(BreakAwayAngle), 
            12, FColor::Red, false, -1.0f, 0, 1.0f);
        
        // Draw cone for reengage angle
        DrawDebugCone(GetWorld(), CameraLocation, CameraForward, ConeLength, 
            FMath::DegreesToRadians(ReengageAngle), FMath::DegreesToRadians(ReengageAngle), 
            12, FColor::Yellow, false, -1.0f, 0, 1.0f);
    }
}