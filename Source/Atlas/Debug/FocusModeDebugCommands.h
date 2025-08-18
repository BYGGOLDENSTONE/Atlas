#pragma once

#include "CoreMinimal.h"

class ATLAS_API FFocusModeDebugCommands
{
public:
    static void RegisterConsoleCommands();
    
private:
    static void ToggleFocusMode(const TArray<FString>& Args);
    static void ShowFocusDebug(const TArray<FString>& Args);
    static void SpawnTestInteractable(const TArray<FString>& Args);
    static void TestInteraction(const TArray<FString>& Args);
    static void SetFocusRange(const TArray<FString>& Args);
    static void ForceUnlockFocus(const TArray<FString>& Args);
    static void ShowFocusInfo(const TArray<FString>& Args);
    static void TestSoftLock(const TArray<FString>& Args);
    static void SimulateVentInteraction(const TArray<FString>& Args);
    static void SimulateValveInteraction(const TArray<FString>& Args);
};