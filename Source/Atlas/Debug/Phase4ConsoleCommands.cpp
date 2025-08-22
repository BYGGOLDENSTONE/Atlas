// Phase4ConsoleCommands.cpp
#include "Phase4ConsoleCommands.h"
#include "../Components/IntegrityVisualizerComponent.h"
#include "../Components/StationIntegrityComponent.h"
#include "../Hazards/EnvironmentalHazardComponent.h"
#include "../Hazards/ElectricalSurgeHazard.h"
#include "../Hazards/ToxicLeakHazard.h"
#include "../Hazards/LowGravityHazard.h"
#include "../Interactables/InteractableComponent.h"
#include "../Characters/GameCharacterBase.h"
#include "Engine/World.h"
#include "Engine/Console.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

void UPhase4ConsoleCommands::RegisterCommands()
{
    // Integrity Visualizer Commands
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.SetIntegrity"),
        TEXT("Set station integrity percentage (0-100)"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::SetIntegrityPercent),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.HullBreach"),
        TEXT("Trigger a hull breach at player location"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::TriggerHullBreach),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.ElectricalFailure"),
        TEXT("Trigger electrical failure at player location"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::TriggerElectricalFailure),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.EmergencyLighting"),
        TEXT("Activate emergency lighting"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::TriggerEmergencyLighting),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.ClearEffects"),
        TEXT("Clear all integrity visual effects"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::ClearIntegrityEffects),
        ECVF_Default
    );
    
    // Hazard Commands
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.SpawnHazard"),
        TEXT("Spawn hazard: SpawnHazard [Type] - Types: Electrical, Toxic, Gravity"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::SpawnHazard),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.ActivateHazards"),
        TEXT("Activate all hazards in the level"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::ActivateHazard),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.DeactivateHazards"),
        TEXT("Deactivate all hazards"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::DeactivateAllHazards),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.TestElectrical"),
        TEXT("Test electrical surge hazard"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::TestElectricalSurge),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.TestToxic"),
        TEXT("Test toxic leak hazard"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::TestToxicLeak),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.TestGravity"),
        TEXT("Test low gravity hazard"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::TestLowGravity),
        ECVF_Default
    );
    
    // Interactable Commands
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.SpawnInteractable"),
        TEXT("Spawn interactable at player location"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::SpawnInteractable),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.TestInteractable"),
        TEXT("Test nearest interactable"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::TestInteractable),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.ResetInteractables"),
        TEXT("Reset all interactables"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::ResetAllInteractables),
        ECVF_Default
    );
    
    // Test Commands
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.TestAll"),
        TEXT("Test all Phase 4 systems"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::TestAllPhase4Systems),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.StressTest"),
        TEXT("Stress test environmental systems"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::StressTestEnvironment),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Atlas.Phase4.Status"),
        TEXT("Show Phase 4 systems status"),
        FConsoleCommandWithArgsDelegate::CreateStatic(&UPhase4ConsoleCommands::ShowPhase4Status),
        ECVF_Default
    );
    
    UE_LOG(LogTemp, Log, TEXT("Phase 4 Console Commands Registered"));
}

void UPhase4ConsoleCommands::UnregisterCommands()
{
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.SetIntegrity"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.HullBreach"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.ElectricalFailure"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.EmergencyLighting"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.ClearEffects"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.SpawnHazard"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.ActivateHazards"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.DeactivateHazards"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.TestElectrical"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.TestToxic"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.TestGravity"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.SpawnInteractable"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.TestInteractable"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.ResetInteractables"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.TestAll"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.StressTest"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("Atlas.Phase4.Status"));
}

