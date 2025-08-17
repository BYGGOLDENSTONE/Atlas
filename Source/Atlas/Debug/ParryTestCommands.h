#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "ParryTestCommands.generated.h"

UCLASS()
class ATLAS_API UParryTestCommands : public UCheatManager
{
    GENERATED_BODY()

public:
    UFUNCTION(Exec, Category = "Parry Testing")
    void TestApplyVulnerability(int32 Charges = 1);

    UFUNCTION(Exec, Category = "Parry Testing")
    void TestParryWindow();

    UFUNCTION(Exec, Category = "Parry Testing")
    void TestIFrames(float Duration = 0.5f);

    UFUNCTION(Exec, Category = "Parry Testing")
    void TestParrySuccess();

    UFUNCTION(Exec, Category = "Parry Testing")
    void ShowParryInfo();

    UFUNCTION(Exec, Category = "Parry Testing")
    void SetVulnerabilityMultiplier(float Multiplier);

    UFUNCTION(Exec, Category = "Parry Testing")
    void EnableIFrames(bool bEnable);

    UFUNCTION(Exec, Category = "Parry Testing")
    void TestDamageWithVulnerability(float BaseDamage = 10.0f);
};