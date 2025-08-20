#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ActionDataAsset.generated.h"

class UBaseAction;
class UAbilityBase;

UENUM(BlueprintType)
enum class EActionType : uint8
{
    Movement        UMETA(DisplayName = "Movement (Dash, etc)"),
    Defense         UMETA(DisplayName = "Defense (Block, etc)"),
    MeleeAttack     UMETA(DisplayName = "Melee Attack"),
    RangedAttack    UMETA(DisplayName = "Ranged Attack"),
    AreaEffect      UMETA(DisplayName = "Area Effect"),
    Utility         UMETA(DisplayName = "Utility"),
    Special         UMETA(DisplayName = "Special")
};

/**
 * Data asset containing configuration for player actions.
 * Fields are shown/hidden based on ActionType selection.
 */
UCLASS(BlueprintType)
class ATLAS_API UActionDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // ========================================
    // CORE IDENTITY (Always Visible)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1. Identity", meta = (Priority = 1))
    FGameplayTag ActionTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1. Identity", meta = (Priority = 2))
    EActionType ActionType = EActionType::Utility;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1. Identity", meta = (Priority = 3))
    TSubclassOf<UBaseAction> ActionClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1. Identity", meta = (Priority = 4))
    FText ActionName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "1. Identity", meta = (MultiLine = true, Priority = 5))
    FText Description;

    // ========================================
    // UNIVERSAL STATS (Always Visible)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "2. Universal", meta = (ClampMin = 0.0, Priority = 10))
    float Cooldown = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "2. Universal", meta = (ClampMin = 0.0, Priority = 11))
    float IntegrityCost = 0.0f;

    // ========================================
    // MOVEMENT CONFIG (Visible when ActionType = Movement)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "3. Movement", 
        meta = (EditCondition = "ActionType == EActionType::Movement", EditConditionHides, ClampMin = 0.0))
    float DashDistance = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "3. Movement",
        meta = (EditCondition = "ActionType == EActionType::Movement", EditConditionHides, ClampMin = 0.0))
    float DashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "3. Movement",
        meta = (EditCondition = "ActionType == EActionType::Movement", EditConditionHides))
    bool bGrantsInvincibility = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "3. Movement",
        meta = (EditCondition = "ActionType == EActionType::Movement && bGrantsInvincibility", EditConditionHides, ClampMin = 0.0))
    float InvincibilityDuration = 0.3f;

    // ========================================
    // DEFENSE CONFIG (Visible when ActionType = Defense)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "4. Defense",
        meta = (EditCondition = "ActionType == EActionType::Defense", EditConditionHides, ClampMin = 0.0, ClampMax = 1.0))
    float DamageReduction = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "4. Defense",
        meta = (EditCondition = "ActionType == EActionType::Defense", EditConditionHides))
    bool bCanBlockUnblockables = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "4. Defense",
        meta = (EditCondition = "ActionType == EActionType::Defense", EditConditionHides))
    bool bRequiresHoldInput = true;

    // ========================================
    // MELEE ATTACK CONFIG (Visible when ActionType = MeleeAttack)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "5. Melee Attack",
        meta = (EditCondition = "ActionType == EActionType::MeleeAttack", EditConditionHides, ClampMin = 0.0))
    float MeleeDamage = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "5. Melee Attack",
        meta = (EditCondition = "ActionType == EActionType::MeleeAttack", EditConditionHides, ClampMin = 0.0))
    float KnockbackForce = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "5. Melee Attack",
        meta = (EditCondition = "ActionType == EActionType::MeleeAttack", EditConditionHides, ClampMin = 0.0))
    float PoiseDamage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "5. Melee Attack",
        meta = (EditCondition = "ActionType == EActionType::MeleeAttack", EditConditionHides))
    bool bCausesRagdoll = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "5. Melee Attack",
        meta = (EditCondition = "ActionType == EActionType::MeleeAttack", EditConditionHides))
    bool bIsUnblockable = false;

    // ========================================
    // RANGED ATTACK CONFIG (Visible when ActionType = RangedAttack)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "6. Ranged Attack",
        meta = (EditCondition = "ActionType == EActionType::RangedAttack", EditConditionHides, ClampMin = 0.0))
    float ProjectileSpeed = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "6. Ranged Attack",
        meta = (EditCondition = "ActionType == EActionType::RangedAttack", EditConditionHides, ClampMin = 0.0))
    float ProjectileDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "6. Ranged Attack",
        meta = (EditCondition = "ActionType == EActionType::RangedAttack", EditConditionHides, ClampMin = 0.0))
    float ProjectileRange = 1500.0f;

    // ========================================
    // AREA EFFECT CONFIG (Visible when ActionType = AreaEffect)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "7. Area Effect",
        meta = (EditCondition = "ActionType == EActionType::AreaEffect", EditConditionHides, ClampMin = 0.0))
    float EffectRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "7. Area Effect",
        meta = (EditCondition = "ActionType == EActionType::AreaEffect", EditConditionHides, ClampMin = 0.0))
    float EffectDamage = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "7. Area Effect",
        meta = (EditCondition = "ActionType == EActionType::AreaEffect", EditConditionHides, ClampMin = 0.0))
    float EffectDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "7. Area Effect",
        meta = (EditCondition = "ActionType == EActionType::AreaEffect", EditConditionHides))
    bool bAffectsAllies = false;

    // ========================================
    // UTILITY CONFIG (Visible when ActionType = Utility)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "8. Utility",
        meta = (EditCondition = "ActionType == EActionType::Utility", EditConditionHides, ClampMin = 0.0))
    float InteractionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "8. Utility",
        meta = (EditCondition = "ActionType == EActionType::Utility", EditConditionHides, ClampMin = 0.0))
    float ChargeTime = 0.0f;

    // ========================================
    // SPECIAL CONFIG (Visible when ActionType = Special)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "9. Special",
        meta = (EditCondition = "ActionType == EActionType::Special", EditConditionHides))
    TSubclassOf<UAbilityBase> AbilityClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "9. Special",
        meta = (EditCondition = "ActionType == EActionType::Special", EditConditionHides))
    TMap<FName, float> CustomFloatParameters;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "9. Special",
        meta = (EditCondition = "ActionType == EActionType::Special", EditConditionHides))
    TMap<FName, FString> CustomStringParameters;

    // ========================================
    // REQUIREMENTS (Always Visible)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "10. Requirements")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "10. Requirements")
    FGameplayTagContainer BlockedDuringTags;

    // ========================================
    // ANIMATION (Optional)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "11. Animation")
    class UAnimMontage* ActionMontage;

    // ========================================
    // VFX & AUDIO (Optional - for later)
    // ========================================
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "12. Effects")
    class UParticleSystem* ActivationEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "12. Effects")
    class USoundBase* ActivationSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "12. Effects")
    UTexture2D* Icon;

public:
    // Helper functions
    UFUNCTION(BlueprintPure, Category = "Action Data")
    bool IsMovementAction() const { return ActionType == EActionType::Movement; }

    UFUNCTION(BlueprintPure, Category = "Action Data")
    bool IsDefenseAction() const { return ActionType == EActionType::Defense; }

    UFUNCTION(BlueprintPure, Category = "Action Data")
    bool IsMeleeAttack() const { return ActionType == EActionType::MeleeAttack; }

    UFUNCTION(BlueprintPure, Category = "Action Data")
    bool IsRangedAttack() const { return ActionType == EActionType::RangedAttack; }

    UFUNCTION(BlueprintPure, Category = "Action Data")
    bool IsAreaEffect() const { return ActionType == EActionType::AreaEffect; }

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};