void UPhase4ConsoleCommands::SetIntegrityPercent(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Usage: Atlas.Phase4.SetIntegrity [0-100]"));
        return;
    }
    
    float Percent = FCString::Atof(*Args[0]);
    Percent = FMath::Clamp(Percent, 0.0f, 100.0f);
    
    // Find and update integrity visualizer
    if (UIntegrityVisualizerComponent* Visualizer = GetIntegrityVisualizer())
    {
        Visualizer->UpdateIntegrityVisuals(Percent);
        UE_LOG(LogTemp, Log, TEXT("Set integrity to %.1f%%"), Percent);
    }
    
    // Also update actual station integrity
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull);
    if (World)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AGameCharacterBase::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (UStationIntegrityComponent* IntegrityComp = Actor->FindComponentByClass<UStationIntegrityComponent>())
            {
                IntegrityComp->SetIntegrityPercent(Percent);
                break;
            }
        }
    }
}

void UPhase4ConsoleCommands::TriggerHullBreach(const TArray<FString>& Args)
{
    if (UIntegrityVisualizerComponent* Visualizer = GetIntegrityVisualizer())
    {
        // Get player location
        UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull);
        if (World)
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    FVector BreachLocation = PlayerPawn->GetActorLocation() + FVector(200, 0, 0);
                    Visualizer->TriggerHullBreach(BreachLocation);
                    UE_LOG(LogTemp, Log, TEXT("Hull breach triggered at player location"));
                }
            }
        }
    }
}

void UPhase4ConsoleCommands::TriggerElectricalFailure(const TArray<FString>& Args)
{
    if (UIntegrityVisualizerComponent* Visualizer = GetIntegrityVisualizer())
    {
        UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull);
        if (World)
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    FVector FailureLocation = PlayerPawn->GetActorLocation();
                    Visualizer->TriggerElectricalFailure(FailureLocation);
                    UE_LOG(LogTemp, Log, TEXT("Electrical failure triggered at player location"));
                }
            }
        }
    }
}

void UPhase4ConsoleCommands::TriggerEmergencyLighting(const TArray<FString>& Args)
{
    if (UIntegrityVisualizerComponent* Visualizer = GetIntegrityVisualizer())
    {
        Visualizer->TriggerEmergencyLighting();
        UE_LOG(LogTemp, Log, TEXT("Emergency lighting activated"));
    }
}

void UPhase4ConsoleCommands::ClearIntegrityEffects(const TArray<FString>& Args)
{
    if (UIntegrityVisualizerComponent* Visualizer = GetIntegrityVisualizer())
    {
        Visualizer->ClearAllEffects();
        UE_LOG(LogTemp, Log, TEXT("All integrity effects cleared"));
    }
}

void UPhase4ConsoleCommands::SpawnHazard(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Usage: Atlas.Phase4.SpawnHazard [Electrical/Toxic/Gravity]"));
        return;
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;
    
    FVector SpawnLocation = PC->GetPawn()->GetActorLocation() + FVector(300, 0, 0);
    FActorSpawnParameters SpawnParams;
    
    AActor* HazardActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (!HazardActor) return;
    
    UEnvironmentalHazardComponent* HazardComp = nullptr;
    FString HazardType = Args[0].ToLower();
    
    if (HazardType == "electrical")
    {
        HazardComp = NewObject<UElectricalSurgeHazard>(HazardActor, TEXT("ElectricalHazard"));
    }
    else if (HazardType == "toxic")
    {
        HazardComp = NewObject<UToxicLeakHazard>(HazardActor, TEXT("ToxicHazard"));
    }
    else if (HazardType == "gravity")
    {
        HazardComp = NewObject<ULowGravityHazard>(HazardActor, TEXT("GravityHazard"));
    }
    
    if (HazardComp)
    {
        HazardComp->RegisterComponent();
        HazardComp->ActivateHazard();
        UE_LOG(LogTemp, Log, TEXT("Spawned %s hazard at player location"), *HazardType);
    }
}

