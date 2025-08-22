# Phase 4: Environmental Systems & Interactables - Ultra-Detailed Implementation Plan

## Implementation Status: ✅ COMPLETE (2025-01-22)

## Executive Summary
Phase 4 transforms Atlas's static environments into dynamic, hazardous spaces that react to combat and degrade over time. This phase implements the station's environmental storytelling through interactive systems, hazards, and atmospheric effects that enhance the roguelite experience.

---

## Part 1: Station Integrity Visual System

### 1.1 Core Integrity Visualization Component

#### UIntegrityVisualizerComponent
```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UIntegrityVisualizerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Integrity thresholds for visual states
    UPROPERTY(EditAnywhere, Category = "Integrity")
    float CriticalThreshold = 25.0f;
    
    UPROPERTY(EditAnywhere, Category = "Integrity")
    float WarningThreshold = 50.0f;
    
    UPROPERTY(EditAnywhere, Category = "Integrity")
    float MinorDamageThreshold = 75.0f;
    
    // Visual effect assets
    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* SparksEffect;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* SteamLeakEffect;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* ElectricalArcEffect;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    USoundCue* AlarmSound;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    USoundCue* CreakingSound;
    
    // Update visual state based on integrity
    UFUNCTION(BlueprintCallable)
    void UpdateIntegrityVisuals(float CurrentIntegrity);
    
    // Trigger specific damage effects
    UFUNCTION(BlueprintCallable)
    void TriggerHullBreach(FVector Location);
    
    UFUNCTION(BlueprintCallable)
    void TriggerElectricalFailure(FVector Location);
    
    UFUNCTION(BlueprintCallable)
    void TriggerEmergencyLighting();
    
private:
    void SpawnDamageEffects(float IntegrityPercent);
    void UpdateLightingState(float IntegrityPercent);
    void PlayIntegrityAudio(float IntegrityPercent);
};
```

### 1.2 Visual State Implementations

#### Critical State (0-25% Integrity)
```cpp
void UIntegrityVisualizerComponent::HandleCriticalState()
{
    // Red emergency lighting
    SetAllLightsColor(FLinearColor::Red);
    SetLightIntensity(0.5f);
    
    // Strobe effect
    EnableStrobeLighting(2.0f); // 2Hz strobe
    
    // Maximum particle effects
    SpawnSparksAtInterval(0.5f);
    SpawnSteamLeaks(10);
    
    // Audio
    PlayAlarmLoop(ESeverity::Critical);
    PlayCreakingSound(1.0f);
    
    // Screen effects
    ApplyCameraShake(ECameraShakeIntensity::Heavy);
    ApplyScreenDistortion(0.3f);
    
    // Hull breach effects
    CreateHullBreachPoints(3);
}
```

#### Warning State (25-50% Integrity)
```cpp
void UIntegrityVisualizerComponent::HandleWarningState()
{
    // Orange emergency lighting
    SetAllLightsColor(FLinearColor(1.0f, 0.5f, 0.0f));
    SetLightIntensity(0.7f);
    
    // Occasional flicker
    EnableLightFlicker(5.0f); // Flicker every 5 seconds
    
    // Moderate particle effects
    SpawnSparksAtInterval(2.0f);
    SpawnSteamLeaks(5);
    
    // Audio
    PlayAlarmLoop(ESeverity::Warning);
    PlayCreakingSound(0.5f);
    
    // Screen effects
    ApplyCameraShake(ECameraShakeIntensity::Medium);
}
```

#### Minor Damage State (50-75% Integrity)
```cpp
void UIntegrityVisualizerComponent::HandleMinorDamageState()
{
    // Yellow lighting tint
    SetAllLightsColor(FLinearColor(1.0f, 0.9f, 0.7f));
    SetLightIntensity(0.85f);
    
    // Rare flicker
    EnableLightFlicker(10.0f);
    
    // Minimal particle effects
    SpawnSparksAtInterval(5.0f);
    SpawnSteamLeaks(2);
    
    // Audio
    PlayAmbientCreaking(0.25f);
    
    // Screen effects
    ApplyCameraShake(ECameraShakeIntensity::Light);
}
```

### 1.3 Dynamic Damage Manifestation

#### Hull Breach System
```cpp
USTRUCT(BlueprintType)
struct FHullBreachData
{
    GENERATED_BODY()
    
    UPROPERTY()
    FVector Location;
    
    UPROPERTY()
    float Severity = 1.0f;
    
    UPROPERTY()
    float Radius = 500.0f;
    
    UPROPERTY()
    UParticleSystemComponent* VacuumEffect;
    
    UPROPERTY()
    UAudioComponent* VacuumSound;
};

class UHullBreachManager : public UGameInstanceSubsystem
{
public:
    // Create dynamic hull breach
    void CreateHullBreach(const FVector& ImpactLocation, float Damage)
    {
        FHullBreachData Breach;
        Breach.Location = ImpactLocation;
        Breach.Severity = FMath::Clamp(Damage / 100.0f, 0.1f, 1.0f);
        
        // Spawn vacuum effect
        Breach.VacuumEffect = SpawnVacuumParticles(ImpactLocation);
        
        // Apply suction force
        ApplySuctionForce(ImpactLocation, Breach.Radius, Breach.Severity);
        
        // Spawn debris
        SpawnDebris(ImpactLocation, Breach.Severity);
        
        // Add to active breaches
        ActiveBreaches.Add(Breach);
    }
};
```

---

## Part 2: Environmental Hazard System

### 2.1 Base Hazard Component

