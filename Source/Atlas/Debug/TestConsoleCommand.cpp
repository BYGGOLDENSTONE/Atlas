#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"

// Simple test command that should always work
static FAutoConsoleCommand TestCommand(
	TEXT("Atlas.Test"),
	TEXT("Simple test command to verify console system works"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UE_LOG(LogTemp, Warning, TEXT("===== ATLAS TEST COMMAND WORKS! ====="));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Atlas Test Command Executed Successfully!"));
		}
	}),
	ECVF_Default
);

// Another test using different registration method
static void TestPhase3()
{
	UE_LOG(LogTemp, Warning, TEXT("===== ATLAS PHASE3 TEST WORKS! ====="));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Atlas.TestPhase3 Command Executed!"));
	}
}

static FAutoConsoleCommand TestPhase3Command(
	TEXT("Atlas.TestPhase3"),
	TEXT("Test Phase3 command registration"),
	FConsoleCommandDelegate::CreateStatic(&TestPhase3),
	ECVF_Default
);