void UPhase4ConsoleCommands::ActivateHazard(const TArray<FString>& Args)
{
    TArray<UEnvironmentalHazardComponent*> Hazards = GetAllHazards();
    int32 ActivatedCount = 0;
    
    for (UEnvironmentalHazardComponent* Hazard : Hazards)
    {
        if (Hazard && !Hazard->IsHazardActive())
        {
            Hazard->ActivateHazard();
            ActivatedCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Activated %d hazards"), ActivatedCount);
}

void UPhase4ConsoleCommands::DeactivateAllHazards(const TArray<FString>& Args)
{
    TArray<UEnvironmentalHazardComponent*> Hazards = GetAllHazards();
    int32 DeactivatedCount = 0;
    
    for (UEnvironmentalHazardComponent* Hazard : Hazards)
    {
        if (Hazard && Hazard->IsHazardActive())
        {
            Hazard->DeactivateHazard();
            DeactivatedCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Deactivated %d hazards"), DeactivatedCount);
}

void UPhase4ConsoleCommands::TestElectricalSurge(const TArray<FString>& Args)
{
    SpawnHazard(TArray<FString>{"Electrical"});
}

void UPhase4ConsoleCommands::TestToxicLeak(const TArray<FString>& Args)
{
    SpawnHazard(TArray<FString>{"Toxic"});
}

void UPhase4ConsoleCommands::TestLowGravity(const TArray<FString>& Args)
{
    SpawnHazard(TArray<FString>{"Gravity"});
}

void UPhase4ConsoleCommands::SpawnInteractable(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;
    
    FVector SpawnLocation = PC->GetPawn()->GetActorLocation() + PC->GetPawn()->GetActorForwardVector() * 200.0f;
    FActorSpawnParameters SpawnParams;
    
    AActor* InteractableActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (!InteractableActor) return;
    
    UInteractableComponent* InteractableComp = NewObject<UInteractableComponent>(InteractableActor, TEXT("TestInteractable"));
    if (InteractableComp)
    {
        InteractableComp->RegisterComponent();
        InteractableComp->InteractionPromptText = FText::FromString("Test Interactable");
        InteractableComp->Cooldown = 5.0f;
        InteractableComp->MaxUses = 3;
        UE_LOG(LogTemp, Log, TEXT("Spawned test interactable at player location"));
    }
}

void UPhase4ConsoleCommands::TestInteractable(const TArray<FString>& Args)
{
    TArray<UInteractableComponent*> Interactables = GetAllInteractables();
    
    if (Interactables.Num() > 0)
    {
        UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull);
        if (World)
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    // Find nearest interactable
                    UInteractableComponent* Nearest = nullptr;
                    float NearestDist = FLT_MAX;
                    
                    for (UInteractableComponent* Interactable : Interactables)
                    {
                        if (Interactable && Interactable->GetOwner())
                        {
                            float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), 
                                                       Interactable->GetOwner()->GetActorLocation());
                            if (Dist < NearestDist)
                            {
                                NearestDist = Dist;
                                Nearest = Interactable;
                            }
                        }
                    }
                    
                    if (Nearest && Nearest->CanInteract_Implementation(PlayerPawn))
                    {
                        Nearest->Interact_Implementation(PlayerPawn);
                        UE_LOG(LogTemp, Log, TEXT("Triggered nearest interactable"));
                    }
                }
            }
        }
    }
}