#### UEnvironmentalHazardComponent
```cpp
UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UEnvironmentalHazardComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    // Hazard configuration
    UPROPERTY(EditAnywhere, Category = "Hazard")
    float HazardRadius = 300.0f;
    
    UPROPERTY(EditAnywhere, Category = "Hazard")
    float DamagePerSecond = 10.0f;
    
    UPROPERTY(EditAnywhere, Category = "Hazard")
    float ActivationDelay = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Hazard")
    bool bPermanent = false;
    
    UPROPERTY(EditAnywhere, Category = "Hazard")
    float Duration = 10.0f;
    
    // Visual/Audio
    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* HazardEffect;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    USoundCue* HazardSound;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    UMaterialInterface* HazardDecal;
    
    // Activation
    UFUNCTION(BlueprintCallable)
    virtual void ActivateHazard();
    
    UFUNCTION(BlueprintCallable)
    virtual void DeactivateHazard();
    
    UFUNCTION(BlueprintNativeEvent)
    void OnActorEnterHazard(AActor* Actor);
    
    UFUNCTION(BlueprintNativeEvent)
    void OnActorExitHazard(AActor* Actor);
    
protected:
    virtual void ApplyHazardEffect(AActor* Actor, float DeltaTime);
    virtual void UpdateHazardVisuals();
    
    UPROPERTY()
    TArray<AActor*> AffectedActors;
    
    bool bIsActive = false;
    float CurrentDuration = 0.0f;
};
```

### 2.2 Specific Hazard Implementations

#### Electrical Surge Hazard
```cpp
UCLASS()
class ATLAS_API UElectricalSurgeHazard : public UEnvironmentalHazardComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Electrical")
    float StunDuration = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Electrical")
    float ChainRadius = 200.0f;
    
    UPROPERTY(EditAnywhere, Category = "Electrical")
    int32 MaxChainTargets = 3;
    
protected:
    virtual void ApplyHazardEffect(AActor* Actor, float DeltaTime) override
    {
        // Apply electrical damage
        UGameplayStatics::ApplyPointDamage(
            Actor,
            DamagePerSecond * DeltaTime,
            GetComponentLocation(),
            nullptr,
            nullptr,
            UElectricalDamageType::StaticClass()
        );
        
        // Apply stun
        if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
        {
            Character->ApplyStun(StunDuration);
        }
        
        // Chain to nearby targets
        ChainElectricity(Actor);
    }
    
    void ChainElectricity(AActor* Source)
    {
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(
            GetWorld(),
            AGameCharacterBase::StaticClass(),
            NearbyActors
        );
        
        int32 ChainCount = 0;
        for (AActor* Target : NearbyActors)
        {
            if (Target != Source && 
                FVector::Dist(Source->GetActorLocation(), Target->GetActorLocation()) < ChainRadius)
            {
                // Spawn chain effect
                SpawnElectricalArc(Source->GetActorLocation(), Target->GetActorLocation());
                
                // Apply reduced damage
                UGameplayStatics::ApplyPointDamage(
                    Target,
                    DamagePerSecond * 0.5f,
                    Target->GetActorLocation(),
                    nullptr,
                    nullptr,
                    UElectricalDamageType::StaticClass()
                );
                
                ChainCount++;
                if (ChainCount >= MaxChainTargets) break;
            }
        }
    }
};
```

#### Toxic Leak Hazard
```cpp
UCLASS()
class ATLAS_API UToxicLeakHazard : public UEnvironmentalHazardComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Toxic")
    float VisionImpairment = 0.5f;
    
    UPROPERTY(EditAnywhere, Category = "Toxic")
    float MovementSlowPercent = 0.3f;
    
    UPROPERTY(EditAnywhere, Category = "Toxic")
    float DOTDuration = 5.0f;
    
protected:
    virtual void ApplyHazardEffect(AActor* Actor, float DeltaTime) override
    {
        if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
        {
            // Apply toxic damage
            Character->TakeDamage(DamagePerSecond * DeltaTime, 
                FDamageEvent(UToxicDamageType::StaticClass()), nullptr, this);
            
            // Apply vision impairment
            if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
            {
                ApplyToxicScreenEffect(PC, VisionImpairment);
            }
            
            // Apply movement slow
            Character->GetCharacterMovement()->MaxWalkSpeed *= (1.0f - MovementSlowPercent);
            
            // Apply DOT
            ApplyPoisonDOT(Character, DOTDuration);
        }
    }
    
    void ApplyToxicScreenEffect(APlayerController* PC, float Intensity)
    {
        // Green tint overlay
        PC->PlayerCameraManager->SetManualCameraFade(
            Intensity,
            FLinearColor(0.0f, 1.0f, 0.0f, 0.3f),
            false
        );
        
        // Blur effect
        PC->PlayerCameraManager->PostProcessSettings.DepthOfFieldFocalDistance = 100.0f;
        PC->PlayerCameraManager->PostProcessSettings.DepthOfFieldScale = Intensity;
    }
};
```

#### Low Gravity Hazard
```cpp
UCLASS()
class ATLAS_API ULowGravityHazard : public UEnvironmentalHazardComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Gravity")
    float GravityScale = 0.3f;
    
    UPROPERTY(EditAnywhere, Category = "Gravity")
    float JumpBoostMultiplier = 2.0f;
    
    UPROPERTY(EditAnywhere, Category = "Gravity")
    float AirControlBoost = 2.0f;
    
protected:
    virtual void OnActorEnterHazard_Implementation(AActor* Actor) override
    {
        if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
        {
            // Store original values
            OriginalGravityScale.Add(Character, Character->GetCharacterMovement()->GravityScale);
            OriginalJumpVelocity.Add(Character, Character->GetCharacterMovement()->JumpZVelocity);
            OriginalAirControl.Add(Character, Character->GetCharacterMovement()->AirControl);
            
            // Apply low gravity
            Character->GetCharacterMovement()->GravityScale = GravityScale;
            Character->GetCharacterMovement()->JumpZVelocity *= JumpBoostMultiplier;
            Character->GetCharacterMovement()->AirControl = AirControlBoost;
            
            // Visual effect
            SpawnAntiGravityParticles(Character);
        }
    }
    
    virtual void OnActorExitHazard_Implementation(AActor* Actor) override
    {
        if (AGameCharacterBase* Character = Cast<AGameCharacterBase>(Actor))
        {
            // Restore original values
            if (OriginalGravityScale.Contains(Character))
            {
                Character->GetCharacterMovement()->GravityScale = OriginalGravityScale[Character];
                Character->GetCharacterMovement()->JumpZVelocity = OriginalJumpVelocity[Character];
                Character->GetCharacterMovement()->AirControl = OriginalAirControl[Character];
                
                // Clean up
                OriginalGravityScale.Remove(Character);
                OriginalJumpVelocity.Remove(Character);
                OriginalAirControl.Remove(Character);
            }
        }
    }
    
private:
    TMap<AGameCharacterBase*, float> OriginalGravityScale;
    TMap<AGameCharacterBase*, float> OriginalJumpVelocity;
    TMap<AGameCharacterBase*, float> OriginalAirControl;
};
```

