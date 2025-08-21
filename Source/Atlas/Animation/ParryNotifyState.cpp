#include "ParryNotifyState.h"
#include "Atlas/Characters/GameCharacterBase.h"
#include "Atlas/Components/ActionManagerComponent.h"
#include "Atlas/Components/VulnerabilityComponent.h"
#include "Atlas/Interfaces/ICombatInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UParryNotifyState::UParryNotifyState()
{
	CurrentParryTime = 0.0f;
	
	// Don't request gameplay tags in constructor - they may not be loaded yet
	// Set default values that will be used in editor
	PerfectParryWindow = 0.2f;
	LateParryWindow = 0.1f;
	PerfectParryTimeDilation = 0.5f;
	TimeDilationDuration = 0.2f;
}

void UParryNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	AGameCharacterBase* Character = Cast<AGameCharacterBase>(MeshComp->GetOwner());
	if (!Character)
	{
		return;
	}

	UActionManagerComponent* ActionManager = Character->GetActionManagerComponent();
	if (!ActionManager)
	{
		return;
	}

	// Reset parry time
	CurrentParryTime = 0.0f;

	// Set parry state active
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Character))
	{
		CombatInterface->SetParryState(true);
	}

	// Add parry gameplay tag - use FName to avoid tag request in constructor
	FGameplayTag ParryTag = FGameplayTag::RequestGameplayTag(FName("Action.Parry"));
	ActionManager->AddCombatStateTag(ParryTag);

	UE_LOG(LogTemp, Log, TEXT("Parry window started for %s"), *Character->GetName());
}

void UParryNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	AGameCharacterBase* Character = Cast<AGameCharacterBase>(MeshComp->GetOwner());
	if (!Character)
	{
		return;
	}

	UActionManagerComponent* ActionManager = Character->GetActionManagerComponent();
	if (!ActionManager)
	{
		return;
	}

	// Clear parry state
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Character))
	{
		CombatInterface->SetParryState(false);
	}

	// Remove parry gameplay tag - use FName to avoid tag request in constructor
	FGameplayTag ParryTag = FGameplayTag::RequestGameplayTag(FName("Action.Parry"));
	ActionManager->RemoveCombatStateTag(ParryTag);

	// Clear any active time dilation
	if (TimeDilationTimerHandle.IsValid())
	{
		ClearTimeDilationEffect(MeshComp->GetWorld());
	}

	UE_LOG(LogTemp, Log, TEXT("Parry window ended for %s"), *Character->GetName());
}

void UParryNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	// Update parry timer
	CurrentParryTime += FrameDeltaTime;

	// Check if we've exited all parry windows
	if (CurrentParryTime > PerfectParryWindow + LateParryWindow)
	{
		AGameCharacterBase* Character = Cast<AGameCharacterBase>(MeshComp->GetOwner());
		if (Character)
		{
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Character))
			{
				CombatInterface->SetParryState(false);
			}
		}
	}
}

FString UParryNotifyState::GetNotifyName_Implementation() const
{
	return TEXT("Parry Window");
}

bool UParryNotifyState::IsInPerfectWindow() const
{
	return CurrentParryTime <= PerfectParryWindow;
}

bool UParryNotifyState::IsInLateWindow() const
{
	return CurrentParryTime > PerfectParryWindow && CurrentParryTime <= (PerfectParryWindow + LateParryWindow);
}

void UParryNotifyState::OnParrySuccess(AGameCharacterBase* Character, AActor* Attacker, bool bPerfectParry)
{
	if (!Character || !Attacker)
	{
		return;
	}

	UWorld* World = Character->GetWorld();
	if (!World)
	{
		return;
	}

	if (bPerfectParry)
	{
		// Apply vulnerability to attacker
		AGameCharacterBase* AttackerChar = Cast<AGameCharacterBase>(Attacker);
		if (AttackerChar)
		{
			UVulnerabilityComponent* VulnComp = AttackerChar->FindComponentByClass<UVulnerabilityComponent>();
			if (VulnComp)
			{
				// Apply Stunned tier vulnerability
				VulnComp->ApplyVulnerabilityTier(EVulnerabilityTier::Stunned);
			}
		}

		// Spawn perfect parry VFX
		if (PerfectParryVFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, PerfectParryVFX, Character->GetActorLocation());
		}

		// Apply time dilation effect
		ApplyTimeDilationEffect(World);

		UE_LOG(LogTemp, Log, TEXT("Perfect parry executed! Attacker stunned."));
	}
	else
	{
		// Spawn regular parry VFX
		if (ParrySuccessVFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, ParrySuccessVFX, Character->GetActorLocation());
		}

		UE_LOG(LogTemp, Log, TEXT("Late parry executed - 50%% damage reduction"));
	}
}

void UParryNotifyState::ApplyTimeDilationEffect(UWorld* World)
{
	if (!World)
	{
		return;
	}

	// Apply time dilation
	UGameplayStatics::SetGlobalTimeDilation(World, PerfectParryTimeDilation);

	// Set timer to clear effect
	World->GetTimerManager().SetTimer(
		TimeDilationTimerHandle,
		[this, World]() { ClearTimeDilationEffect(World); },
		TimeDilationDuration,
		false
	);
}

void UParryNotifyState::ClearTimeDilationEffect(UWorld* World)
{
	if (!World)
	{
		return;
	}

	// Reset time dilation
	UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);

	// Clear timer
	if (TimeDilationTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(TimeDilationTimerHandle);
		TimeDilationTimerHandle.Invalidate();
	}
}