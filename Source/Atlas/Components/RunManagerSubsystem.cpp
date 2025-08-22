#include "RunManagerSubsystem.h"
#include "RunManagerComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

void URunManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("RunManagerSubsystem: Initializing"));
	
	// We'll create the component when first requested
	RunManagerComponent = nullptr;
	ComponentHolder = nullptr;
}

void URunManagerSubsystem::Deinitialize()
{
	// Clean up
	if (ComponentHolder)
	{
		ComponentHolder->Destroy();
		ComponentHolder = nullptr;
	}
	
	RunManagerComponent = nullptr;
	
	UE_LOG(LogTemp, Log, TEXT("RunManagerSubsystem: Deinitialized"));
	
	Super::Deinitialize();
}

URunManagerComponent* URunManagerSubsystem::GetRunManager()
{
	// Create on first access
	if (!RunManagerComponent)
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("RunManagerSubsystem: No world available"));
			return nullptr;
		}
		
		// Create a dummy actor to hold the component
		ComponentHolder = World->SpawnActor<AActor>();
		if (!ComponentHolder)
		{
			UE_LOG(LogTemp, Error, TEXT("RunManagerSubsystem: Failed to spawn component holder"));
			return nullptr;
		}
		
		ComponentHolder->SetActorLabel(TEXT("RunManagerHolder"));
		
		// Create the run manager component
		RunManagerComponent = NewObject<URunManagerComponent>(ComponentHolder, TEXT("RunManagerComponent"));
		if (RunManagerComponent)
		{
			RunManagerComponent->RegisterComponent();
			UE_LOG(LogTemp, Log, TEXT("RunManagerSubsystem: Created RunManagerComponent"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RunManagerSubsystem: Failed to create RunManagerComponent"));
		}
	}
	
	return RunManagerComponent;
}

URunManagerSubsystem* URunManagerSubsystem::Get(const UObject* WorldContextObject)
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
	
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}
	
	return GameInstance->GetSubsystem<URunManagerSubsystem>();
}