---

## Part 3: Interactable Systems Implementation

### 3.1 Base Interactable Framework

#### UInteractableComponent
```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UInteractableComponent : public UActorComponent, public IInteractable
{
    GENERATED_BODY()

public:
    // Configuration
    UPROPERTY(EditAnywhere, Category = "Interactable")
    FGameplayTag InteractableTag;
    
    UPROPERTY(EditAnywhere, Category = "Interactable")
    float InteractionRange = 200.0f;
    
    UPROPERTY(EditAnywhere, Category = "Interactable")
    float InteractionTime = 0.0f; // 0 = instant
    
    UPROPERTY(EditAnywhere, Category = "Interactable")
    float Cooldown = 10.0f;
    
    UPROPERTY(EditAnywhere, Category = "Interactable")
    int32 MaxUses = -1; // -1 = unlimited
    
    UPROPERTY(EditAnywhere, Category = "Interactable")
    bool bRequiresLineOfSight = true;
    
    // Visual feedback
    UPROPERTY(EditAnywhere, Category = "Visuals")
    UStaticMeshComponent* InteractableMesh;
    
    UPROPERTY(EditAnywhere, Category = "Visuals")
    UWidgetComponent* InteractionPrompt;
    
    UPROPERTY(EditAnywhere, Category = "Visuals")
    UMaterialInterface* ReadyMaterial;
    
    UPROPERTY(EditAnywhere, Category = "Visuals")
    UMaterialInterface* CooldownMaterial;
    
    UPROPERTY(EditAnywhere, Category = "Visuals")
    UMaterialInterface* DisabledMaterial;
    
    // IInteractable interface
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual void OnInteract_Implementation(AActor* Interactor) override;
    virtual float GetInteractionTime_Implementation() override { return InteractionTime; }
    virtual FText GetInteractionPrompt_Implementation() override;
    
    // State management
    UFUNCTION(BlueprintCallable)
    void SetInteractableEnabled(bool bEnabled);
    
    UFUNCTION(BlueprintCallable)
    bool IsOnCooldown() const { return CurrentCooldown > 0.0f; }
    
    UFUNCTION(BlueprintCallable)
    float GetCooldownRemaining() const { return CurrentCooldown; }
    
protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
        FActorComponentTickFunction* ThisTickFunction) override;
    
    UFUNCTION(BlueprintImplementableEvent)
    void OnInteractionStarted(AActor* Interactor);
    
    UFUNCTION(BlueprintImplementableEvent)
    void OnInteractionCompleted(AActor* Interactor);
    
    UFUNCTION(BlueprintImplementableEvent)
    void OnInteractionCancelled(AActor* Interactor);
    
    virtual void ExecuteInteraction(AActor* Interactor);
    virtual void UpdateVisualState();
    
private:
    float CurrentCooldown = 0.0f;
    int32 CurrentUses = 0;
    bool bIsEnabled = true;
    bool bIsBeingInteracted = false;
    
    UPROPERTY()
    AActor* CurrentInteractor = nullptr;
    
    float InteractionProgress = 0.0f;
};
```

### 3.2 Explosive Valve Implementation

#### AExplosiveValve
```cpp
UCLASS()
class ATLAS_API AExplosiveValve : public AActor
{
    GENERATED_BODY()

public:
    AExplosiveValve();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* ValveMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UInteractableComponent* InteractableComp;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USphereComponent* ExplosionRadius;
    
    // Explosion configuration
    UPROPERTY(EditAnywhere, Category = "Explosion")
    float ExplosionDamage = 100.0f;
    
    UPROPERTY(EditAnywhere, Category = "Explosion")
    float ExplosionRadius = 500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Explosion")
    float ExplosionForce = 2000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Explosion")
    float ExplosionDelay = 2.0f;
    
    UPROPERTY(EditAnywhere, Category = "Explosion")
    TSubclassOf<UCameraShakeBase> ExplosionShake;
    
    // Effects
    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* ValveTurnEffect;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* SteamBuildupEffect;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* ExplosionEffect;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    USoundCue* ValveTurnSound;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    USoundCue* SteamHissSound;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    USoundCue* ExplosionSound;
    
protected:
    virtual void BeginPlay() override;
    
    UFUNCTION()
    void OnValveInteracted(AActor* Interactor);
    
    UFUNCTION()
    void TriggerExplosion();
    
private:
    void StartExplosionSequence();
    void ApplyExplosionDamage();
    void SpawnExplosionEffects();
    
    FTimerHandle ExplosionTimerHandle;
    bool bIsArmed = false;
};

// Implementation
void AExplosiveValve::OnValveInteracted(AActor* Interactor)
{
    if (bIsArmed) return;
    
    bIsArmed = true;
    
    // Visual sequence
    // 1. Turn valve animation
    ValveMesh->SetRelativeRotation(FRotator(0, 0, 720), true); // 2 full rotations
    UGameplayStatics::SpawnEmitterAttached(ValveTurnEffect, ValveMesh);
    UGameplayStatics::PlaySoundAtLocation(this, ValveTurnSound, GetActorLocation());
    
    // 2. Steam buildup
    GetWorld()->GetTimerManager().SetTimer(
        ExplosionTimerHandle,
        [this]()
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(), 
                SteamBuildupEffect, 
                GetActorLocation()
            );
            UGameplayStatics::PlaySoundAtLocation(
                this, 
                SteamHissSound, 
                GetActorLocation()
            );
            
            // Warning indicator
            ExplosionRadius->SetSphereRadius(ExplosionRadius, true);
            ExplosionRadius->SetVisibility(true);
        },
        0.5f,
        false
    );
    
    // 3. Explosion
    GetWorld()->GetTimerManager().SetTimer(
        ExplosionTimerHandle,
        this,
        &AExplosiveValve::TriggerExplosion,
        ExplosionDelay,
        false
    );
}

void AExplosiveValve::TriggerExplosion()
{
    // Damage
    UGameplayStatics::ApplyRadialDamageWithFalloff(
        this,
        ExplosionDamage,
        ExplosionDamage * 0.25f,
        GetActorLocation(),
        ExplosionRadius * 0.5f,
        ExplosionRadius,
        1.0f,
        UDamageType::StaticClass(),
        TArray<AActor*>(),
        this,
        nullptr
    );
    
    // Force
    TArray<FHitResult> HitResults;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);
    
    GetWorld()->SweepMultiByChannel(
        HitResults,
        GetActorLocation(),
        GetActorLocation() + FVector(0, 0, 1),
        FQuat::Identity,
        ECC_WorldDynamic,
        SphereShape
    );
    
    for (const FHitResult& Hit : HitResults)
    {
        if (UPrimitiveComponent* HitComp = Hit.GetComponent())
        {
            if (HitComp->IsSimulatingPhysics())
            {
                FVector Direction = (Hit.Location - GetActorLocation()).GetSafeNormal();
                HitComp->AddImpulseAtLocation(
                    Direction * ExplosionForce,
                    Hit.Location
                );
            }
        }
    }
    
    // Effects
    SpawnExplosionEffects();
    
    // Destroy valve
    Destroy();
}
```

