#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CombatComponent.generated.h"

class UAttackDataAsset;
class UCombatRulesDataAsset;
class UDamageCalculator;
class UHealthComponent;
class UVulnerabilityComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackStarted, const FGameplayTag&, AttackTag, const UAttackDataAsset*, AttackData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlockStarted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBlockEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVulnerabilityApplied);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ATLAS_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    UCombatRulesDataAsset* CombatRules;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TMap<FGameplayTag, UAttackDataAsset*> AttackDataMap;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FGameplayTagContainer CombatStateTags;


    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    int32 VulnerabilityCharges = 0;  // Deprecated - use VulnerabilityComponent instead

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool StartAttack(const FGameplayTag& AttackTag);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool StartBlock();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndBlock();


    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyVulnerability(int32 Charges = 1);  // Deprecated - use VulnerabilityComponent

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ConsumeVulnerabilityCharge();  // Deprecated - use VulnerabilityComponent

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyVulnerabilityWithIFrames(int32 Charges = 1, bool bGrantIFrames = false);


    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAttacking() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsBlocking() const;


    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsVulnerable() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool HasIFrames() const;


    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInCombat() const;
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetTimeSinceLastCombatAction() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ProcessHit(AActor* HitActor, const FGameplayTag& AttackTag);

    void ProcessHitFromAnimation(class AGameCharacterBase* HitCharacter);


    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartBlocking();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StopBlocking();



    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DealDamageToTarget(AActor* Target, float Damage, const FGameplayTagContainer& AttackTags);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    class UAnimMontage* GetAttackMontage(const FGameplayTag& AttackTag) const;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnAttackStarted OnAttackStarted;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnAttackEnded OnAttackEnded;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnBlockStarted OnBlockStarted;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnBlockEnded OnBlockEnded;


    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnVulnerabilityApplied OnVulnerabilityApplied;

private:
    UPROPERTY()
    UAttackDataAsset* CurrentAttackData;

    UPROPERTY()
    UDamageCalculator* DamageCalculator;

    UPROPERTY()
    UVulnerabilityComponent* VulnerabilityComponent;

    FTimerHandle VulnerabilityTimerHandle;
    float LastCombatActionTime = 0.0f;

    void EndVulnerability();  // Deprecated - handled by VulnerabilityComponent

    void AddCombatStateTag(const FGameplayTag& Tag);
    void RemoveCombatStateTag(const FGameplayTag& Tag);
    bool HasCombatStateTag(const FGameplayTag& Tag) const;
};