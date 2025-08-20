#include "AtlasGameState.h"
#include "../Components/StationIntegrityComponent.h"
#include "AtlasGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AAtlasGameState::AAtlasGameState()
{
    StationIntegrityComponent = CreateDefaultSubobject<UStationIntegrityComponent>(TEXT("StationIntegrityComponent"));
}

void AAtlasGameState::BeginPlay()
{
    Super::BeginPlay();
    
    if (StationIntegrityComponent)
    {
        StationIntegrityComponent->OnIntegrityFailed.AddDynamic(this, &AAtlasGameState::OnIntegrityFailedHandler);
        StationIntegrityComponent->OnIntegrityCritical.AddDynamic(this, &AAtlasGameState::OnIntegrityCriticalHandler);
    }
}

void AAtlasGameState::OnIntegrityFailedHandler()
{
    UE_LOG(LogTemp, Error, TEXT("Station Integrity Failed! Triggering Game Over"));
    
    UAtlasGameInstance* GameInstance = Cast<UAtlasGameInstance>(GetGameInstance());
    if (GameInstance)
    {
        GameInstance->TriggerGameOver(EGameOverReason::StationIntegrityFailure);
    }
}

void AAtlasGameState::OnIntegrityCriticalHandler()
{
    UE_LOG(LogTemp, Warning, TEXT("Station Integrity Critical! At 50%%"));
}

AAtlasGameState* AAtlasGameState::GetAtlasGameState(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }
    
    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    return Cast<AAtlasGameState>(World->GetGameState());
}