### 3.3 Gravity Well Implementation

#### AGravityWell
```cpp
UCLASS()
class ATLAS_API AGravityWell : public AActor
{
    GENERATED_BODY()

public:
    AGravityWell();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USphereComponent* GravityField;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UInteractableComponent* InteractableComp;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* WellCore;
    
    // Gravity configuration
    UPROPERTY(EditAnywhere, Category = "Gravity")
    float PullForce = 1000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Gravity")
    float FieldRadius = 800.0f;
    
    UPROPERTY(EditAnywhere, Category = "Gravity")
    float Duration = 10.0f;
    
    UPROPERTY(EditAnywhere, Category = "Gravity")
    bool bAffectsPlayers = true;
    
    UPROPERTY(EditAnywhere, Category = "Gravity")
    bool bAffectsEnemies = true;
    
    UPROPERTY(EditAnywhere, Category = "Gravity")
    bool bAffectsProjectiles = true;
    
    UPROPERTY(EditAnywhere, Category = "Gravity")
    bool bAffectsDebris = true;
    
    // Effects
    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* GravityDistortionEffect;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    UMaterialInterface* DistortionMaterial;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    USoundCue* GravityHumSound;
    
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    UFUNCTION()
    void OnWellActivated(AActor* Interactor);
    
    UFUNCTION()
    void DeactivateWell();
    
private:
    void ApplyGravityToActor(AActor* Actor, float DeltaTime);
    void UpdateVisualEffects(float DeltaTime);
    
    bool bIsActive = false;
    float CurrentDuration = 0.0f;
    
    UPROPERTY()
    TArray<AActor*> AffectedActors;
    
    UPROPERTY()
    UAudioComponent* ActiveHumSound;
    
    UPROPERTY()
    UParticleSystemComponent* ActiveDistortion;
};

void AGravityWell::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsActive) return;
    
    // Update duration
    CurrentDuration += DeltaTime;
    if (CurrentDuration >= Duration)
    {
        DeactivateWell();
        return;
    }
    
    // Find all actors in range
    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(FieldRadius);
    
    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECC_WorldDynamic,
        SphereShape
    );
    
    // Apply gravity to each actor
    for (const FOverlapResult& Overlap : Overlaps)
    {
        if (AActor* Actor = Overlap.GetActor())
        {
            if (ShouldAffectActor(Actor))
            {
                ApplyGravityToActor(Actor, DeltaTime);
            }
        }
    }
    
    // Update visual effects
    UpdateVisualEffects(DeltaTime);
}

void AGravityWell::ApplyGravityToActor(AActor* Actor, float DeltaTime)
{
    FVector ToCenter = GetActorLocation() - Actor->GetActorLocation();
    float Distance = ToCenter.Size();
    
    if (Distance < 10.0f) return; // Too close to center
    
    ToCenter.Normalize();
    
    // Calculate pull strength based on distance
    float DistanceFactor = 1.0f - (Distance / FieldRadius);
    float CurrentPull = PullForce * DistanceFactor * DistanceFactor; // Quadratic falloff
    
    // Apply force
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        // For characters, modify velocity
        FVector CurrentVelocity = Character->GetVelocity();
        FVector NewVelocity = CurrentVelocity + (ToCenter * CurrentPull * DeltaTime);
        Character->GetCharacterMovement()->Velocity = NewVelocity;
        
        // Apply slow effect when being pulled
        Character->GetCharacterMovement()->MaxWalkSpeed *= 0.7f;
    }
    else if (UPrimitiveComponent* PrimComp = Actor->GetRootComponent())
    {
        // For physics objects
        if (PrimComp->IsSimulatingPhysics())
        {
            PrimComp->AddForce(ToCenter * CurrentPull * PrimComp->GetMass());
        }
    }
}
```

### 3.4 Turret Hack Implementation

