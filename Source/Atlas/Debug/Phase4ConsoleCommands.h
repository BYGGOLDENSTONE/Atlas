// Phase4ConsoleCommands.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Phase4ConsoleCommands.generated.h"

UCLASS()
class ATLAS_API UPhase4ConsoleCommands : public UObject
{
    GENERATED_BODY()

public:
    static void RegisterCommands();
    static void UnregisterCommands();

private:
    // Integrity Visualizer Commands
    static void SetIntegrityPercent(const TArray<FString>& Args);
    static void TriggerHullBreach(const TArray<FString>& Args);
    static void TriggerElectricalFailure(const TArray<FString>& Args);
    static void TriggerEmergencyLighting(const TArray<FString>& Args);
    static void ClearIntegrityEffects(const TArray<FString>& Args);
    
    // Hazard Commands
    static void SpawnHazard(const TArray<FString>& Args);
    static void ActivateHazard(const TArray<FString>& Args);
    static void DeactivateAllHazards(const TArray<FString>& Args);
    static void TestElectricalSurge(const TArray<FString>& Args);
    static void TestToxicLeak(const TArray<FString>& Args);
    static void TestLowGravity(const TArray<FString>& Args);
    
    // Interactable Commands
    static void SpawnInteractable(const TArray<FString>& Args);
    static void TestInteractable(const TArray<FString>& Args);
    static void ResetAllInteractables(const TArray<FString>& Args);
    
    // Environmental Testing
    static void TestAllPhase4Systems(const TArray<FString>& Args);
    static void StressTestEnvironment(const TArray<FString>& Args);
    static void ShowPhase4Status(const TArray<FString>& Args);
    
    // Helper functions
    static class UIntegrityVisualizerComponent* GetIntegrityVisualizer();
    static TArray<class UEnvironmentalHazardComponent*> GetAllHazards();
    static TArray<class UInteractableComponent*> GetAllInteractables();
};