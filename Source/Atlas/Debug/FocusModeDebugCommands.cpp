#include "FocusModeDebugCommands.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "../Characters/PlayerCharacter.h"
#include "../Components/FocusModeComponent.h"
#include "../Actors/VentInteractable.h"
#include "../Actors/ValveInteractable.h"
#include "../Core/AtlasGameplayTags.h"
#include "DrawDebugHelpers.h"

static FAutoConsoleCommand ToggleFocusModeCmd(
    TEXT("Atlas.ToggleFocusMode"),
    TEXT("Toggle Focus Mode on/off"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&FFocusModeDebugCommands::ToggleFocusMode)
);

static FAutoConsoleCommand ShowFocusDebugCmd(
    TEXT("Atlas.ShowFocusDebug"),
    TEXT("Toggle Focus Mode debug visualization. Args: [0/1]"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&FFocusModeDebugCommands::ShowFocusDebug)
);

static FAutoConsoleCommand SpawnTestInteractableCmd(
    TEXT("Atlas.SpawnTestInteractable"),
    TEXT("Spawn a test interactable. Args: [Type (Vent/Valve)] [Distance (optional)]"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&FFocusModeDebugCommands::SpawnTestInteractable)
);

static FAutoConsoleCommand TestInteractionCmd(
    TEXT("Atlas.TestInteraction"),
    TEXT("Test interaction with currently focused target"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&FFocusModeDebugCommands::TestInteraction)
);

static FAutoConsoleCommand SetFocusRangeCmd(
    TEXT("Atlas.SetFocusRange"),
    TEXT("Set the focus mode detection range. Args: [Range]"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&FFocusModeDebugCommands::SetFocusRange)
);

static FAutoConsoleCommand ForceUnlockCmd(
    TEXT("Atlas.ForceUnlock"),
    TEXT("Force unlock the current soft lock"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&FFocusModeDebugCommands::ForceUnlockFocus)
);

static FAutoConsoleCommand ShowFocusInfoCmd(
    TEXT("Atlas.ShowFocusInfo"),
    TEXT("Display current focus mode information"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&FFocusModeDebugCommands::ShowFocusInfo)
);

static FAutoConsoleCommand TestSoftLockCmd(
    TEXT("Atlas.TestSoftLock"),
    TEXT("Test soft lock engagement. Args: [Enable (0/1)]"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&FFocusModeDebugCommands::TestSoftLock)
);

static FAutoConsoleCommand SimulateVentCmd(
    TEXT("Atlas.SimulateVent"),
    TEXT("Simulate a Vent interaction at player location"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&FFocusModeDebugCommands::SimulateVentInteraction)
);

static FAutoConsoleCommand SimulateValveCmd(
    TEXT("Atlas.SimulateValve"),
    TEXT("Simulate a Valve interaction. Args: [Type (Fire/Electric/Poison/Physical)]"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&FFocusModeDebugCommands::SimulateValveInteraction)
);

void FFocusModeDebugCommands::RegisterConsoleCommands()
{
}

void FFocusModeDebugCommands::ToggleFocusMode(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (!PlayerChar) return;
    
    UFocusModeComponent* FocusComp = PlayerChar->FindComponentByClass<UFocusModeComponent>();
    if (!FocusComp) return;
    
    FocusComp->ToggleFocusMode();
    
    FString Status = FocusComp->IsFocusModeActive() ? TEXT("ON") : TEXT("OFF");
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("Focus Mode: %s"), *Status));
}

void FFocusModeDebugCommands::ShowFocusDebug(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (!PlayerChar) return;
    
    UFocusModeComponent* FocusComp = PlayerChar->FindComponentByClass<UFocusModeComponent>();
    if (!FocusComp) return;
    
    bool bEnable = true;
    if (Args.Num() > 0)
    {
        bEnable = FCString::Atoi(*Args[0]) != 0;
    }
    else
    {
        bEnable = !FocusComp->bDebugDrawFocusInfo;
    }
    
    FocusComp->bDebugDrawFocusInfo = bEnable;
    
    FString Status = bEnable ? TEXT("ON") : TEXT("OFF");
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, FString::Printf(TEXT("Focus Debug Visualization: %s"), *Status));
}

