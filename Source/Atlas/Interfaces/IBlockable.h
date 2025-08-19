#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "IBlockable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UBlockable : public UInterface
{
    GENERATED_BODY()
};

class ATLAS_API IBlockable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Block")
    bool StartBlock();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Block")
    void EndBlock();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Block")
    bool IsBlocking() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Block")
    float GetBlockDamageReduction() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Block")
    bool CanBlock() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Block")
    void OnBlockedAttack(const FGameplayTag& AttackTag, float DamageBlocked, AActor* Attacker);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Block")
    float GetBlockStability() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Block")
    void ConsumeBlockStability(float Amount);
};