#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AttackDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FAttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    FName AttackName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    float BaseDamage = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    float Knockback = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    float StaggerDamage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (Categories = "Attack"))
    FGameplayTagContainer AttackTags;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    bool bCausesRagdoll = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    float AttackAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    class UAnimMontage* AttackMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cooldown")
    float Cooldown = 0.0f;

    FAttackData()
    {
    }
};

UCLASS(BlueprintType)
class ATLAS_API UAttackDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Data")
    FAttackData AttackData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Data")
    FGameplayTag AttackTypeTag;
};