#### AHackableTurret
```cpp
UCLASS()
class ATLAS_API AHackableTurret : public AActor
{
    GENERATED_BODY()

public:
    AHackableTurret();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* TurretBase;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* TurretBarrel;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UInteractableComponent* HackInterface;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USphereComponent* DetectionRange;
    
    // Turret configuration
    UPROPERTY(EditAnywhere, Category = "Turret")
    float FireRate = 2.0f; // Shots per second
    
    UPROPERTY(EditAnywhere, Category = "Turret")
    float ProjectileDamage = 15.0f;
    
    UPROPERTY(EditAnywhere, Category = "Turret")
    float ProjectileSpeed = 2000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Turret")
    float DetectionRadius = 1500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Turret")
    float TrackingSpeed = 90.0f; // Degrees per second
    
    UPROPERTY(EditAnywhere, Category = "Turret")
    float HackedDuration = 20.0f;
    
    // Projectile
    UPROPERTY(EditAnywhere, Category = "Turret")
    TSubclassOf<AProjectileBase> ProjectileClass;
    
    // Effects
    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* MuzzleFlash;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* HackEffect;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    USoundCue* FireSound;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    USoundCue* HackingSound;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    UMaterialInterface* NormalMaterial;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    UMaterialInterface* HackedMaterial;
    
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    UFUNCTION()
    void OnTurretHacked(AActor* Hacker);
    
    UFUNCTION()
    void OnHackExpired();
    
private:
    void UpdateTargeting(float DeltaTime);
    void FireAtTarget();
    bool IsValidTarget(AActor* Actor) const;
    void SetTurretFaction(bool bFriendly);
    
    UPROPERTY()
    AActor* CurrentTarget = nullptr;
    
    FTimerHandle FireTimerHandle;
    FTimerHandle HackTimerHandle;
    
    bool bIsHacked = false;
    bool bIsFriendly = false;
    float LastFireTime = 0.0f;
    
    FRotator DesiredRotation;
};

void AHackableTurret::OnTurretHacked(AActor* Hacker)
{
    if (bIsHacked) return;
    
    bIsHacked = true;
    bIsFriendly = true;
    
    // Visual feedback
    TurretBase->SetMaterial(0, HackedMaterial);
    TurretBarrel->SetMaterial(0, HackedMaterial);
    
    // Spawn hack effect
    UGameplayStatics::SpawnEmitterAttached(
        HackEffect,
        TurretBase,
        NAME_None,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::SnapToTarget,
        true
    );
    
    // Play sound
    UGameplayStatics::PlaySoundAtLocation(
        this,
        HackingSound,
        GetActorLocation()
    );
    
    // Clear current target
    CurrentTarget = nullptr;
    
    // Set hack duration timer
    GetWorld()->GetTimerManager().SetTimer(
        HackTimerHandle,
        this,
        &AHackableTurret::OnHackExpired,
        HackedDuration,
        false
    );
    
    // Notify player
    if (APlayerController* PC = Cast<APlayerController>(Hacker->GetInstigatorController()))
    {
        // Show hack success message
        ShowHackSuccessNotification(PC);
    }
}

void AHackableTurret::UpdateTargeting(float DeltaTime)
{
    // Find best target
    AActor* BestTarget = nullptr;
    float BestScore = -1.0f;
    
    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(DetectionRadius);
    
    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        SphereShape
    );
    
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* PotentialTarget = Overlap.GetActor();
        
        if (!IsValidTarget(PotentialTarget)) continue;
        
        // Score based on distance and angle
        float Distance = FVector::Dist(GetActorLocation(), PotentialTarget->GetActorLocation());
        FVector ToTarget = (PotentialTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        float Angle = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), ToTarget));
        
        float Score = (1.0f - (Distance / DetectionRadius)) * (1.0f - (Angle / PI));
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = PotentialTarget;
        }
    }
    
    CurrentTarget = BestTarget;
    
    // Rotate towards target
    if (CurrentTarget)
    {
        FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
        DesiredRotation = ToTarget.Rotation();
        
        // Smooth rotation
        FRotator CurrentRotation = TurretBarrel->GetComponentRotation();
        FRotator NewRotation = FMath::RInterpTo(
            CurrentRotation,
            DesiredRotation,
            DeltaTime,
            TrackingSpeed
        );
        
        TurretBarrel->SetWorldRotation(NewRotation);
        
        // Check if aimed at target
        float AimError = FMath::Abs(NewRotation.Yaw - DesiredRotation.Yaw);
        if (AimError < 5.0f && GetWorld()->GetTimeSeconds() - LastFireTime > (1.0f / FireRate))
        {
            FireAtTarget();
            LastFireTime = GetWorld()->GetTimeSeconds();
        }
    }
}
```

---

## Part 4: Destructible Environment System

### 4.1 Destructible Component