void UPhase4ConsoleCommands::ResetAllInteractables(const TArray<FString>& Args)
{
    TArray<UInteractableComponent*> Interactables = GetAllInteractables();
    int32 ResetCount = 0;
    
    for (UInteractableComponent* Interactable : Interactables)
    {
        if (Interactable)
        {
            Interactable->ResetInteractable();
            ResetCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Reset %d interactables"), ResetCount);
}

void UPhase4ConsoleCommands::TestAllPhase4Systems(const TArray<FString>& Args)
{
    UE_LOG(LogTemp, Log, TEXT("=== Testing All Phase 4 Systems ==="));
    
    // Test integrity at different levels
    SetIntegrityPercent(TArray<FString>{"75"});
    FPlatformProcess::Sleep(1.0f);
    SetIntegrityPercent(TArray<FString>{"50"});
    FPlatformProcess::Sleep(1.0f);
    SetIntegrityPercent(TArray<FString>{"25"});
    FPlatformProcess::Sleep(1.0f);
    
    // Test hazards
    TestElectricalSurge(Args);
    FPlatformProcess::Sleep(1.0f);
    TestToxicLeak(Args);
    FPlatformProcess::Sleep(1.0f);
    TestLowGravity(Args);
    
    // Test interactables
    SpawnInteractable(Args);
    
    UE_LOG(LogTemp, Log, TEXT("=== Phase 4 Systems Test Complete ==="));
}

void UPhase4ConsoleCommands::StressTestEnvironment(const TArray<FString>& Args)
{
    UE_LOG(LogTemp, Log, TEXT("=== Starting Environment Stress Test ==="));
    
    // Spawn multiple hazards
    for (int32 i = 0; i < 5; i++)
    {
        SpawnHazard(TArray<FString>{"Electrical"});
        SpawnHazard(TArray<FString>{"Toxic"});
    }
    
    // Trigger multiple hull breaches
    for (int32 i = 0; i < 3; i++)
    {
        TriggerHullBreach(Args);
    }
    
    // Set critical integrity
    SetIntegrityPercent(TArray<FString>{"10"});
    
    UE_LOG(LogTemp, Log, TEXT("=== Stress Test Active - Monitor Performance ==="));
}

void UPhase4ConsoleCommands::ShowPhase4Status(const TArray<FString>& Args)
{
    UE_LOG(LogTemp, Log, TEXT("=== Phase 4 Systems Status ==="));
    
    // Check Integrity Visualizer
    if (UIntegrityVisualizerComponent* Visualizer = GetIntegrityVisualizer())
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Integrity Visualizer: Active"));
        UE_LOG(LogTemp, Log, TEXT("  Current State: %s"), *UEnum::GetValueAsString(Visualizer->GetCurrentIntegrityState()));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✗ Integrity Visualizer: Not Found"));
    }
    
    // Check Hazards
    TArray<UEnvironmentalHazardComponent*> Hazards = GetAllHazards();
    UE_LOG(LogTemp, Log, TEXT("✓ Environmental Hazards: %d found"), Hazards.Num());
    int32 ActiveHazards = 0;
    for (UEnvironmentalHazardComponent* Hazard : Hazards)
    {
        if (Hazard && Hazard->IsHazardActive())
        {
            ActiveHazards++;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("  Active: %d"), ActiveHazards);
    
    // Check Interactables
    TArray<UInteractableComponent*> Interactables = GetAllInteractables();
    UE_LOG(LogTemp, Log, TEXT("✓ Interactables: %d found"), Interactables.Num());
    int32 ReadyInteractables = 0;
    for (UInteractableComponent* Interactable : Interactables)
    {
        if (Interactable && Interactable->GetCurrentState() == EInteractableState::Ready)
        {
            ReadyInteractables++;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("  Ready: %d"), ReadyInteractables);
    
    UE_LOG(LogTemp, Log, TEXT("=== End Status Report ==="));
}

UIntegrityVisualizerComponent* UPhase4ConsoleCommands::GetIntegrityVisualizer()
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (UIntegrityVisualizerComponent* Visualizer = Actor->FindComponentByClass<UIntegrityVisualizerComponent>())
        {
            return Visualizer;
        }
    }
    
    return nullptr;
}

TArray<UEnvironmentalHazardComponent*> UPhase4ConsoleCommands::GetAllHazards()
{
    TArray<UEnvironmentalHazardComponent*> Hazards;
    
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return Hazards;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        TArray<UEnvironmentalHazardComponent*> ActorHazards;
        Actor->GetComponents<UEnvironmentalHazardComponent>(ActorHazards);
        Hazards.Append(ActorHazards);
    }
    
    return Hazards;
}

TArray<UInteractableComponent*> UPhase4ConsoleCommands::GetAllInteractables()
{
    TArray<UInteractableComponent*> Interactables;
    
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine->GameViewport, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return Interactables;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        TArray<UInteractableComponent*> ActorInteractables;
        Actor->GetComponents<UInteractableComponent>(ActorInteractables);
        Interactables.Append(ActorInteractables);
    }
    
    return Interactables;
}