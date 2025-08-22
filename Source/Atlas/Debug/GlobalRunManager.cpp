#include "GlobalRunManager.h"
#include "Atlas/Components/RunManagerComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// Static member initialization
URunManagerComponent* FGlobalRunManager::Instance = nullptr;

URunManagerComponent* FGlobalRunManager::Get()
{
	// If no instance exists, create one
	if (!Instance)
	{
		Instance = CreateNew();
	}
	
	return Instance;
}

void FGlobalRunManager::Set(URunManagerComponent* InRunManager)
{
	Instance = InRunManager;
	if (Instance)
	{
		UE_LOG(LogTemp, Log, TEXT("GlobalRunManager: Set to %s"), *Instance->GetName());
	}
}

URunManagerComponent* FGlobalRunManager::CreateNew()
{
	if (!GEngine)
	{
		UE_LOG(LogTemp, Error, TEXT("GlobalRunManager: Cannot create - GEngine is null"));
		return nullptr;
	}
	
	// Get the current world
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World())
		{
			World = Context.World();
			break;
		}
	}
	
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("GlobalRunManager: Cannot create - No world found"));
		return nullptr;
	}
	
	// Create a dummy actor to hold the component
	AActor* Holder = World->SpawnActor<AActor>();
	if (!Holder)
	{
		UE_LOG(LogTemp, Error, TEXT("GlobalRunManager: Cannot create holder actor"));
		return nullptr;
	}
	
	Holder->SetActorLabel(TEXT("GlobalRunManagerHolder"));
	
	// Create the RunManager component
	Instance = NewObject<URunManagerComponent>(Holder, TEXT("GlobalRunManager"));
	if (Instance)
	{
		Instance->RegisterComponent();
		UE_LOG(LogTemp, Warning, TEXT("GlobalRunManager: Created new global RunManager instance"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GlobalRunManager: Failed to create RunManager component"));
	}
	
	return Instance;
}