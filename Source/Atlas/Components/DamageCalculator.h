#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "DamageCalculator.generated.h"

class UAttackDataAsset;
class UCombatRulesDataAsset;
class UHealthComponent;
class UCombatComponent;

USTRUCT(BlueprintType)
struct FDamageInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    AActor* Attacker = nullptr;

    UPROPERTY(BlueprintReadOnly)
    AActor* Target = nullptr;

    UPROPERTY(BlueprintReadOnly)
    float BaseDamage = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float FinalDamage = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float Knockback = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bCausedRagdoll = false;

    UPROPERTY(BlueprintReadOnly)
    bool bWasBlocked = false;

    UPROPERTY(BlueprintReadOnly)
    bool bWasParried = false;

    UPROPERTY(BlueprintReadOnly)
    bool bWasVulnerable = false;

    UPROPERTY(BlueprintReadOnly)
    FGameplayTagContainer AttackTags;

    FDamageInfo() {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageCalculated, const FDamageInfo&, DamageInfo);

UCLASS(BlueprintType)
class ATLAS_API UDamageCalculator : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Damage")
    FOnDamageCalculated OnDamageCalculated;
    
    // Knockback Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float KnockbackUpwardForce = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback", meta = (ClampMin = "1.0", ClampMax = "5.0"))
    float RagdollKnockbackMultiplier = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knockback")
    float StandardKnockbackMultiplier = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Damage")
    FDamageInfo ProcessDamage(
        AActor* Attacker,
        AActor* Target,
        const UAttackDataAsset* AttackData,
        const FGameplayTagContainer& AttackerTags,
        const FGameplayTagContainer& DefenderTags
    );

    UFUNCTION(BlueprintCallable, Category = "Damage")
    float CalculateFinalDamage(
        float BaseDamage,
        const UCombatRulesDataAsset* CombatRules,
        const FGameplayTagContainer& AttackerTags,
        const FGameplayTagContainer& DefenderTags,
        bool& bWasBlocked,
        bool& bWasVulnerable
    );

    UFUNCTION(BlueprintCallable, Category = "Damage")
    void ApplyDamageToTarget(AActor* Target, const FDamageInfo& DamageInfo);

    UFUNCTION(BlueprintCallable, Category = "Damage")
    void ApplyKnockback(AActor* Target, AActor* Attacker, float KnockbackForce, bool bCauseRagdoll);

private:
    void LogDamageInfo(const FDamageInfo& DamageInfo);
    
    UPROPERTY()
    class UWallImpactComponent* WallImpactComponent;
};