void FFocusModeDebugCommands::SpawnTestInteractable(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (!PlayerChar) return;
    
    FString Type = TEXT("Vent");
    float Distance = 300.0f;
    
    if (Args.Num() > 0)
    {
        Type = Args[0];
    }
    if (Args.Num() > 1)
    {
        Distance = FCString::Atof(*Args[1]);
    }
    
    FVector SpawnLocation = PlayerChar->GetActorLocation() + PlayerChar->GetActorForwardVector() * Distance;
    SpawnLocation.Z = PlayerChar->GetActorLocation().Z;
    
    FRotator SpawnRotation = PlayerChar->GetActorRotation();
    SpawnRotation.Pitch = 0;
    SpawnRotation.Roll = 0;
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* SpawnedActor = nullptr;
    
    if (Type.Equals(TEXT("Valve"), ESearchCase::IgnoreCase))
    {
        SpawnedActor = World->SpawnActor<AValveInteractable>(AValveInteractable::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
    }
    else
    {
        SpawnedActor = World->SpawnActor<AVentInteractable>(AVentInteractable::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
    }
    
    if (SpawnedActor)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("Spawned %s at distance %.0f"), *Type, Distance));
        
        DrawDebugBox(World, SpawnLocation, FVector(50, 50, 100), FColor::Cyan, false, 5.0f, 0, 5.0f);
    }
}

void FFocusModeDebugCommands::TestInteraction(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (!PlayerChar) return;
    
    UFocusModeComponent* FocusComp = PlayerChar->FindComponentByClass<UFocusModeComponent>();
    if (!FocusComp) return;
    
    if (!FocusComp->IsFocusModeActive())
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Focus Mode is not active!"));
        return;
    }
    
    if (FocusComp->TryInteractWithFocusedTarget())
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Interaction successful!"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Interaction failed (no target or on cooldown)"));
    }
}

void FFocusModeDebugCommands::SetFocusRange(const TArray<FString>& Args)
{
    if (Args.Num() == 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Usage: Atlas.SetFocusRange [Range]"));
        return;
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (!PlayerChar) return;
    
    UFocusModeComponent* FocusComp = PlayerChar->FindComponentByClass<UFocusModeComponent>();
    if (!FocusComp) return;
    
    float NewRange = FCString::Atof(*Args[0]);
    FocusComp->FocusRange = FMath::Clamp(NewRange, 100.0f, 5000.0f);
    
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
        FString::Printf(TEXT("Focus Range set to: %.0f"), FocusComp->FocusRange));
}

void FFocusModeDebugCommands::ForceUnlockFocus(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (!PlayerChar) return;
    
    UFocusModeComponent* FocusComp = PlayerChar->FindComponentByClass<UFocusModeComponent>();
    if (!FocusComp) return;
    
    FocusComp->ForceUnlock();
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Force unlock executed"));
}