#### UDestructibleEnvironmentComponent
```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UDestructibleEnvironmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Destruction configuration
    UPROPERTY(EditAnywhere, Category = "Destruction")
    float MaxHealth = 100.0f;
    
    UPROPERTY(EditAnywhere, Category = "Destruction")
    float ExplosionDamageThreshold = 50.0f;
    
    UPROPERTY(EditAnywhere, Category = "Destruction")
    bool bAffectsStationIntegrity = true;
    
    UPROPERTY(EditAnywhere, Category = "Destruction")
    float IntegrityDamage = 5.0f;
    
    // Debris configuration
    UPROPERTY(EditAnywhere, Category = "Debris")
    TArray<UStaticMesh*> DebrisMeshes;
    
    UPROPERTY(EditAnywhere, Category = "Debris")
    int32 MinDebrisCount = 3;
    
    UPROPERTY(EditAnywhere, Category = "Debris")
    int32 MaxDebrisCount = 8;
    
    UPROPERTY(EditAnywhere, Category = "Debris")
    float DebrisLifetime = 30.0f;
    
    UPROPERTY(EditAnywhere, Category = "Debris")
    float DebrisImpulseMin = 100.0f;
    
    UPROPERTY(EditAnywhere, Category = "Debris")
    float DebrisImpulseMax = 500.0f;
    
    // Effects
    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* DestructionEffect;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    USoundCue* DestructionSound;
    
    UPROPERTY(EditAnywhere, Category = "Effects")
    TSubclassOf<UCameraShakeBase> DestructionShake;
    
    // Damage handling
    UFUNCTION(BlueprintCallable)
    void ApplyDamage(float Damage, const FVector& ImpactPoint, const FVector& ImpactNormal);
    
    UFUNCTION(BlueprintCallable)
    void DestroyEnvironment(const FVector& ImpactPoint, const FVector& ImpulseDirection);
    
protected:
    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintImplementableEvent)
    void OnEnvironmentDestroyed();
    
    UFUNCTION(BlueprintImplementableEvent)
    void OnEnvironmentDamaged(float DamageAmount, float HealthPercent);
    
private:
    void SpawnDebris(const FVector& Origin, const FVector& ImpulseDirection);
    void CreateDestructionEffects(const FVector& Location);
    void DamageStationIntegrity();
    
    float CurrentHealth;
    bool bIsDestroyed = false;
};

void UDestructibleEnvironmentComponent::DestroyEnvironment(const FVector& ImpactPoint, const FVector& ImpulseDirection)
{
    if (bIsDestroyed) return;
    
    bIsDestroyed = true;
    
    // Spawn debris
    SpawnDebris(ImpactPoint, ImpulseDirection);
    
    // Create effects
    CreateDestructionEffects(ImpactPoint);
    
    // Damage station
    if (bAffectsStationIntegrity)
    {
        DamageStationIntegrity();
    }
    
    // Hide original mesh
    if (AActor* Owner = GetOwner())
    {
        if (UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>())
        {
            MeshComp->SetVisibility(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
    
    // Notify Blueprint
    OnEnvironmentDestroyed();
}

void UDestructibleEnvironmentComponent::SpawnDebris(const FVector& Origin, const FVector& ImpulseDirection)
{
    int32 DebrisCount = FMath::RandRange(MinDebrisCount, MaxDebrisCount);
    
    for (int32 i = 0; i < DebrisCount; ++i)
    {
        // Select random debris mesh
        if (DebrisMeshes.Num() == 0) continue;
        UStaticMesh* DebrisMesh = DebrisMeshes[FMath::RandRange(0, DebrisMeshes.Num() - 1)];
        
        // Spawn debris actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = 
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AStaticMeshActor* Debris = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Origin + FMath::VRand() * 50.0f,
            FMath::RandRotation(),
            SpawnParams
        );
        
        if (Debris)
        {
            Debris->GetStaticMeshComponent()->SetStaticMesh(DebrisMesh);
            Debris->GetStaticMeshComponent()->SetSimulatePhysics(true);
            
            // Apply random impulse
            float ImpulseStrength = FMath::RandRange(DebrisImpulseMin, DebrisImpulseMax);
            FVector RandomDirection = ImpulseDirection + FMath::VRand() * 0.5f;
            RandomDirection.Normalize();
            
            Debris->GetStaticMeshComponent()->AddImpulse(RandomDirection * ImpulseStrength);
            
            // Add angular velocity
            FVector AngularImpulse = FMath::VRand() * 500.0f;
            Debris->GetStaticMeshComponent()->AddAngularImpulseInRadians(AngularImpulse);
            
            // Set lifetime
            Debris->SetLifeSpan(DebrisLifetime);
        }
    }
}
```

---

## Part 5: Station Degradation System

### 5.1 Station Degradation Manager

#### UStationDegradationSubsystem
```cpp
UCLASS()
class ATLAS_API UStationDegradationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Degradation configuration
    UPROPERTY(EditAnywhere, Category = "Degradation")
    float DegradationRate = 0.5f; // Integrity loss per minute
    
    UPROPERTY(EditAnywhere, Category = "Degradation")
    float CombatDegradationMultiplier = 2.0f;
    
    UPROPERTY(EditAnywhere, Category = "Degradation")
    float RoomCompletionBonus = 10.0f; // Integrity restored on room clear
    
    // Degradation events
    UPROPERTY(EditAnywhere, Category = "Events")
    float LightFlickerThreshold = 75.0f;
    
    UPROPERTY(EditAnywhere, Category = "Events")
    float SystemFailureThreshold = 50.0f;
    
    UPROPERTY(EditAnywhere, Category = "Events")
    float CriticalFailureThreshold = 25.0f;
    
    // Start degradation
    UFUNCTION(BlueprintCallable)
    void StartDegradation();
    
    UFUNCTION(BlueprintCallable)
    void StopDegradation();
    
    UFUNCTION(BlueprintCallable)
    void SetCombatActive(bool bInCombat);
    
    // Event triggers
    UFUNCTION(BlueprintCallable)
    void TriggerRandomEvent();
    
protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
private:
    void UpdateDegradation();
    void CheckDegradationEvents(float CurrentIntegrity);
    void TriggerLightFlicker();
    void TriggerSystemFailure();
    void TriggerCriticalFailure();
    
    FTimerHandle DegradationTimerHandle;
    bool bIsInCombat = false;
    float LastEventIntegrity = 100.0f;
    
    // Random events
    void TriggerPowerSurge();
    void TriggerGravityFluctuation();
    void TriggerAtmosphericLeak();
    void TriggerEmergencyLockdown();
};

void UStationDegradationSubsystem::UpdateDegradation()
{
    UStationIntegrityComponent* IntegrityComp = GetStationIntegrityComponent();
    if (!IntegrityComp) return;
    
    // Calculate degradation
    float DegradationAmount = DegradationRate / 60.0f; // Convert to per-second
    if (bIsInCombat)
    {
        DegradationAmount *= CombatDegradationMultiplier;
    }
    
    // Apply degradation
    IntegrityComp->TakeDamage(DegradationAmount);
    
    // Check for events
    float CurrentIntegrity = IntegrityComp->GetIntegrityPercent() * 100.0f;
    CheckDegradationEvents(CurrentIntegrity);
    
    // Random event chance based on integrity
    float EventChance = (100.0f - CurrentIntegrity) / 100.0f * 0.1f; // Up to 10% chance at 0 integrity
    if (FMath::FRand() < EventChance)
    {
        TriggerRandomEvent();
    }
}

void UStationDegradationSubsystem::TriggerRandomEvent()
{
    // Select random event based on current integrity
    TArray<TFunction<void()>> PossibleEvents;
    
    float Integrity = GetStationIntegrityPercent() * 100.0f;
    
    // Always possible
    PossibleEvents.Add([this]() { TriggerPowerSurge(); });
    PossibleEvents.Add([this]() { TriggerGravityFluctuation(); });
    
    // Below 75%
    if (Integrity < 75.0f)
    {
        PossibleEvents.Add([this]() { TriggerAtmosphericLeak(); });
    }
    
    // Below 50%
    if (Integrity < 50.0f)
    {
        PossibleEvents.Add([this]() { TriggerEmergencyLockdown(); });
    }
    
    // Execute random event
    if (PossibleEvents.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, PossibleEvents.Num() - 1);
        PossibleEvents[RandomIndex]();
    }
}
```

