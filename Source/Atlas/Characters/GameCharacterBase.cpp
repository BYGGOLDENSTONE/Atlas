#include "GameCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/ActionManagerComponent.h"
#include "../Actions/BaseAction.h"

AGameCharacterBase::AGameCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	ActionManagerComponent = CreateDefaultSubobject<UActionManagerComponent>(TEXT("ActionManagerComponent"));
}

void AGameCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AGameCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGameCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// ICombatInterface Implementation
bool AGameCharacterBase::IsInCombat_Implementation() const
{
	if (ActionManagerComponent)
	{
		return ActionManagerComponent->IsInCombat();
	}
	return false;
}

bool AGameCharacterBase::IsAttacking_Implementation() const
{
	if (ActionManagerComponent)
	{
		return ActionManagerComponent->IsAttacking();
	}
	return false;
}

bool AGameCharacterBase::IsBlocking_Implementation() const
{
	if (ActionManagerComponent)
	{
		return ActionManagerComponent->IsBlocking();
	}
	return false;
}

bool AGameCharacterBase::IsVulnerable_Implementation() const
{
	if (ActionManagerComponent)
	{
		return ActionManagerComponent->IsVulnerable();
	}
	return false;
}

bool AGameCharacterBase::HasIFrames_Implementation() const
{
	if (ActionManagerComponent)
	{
		return ActionManagerComponent->HasIFrames();
	}
	return false;
}

void AGameCharacterBase::AddCombatStateTag_Implementation(const FGameplayTag& Tag)
{
	if (ActionManagerComponent)
	{
		ActionManagerComponent->AddCombatStateTag(Tag);
	}
}

void AGameCharacterBase::RemoveCombatStateTag_Implementation(const FGameplayTag& Tag)
{
	if (ActionManagerComponent)
	{
		ActionManagerComponent->RemoveCombatStateTag(Tag);
	}
}

bool AGameCharacterBase::HasCombatStateTag_Implementation(const FGameplayTag& Tag) const
{
	if (ActionManagerComponent)
	{
		return ActionManagerComponent->HasCombatStateTag(Tag);
	}
	return false;
}

bool AGameCharacterBase::StartAttack_Implementation(const FGameplayTag& AttackTag)
{
	// Deprecated - attacks are handled through action slots now
	return false;
}

void AGameCharacterBase::EndAttack_Implementation()
{
	// Deprecated - attacks are handled through action slots now
}

bool AGameCharacterBase::StartBlock_Implementation()
{
	if (ActionManagerComponent)
	{
		return ActionManagerComponent->StartBlock();
	}
	return false;
}

void AGameCharacterBase::EndBlock_Implementation()
{
	if (ActionManagerComponent)
	{
		ActionManagerComponent->EndBlock();
	}
}

// IHealthInterface Implementation
float AGameCharacterBase::GetCurrentHealth_Implementation() const
{
	if (HealthComponent)
	{
		return HealthComponent->GetCurrentHealth();
	}
	return 0.0f;
}

float AGameCharacterBase::GetMaxHealth_Implementation() const
{
	if (HealthComponent)
	{
		return HealthComponent->GetMaxHealth();
	}
	return 100.0f;
}

float AGameCharacterBase::GetHealthPercent_Implementation() const
{
	if (HealthComponent)
	{
		return HealthComponent->GetHealthPercent();
	}
	return 1.0f;
}

bool AGameCharacterBase::IsAlive_Implementation() const
{
	if (HealthComponent)
	{
		return HealthComponent->IsAlive();
	}
	return true;
}

bool AGameCharacterBase::IsDead_Implementation() const
{
	if (HealthComponent)
	{
		return !HealthComponent->IsAlive();
	}
	return false;
}

void AGameCharacterBase::ApplyDamage_Implementation(float DamageAmount, AActor* DamageInstigator)
{
	if (HealthComponent)
	{
		HealthComponent->TakeDamage(DamageAmount, DamageInstigator);
	}
}

void AGameCharacterBase::ApplyHealing_Implementation(float HealAmount, AActor* Healer)
{
	if (HealthComponent)
	{
		HealthComponent->Heal(HealAmount, Healer);
	}
}

// IActionInterface Implementation
bool AGameCharacterBase::CanPerformAction_Implementation(const FGameplayTag& ActionTag) const
{
	// Delegate to action manager or check conditions
	return true;
}

bool AGameCharacterBase::TryPerformAction_Implementation(const FGameplayTag& ActionTag)
{
	if (ActionManagerComponent)
	{
		// Find which slot has this action and trigger it
		TArray<FName> Slots = ActionManagerComponent->GetAllSlotNames();
		for (const FName& Slot : Slots)
		{
			if (UBaseAction* Action = ActionManagerComponent->GetActionInSlot(Slot))
			{
				if (Action->GetActionTag() == ActionTag)
				{
					ActionManagerComponent->OnSlotPressed(Slot);
					return true;
				}
			}
		}
	}
	return false;
}

void AGameCharacterBase::InterruptCurrentAction_Implementation()
{
	if (ActionManagerComponent)
	{
		ActionManagerComponent->InterruptCurrentAction();
	}
}

bool AGameCharacterBase::IsPerformingAction_Implementation() const
{
	if (ActionManagerComponent)
	{
		return ActionManagerComponent->IsActionActive();
	}
	return false;
}

UBaseAction* AGameCharacterBase::GetCurrentAction_Implementation() const
{
	if (ActionManagerComponent)
	{
		return ActionManagerComponent->GetCurrentAction();
	}
	return nullptr;
}

bool AGameCharacterBase::AssignActionToSlot_Implementation(FName SlotName, const FGameplayTag& ActionTag)
{
	if (ActionManagerComponent)
	{
		return ActionManagerComponent->AssignActionToSlot(SlotName, ActionTag);
	}
	return false;
}

void AGameCharacterBase::ClearActionSlot_Implementation(FName SlotName)
{
	if (ActionManagerComponent)
	{
		ActionManagerComponent->ClearSlot(SlotName);
	}
}

UBaseAction* AGameCharacterBase::GetActionInSlot_Implementation(FName SlotName) const
{
	if (ActionManagerComponent)
	{
		return ActionManagerComponent->GetActionInSlot(SlotName);
	}
	return nullptr;
}