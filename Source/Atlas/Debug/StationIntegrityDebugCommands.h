#pragma once

#include "CoreMinimal.h"

class ATLAS_API FStationIntegrityDebugCommands
{
public:
    static void RegisterCommands();
    
private:
    static void DamageIntegrity(const TArray<FString>& Args);
    static void SetIntegrity(const TArray<FString>& Args);
    static void SetIntegrityPercent(const TArray<FString>& Args);
    static void ShowIntegrityStatus(const TArray<FString>& Args);
    static void TestHighRiskAbility(const TArray<FString>& Args);
    static void ResetIntegrity(const TArray<FString>& Args);
};