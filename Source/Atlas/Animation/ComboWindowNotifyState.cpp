#include "ComboWindowNotifyState.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Components/CombatComponent.h"
#include "../Components/ActionManagerComponent.h"
#include "../Characters/GameCharacterBase.h"

UComboWindowNotifyState::UComboWindowNotifyState()
{
	// NotifyColor is inherited from UAnimNotifyState
	// Set it in constructor if needed
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Cyan;
#endif
}

void UComboWindowNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
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

	// Get the action manager to handle input buffering
	UActionManagerComponent* ActionManager = Character->FindComponentByClass<UActionManagerComponent>();
	if (ActionManager)
	{
		// Enable input buffering for combo
		ActionManager->SetComboWindowActive(true, ComboWindowName);
		UE_LOG(LogTemp, Log, TEXT("ComboWindow OPEN: %s"), *ComboWindowName.ToString());
	}
}

void UComboWindowNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

	// Get the action manager to handle input buffering
	UActionManagerComponent* ActionManager = Character->FindComponentByClass<UActionManagerComponent>();
	if (ActionManager)
	{
		// Close input buffering window and execute any buffered input
		ActionManager->SetComboWindowActive(false, ComboWindowName);
		ActionManager->ExecuteBufferedAction();
		UE_LOG(LogTemp, Log, TEXT("ComboWindow CLOSED: %s"), *ComboWindowName.ToString());
	}
}

FString UComboWindowNotifyState::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("Combo: %s"), *ComboWindowName.ToString());
}