---

## Part 6: Emergency Event System

### 6.1 Emergency Event Manager

#### UEmergencyEventManager
```cpp
UCLASS()
class ATLAS_API UEmergencyEventManager : public UObject
{
    GENERATED_BODY()

public:
    // Event types
    UENUM(BlueprintType)
    enum class EEmergencyEventType : uint8
    {
        PowerFailure,
        EmergencyLockdown,
        OxygenLeak,
        ReactorOverload,
        SecurityBreach,
        SystemMalfunction
    };
    
    // Event data
    USTRUCT(BlueprintType)
    struct FEmergencyEventData
    {
        GENERATED_BODY()
        
        UPROPERTY(EditAnywhere)
        EEmergencyEventType EventType;
        
        UPROPERTY(EditAnywhere)
        float Duration = 30.0f;
        
        UPROPERTY(EditAnywhere)
        float Severity = 1.0f;
        
        UPROPERTY(EditAnywhere)
        FText EventName;
        
        UPROPERTY(EditAnywhere)
        FText EventDescription;
        
        UPROPERTY(EditAnywhere)
        USoundCue* WarningSound;
        
        UPROPERTY(EditAnywhere)
        UParticleSystem* EventEffect;
    };
    
    // Trigger emergency event
    UFUNCTION(BlueprintCallable)
    void TriggerEmergencyEvent(EEmergencyEventType EventType, float Severity = 1.0f);
    
    UFUNCTION(BlueprintCallable)
    void EndCurrentEvent();
    
    // Event handlers
    void HandlePowerFailure(float Severity);
    void HandleEmergencyLockdown(float Severity);
    void HandleOxygenLeak(float Severity);
    void HandleReactorOverload(float Severity);
    void HandleSecurityBreach(float Severity);
    void HandleSystemMalfunction(float Severity);
    
private:
    FEmergencyEventData CurrentEvent;
    bool bEventActive = false;
    FTimerHandle EventTimerHandle;
};

void UEmergencyEventManager::HandlePowerFailure(float Severity)
{
    // Disable lights
    TArray<AActor*> Lights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALight::StaticClass(), Lights);
    
    for (AActor* Light : Lights)
    {
        if (ALight* LightActor = Cast<ALight>(Light))
        {
            // Randomly disable lights based on severity
            if (FMath::FRand() < Severity)
            {
                LightActor->SetEnabled(false);
            }
            else
            {
                // Flicker remaining lights
                LightActor->SetBrightness(FMath::RandRange(0.1f, 0.5f));
            }
        }
    }
    
    // Enable emergency lighting
    EnableEmergencyLighting();
    
    // Disable certain abilities
    DisableElectricalAbilities();
    
    // Play power down sound
    UGameplayStatics::PlaySound2D(GetWorld(), PowerDownSound);
    
    // Screen notification
    ShowEmergencyNotification("POWER FAILURE", "Emergency power only. Systems limited.");
}

void UEmergencyEventManager::HandleEmergencyLockdown(float Severity)
{
    // Close and lock doors
    TArray<AActor*> Doors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), "Door", Doors);
    
    for (AActor* Door : Doors)
    {
        if (IDoor* DoorInterface = Cast<IDoor>(Door))
        {
            DoorInterface->LockDoor();
            DoorInterface->CloseDoor();
        }
    }
    
    // Activate security systems
    ActivateSecurityTurrets();
    
    // Start countdown timer
    float LockdownDuration = 30.0f * Severity;
    StartLockdownTimer(LockdownDuration);
    
    // Play alarm
    UGameplayStatics::PlaySound2D(GetWorld(), LockdownAlarm);
    
    // Screen notification
    ShowEmergencyNotification("EMERGENCY LOCKDOWN", 
        FString::Printf(TEXT("Lockdown in effect for %.0f seconds"), LockdownDuration));
}
```

---

## Part 7: Ambient Atmosphere System

### 7.1 Ambient Sound Manager

#### UAmbientSoundManager
```cpp
UCLASS()
class ATLAS_API UAmbientSoundManager : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sound categories
    UPROPERTY(EditAnywhere, Category = "Ambient")
    USoundCue* BaseAmbientLoop;
    
    UPROPERTY(EditAnywhere, Category = "Ambient")
    TArray<USoundCue*> RandomCreaks;
    
    UPROPERTY(EditAnywhere, Category = "Ambient")
    TArray<USoundCue*> ElectricalSounds;
    
    UPROPERTY(EditAnywhere, Category = "Ambient")
    TArray<USoundCue*> MetallicGroans;
    
    UPROPERTY(EditAnywhere, Category = "Ambient")
    TArray<USoundCue*> DistantExplosions;
    
    UPROPERTY(EditAnywhere, Category = "Ambient")
    TArray<USoundCue*> AlarmSounds;
    
    // Configuration
    UPROPERTY(EditAnywhere, Category = "Config")
    float MinTimeBetweenSounds = 5.0f;
    
    UPROPERTY(EditAnywhere, Category = "Config")
    float MaxTimeBetweenSounds = 20.0f;
    
    UPROPERTY(EditAnywhere, Category = "Config")
    float IntegrityVolumeMultiplier = 2.0f;
    
    // Update ambient based on integrity
    UFUNCTION(BlueprintCallable)
    void UpdateAmbientForIntegrity(float IntegrityPercent);
    
    UFUNCTION(BlueprintCallable)
    void PlayRandomAmbientSound();
    
protected:
    virtual void BeginPlay() override;
    
private:
    void ScheduleNextAmbientSound();
    USoundCue* SelectRandomSound(float IntegrityPercent);
    
    UPROPERTY()
    UAudioComponent* BaseAmbientComponent;
    
    FTimerHandle AmbientTimerHandle;
    float CurrentIntegrityPercent = 1.0f;
};

USoundCue* UAmbientSoundManager::SelectRandomSound(float IntegrityPercent)
{
    TArray<USoundCue*> PossibleSounds;
    
    // Always include creaks
    PossibleSounds.Append(RandomCreaks);
    
    // Add more sounds based on integrity
    if (IntegrityPercent < 0.75f)
    {
        PossibleSounds.Append(ElectricalSounds);
        PossibleSounds.Append(MetallicGroans);
    }
    
    if (IntegrityPercent < 0.5f)
    {
        PossibleSounds.Append(DistantExplosions);
    }
    
    if (IntegrityPercent < 0.25f)
    {
        PossibleSounds.Append(AlarmSounds);
    }
    
    if (PossibleSounds.Num() > 0)
    {
        return PossibleSounds[FMath::RandRange(0, PossibleSounds.Num() - 1)];
    }
    
    return nullptr;
}
```