void FFocusModeDebugCommands::ShowFocusInfo(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (!PlayerChar) return;
    
    UFocusModeComponent* FocusComp = PlayerChar->FindComponentByClass<UFocusModeComponent>();
    if (!FocusComp) return;
    
    FString InfoText = FString::Printf(TEXT("=== Focus Mode Info ===\n"));
    InfoText += FString::Printf(TEXT("Active: %s\n"), FocusComp->IsFocusModeActive() ? TEXT("YES") : TEXT("NO"));
    InfoText += FString::Printf(TEXT("Soft Locked: %s\n"), FocusComp->IsSoftLocked() ? TEXT("YES") : TEXT("NO"));
    InfoText += FString::Printf(TEXT("Focus Range: %.0f\n"), FocusComp->FocusRange);
    InfoText += FString::Printf(TEXT("Potential Targets: %d\n"), FocusComp->PotentialTargets.Num());
    
    if (FocusComp->CurrentFocusedTarget)
    {
        InfoText += FString::Printf(TEXT("Current Target: %s\n"), *FocusComp->CurrentFocusedTarget->GetName());
        InfoText += FString::Printf(TEXT("  - Distance: %.0f\n"), FocusComp->CurrentTargetInfo.WorldDistance);
        InfoText += FString::Printf(TEXT("  - Screen Distance: %.1f\n"), FocusComp->CurrentTargetInfo.ScreenDistance);
        InfoText += FString::Printf(TEXT("  - Priority: %.2f\n"), FocusComp->CurrentTargetInfo.Priority);
        InfoText += FString::Printf(TEXT("  - Is Interactable: %s\n"), FocusComp->CurrentTargetInfo.bIsInteractable ? TEXT("YES") : TEXT("NO"));
        InfoText += FString::Printf(TEXT("  - Is Enemy: %s\n"), FocusComp->CurrentTargetInfo.bIsEnemy ? TEXT("YES") : TEXT("NO"));
    }
    else
    {
        InfoText += TEXT("Current Target: NONE\n");
    }
    
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, InfoText);
}

void FFocusModeDebugCommands::TestSoftLock(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (!PlayerChar) return;
    
    UFocusModeComponent* FocusComp = PlayerChar->FindComponentByClass<UFocusModeComponent>();
    if (!FocusComp) return;
    
    bool bEnable = true;
    if (Args.Num() > 0)
    {
        bEnable = FCString::Atoi(*Args[0]) != 0;
    }
    
    FocusComp->bIsSoftLocked = bEnable;
    
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
        FString::Printf(TEXT("Soft Lock %s"), bEnable ? TEXT("ENGAGED") : TEXT("DISENGAGED")));
}

void FFocusModeDebugCommands::SimulateVentInteraction(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (!PlayerChar) return;
    
    FVector Location = PlayerChar->GetActorLocation() + PlayerChar->GetActorForwardVector() * 200;
    
    DrawDebugSphere(World, Location, 50, 12, FColor::Blue, false, 3.0f);
    DrawDebugLine(World, Location, Location + PlayerChar->GetActorForwardVector() * 1000, FColor::Blue, false, 3.0f, 0, 5.0f);
    
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("Simulated Vent projectile stagger effect"));
}

void FFocusModeDebugCommands::SimulateValveInteraction(const TArray<FString>& Args)
{
    UWorld* World = GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return;
    
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (!PlayerChar) return;
    
    FString EffectType = TEXT("Fire");
    if (Args.Num() > 0)
    {
        EffectType = Args[0];
    }
    
    FVector Location = PlayerChar->GetActorLocation();
    float Radius = 500.0f;
    
    FColor Color = FColor::Red;
    if (EffectType.Equals(TEXT("Electric"), ESearchCase::IgnoreCase))
    {
        Color = FColor::Cyan;
    }
    else if (EffectType.Equals(TEXT("Poison"), ESearchCase::IgnoreCase))
    {
        Color = FColor::Green;
    }
    else if (EffectType.Equals(TEXT("Physical"), ESearchCase::IgnoreCase))
    {
        Color = FColor::White;
    }
    
    DrawDebugSphere(World, Location, Radius, 32, Color, false, 3.0f, 0, 3.0f);
    
    for (int32 i = 0; i < 8; i++)
    {
        float Angle = (360.0f / 8) * i;
        FVector EndPoint = Location + FVector(FMath::Cos(FMath::DegreesToRadians(Angle)), FMath::Sin(FMath::DegreesToRadians(Angle)), 0) * Radius;
        DrawDebugLine(World, Location, EndPoint, Color, false, 3.0f, 0, 5.0f);
    }
    
    GEngine->AddOnScreenDebugMessage(-1, 3.0f, Color, 
        FString::Printf(TEXT("Simulated Valve %s AoE effect (Radius: %.0f)"), *EffectType, Radius));
}