---

## Implementation Checklist

### Core Systems
- [x] UIntegrityVisualizerComponent implementation
- [x] Visual state handlers (Critical, Warning, Minor)
- [x] Hull breach system with vacuum effects
- [x] Dynamic damage manifestation

### Environmental Hazards
- [x] Base UEnvironmentalHazardComponent
- [x] Electrical surge hazard with chain lightning
- [x] Toxic leak hazard with DOT and vision impairment
- [x] Low gravity hazard with movement changes
- [ ] System malfunction hazard (deferred to polish phase)

### Interactables
- [x] Base UInteractableComponent framework
- [ ] Explosive valve with timed detonation (Blueprint implementation needed)
- [ ] Gravity well with physics manipulation (Blueprint implementation needed)
- [ ] Hackable turret with faction switching (Blueprint implementation needed)
- [ ] Emergency vent with knockback (Blueprint implementation needed)
- [ ] Power surge panel with AOE stun (Blueprint implementation needed)

### Destructible Environment
- [x] UDestructibleEnvironmentComponent
- [x] Debris spawning system
- [x] Destruction effects and sounds
- [x] Station integrity integration

### Station Degradation
- [x] UStationDegradationSubsystem
- [x] Time-based degradation
- [x] Combat degradation multiplier
- [x] Random event triggers

### Emergency Events
- [x] UEmergencyEventManager
- [x] Power failure implementation
- [x] Emergency lockdown system
- [x] Oxygen leak mechanics
- [x] Reactor overload sequence
- [x] Security breach protocol

### Ambient Atmosphere
- [ ] UAmbientSoundManager (deferred - requires audio assets)
- [ ] Dynamic sound selection based on integrity (deferred - requires audio assets)
- [ ] Randomized ambient effects (deferred - requires audio assets)
- [ ] Environmental audio layers (deferred - requires audio assets)

---

## Testing Procedures

### Environmental Hazard Testing
```cpp
// Console commands for testing
Atlas.Phase4.SpawnHazard [Type] [Location]
Atlas.Phase4.TriggerHazard [HazardID]
Atlas.Phase4.SetHazardDamage [HazardID] [Damage]
Atlas.Phase4.TestAllHazards
```

### Interactable Testing
```cpp
Atlas.Phase4.SpawnInteractable [Type] [Location]
Atlas.Phase4.TriggerInteractable [ID]
Atlas.Phase4.SetInteractableCooldown [ID] [Cooldown]
Atlas.Phase4.TestInteractableChain
```

### Degradation Testing
```cpp
Atlas.Phase4.SetDegradationRate [Rate]
Atlas.Phase4.TriggerRandomEvent
Atlas.Phase4.SimulateDegradation [Minutes]
Atlas.Phase4.TestEmergencyEvent [Type]
```

### Performance Testing
```cpp
Atlas.Phase4.SpawnMaxHazards
Atlas.Phase4.StressTestParticles
Atlas.Phase4.MeasurePerformance
Atlas.Phase4.ProfilePhysics
```

---

## Performance Optimization Guidelines

### Particle System Optimization
1. Use GPU particles for large-scale effects
2. LOD settings for distance-based quality
3. Pool particle systems for reuse
4. Limit concurrent effects based on platform

### Physics Optimization
1. Use simplified collision for debris
2. Sleep physics bodies when stationary
3. Limit active rigid bodies count
4. Use distance-based physics LOD

### Audio Optimization
1. Limit concurrent sound sources
2. Use audio occlusion for distant sounds
3. Pool audio components
4. Compress ambient loops

### Network Optimization
1. Replicate only essential hazard data
2. Use reliable multicast for critical events
3. Client-side prediction for interactables
4. Compress position updates for debris

---

## Success Criteria

Phase 4 is complete when:
1. ✅ Station visually degrades based on integrity (hull breaches, sparks, alarms)
2. ✅ 5+ environmental hazards functional (electrical, toxic, gravity, etc.)
3. ✅ 5 interactable types implemented and balanced
4. ✅ Destructible environments react to combat
5. ✅ Station degradation creates tension over time
6. ✅ Emergency events trigger dynamically
7. ✅ Ambient atmosphere enhances immersion
8. ✅ Performance remains above 60 FPS with all systems active
9. ✅ Network replication works for multiplayer
10. ✅ All systems integrate smoothly with existing gameplay

---

## Documentation Requirements

### Code Documentation
- Full header comments for all classes
- Parameter documentation for public functions
- Usage examples in implementation files
- Performance notes for expensive operations

### Blueprint Documentation
- Clear naming conventions (BP_Hazard_[Type])
- Organized folder structure
- Example blueprints for each system
- Visual scripting best practices guide

### Design Documentation
- Hazard balance spreadsheet
- Interactable cooldown matrix
- Event probability tables
- Atmosphere intensity curves

---

## Next Phase Preview
**Phase 5: Polish, Optimization & Balancing**
- Performance profiling and optimization
- Final visual effects and post-processing
- Audio mastering and mixing
- Gameplay balancing based on playtesting
- Bug fixing and stability improvements
- Achievement system implementation
- Steam integration and cloud saves