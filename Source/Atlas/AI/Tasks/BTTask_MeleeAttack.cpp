#include "BTTask_MeleeAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Characters/EnemyCharacter.h"
#include "../../Components/CombatComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Engine/World.h"

UBTTask_MeleeAttack::UBTTask_MeleeAttack()
{
	NodeName = "Melee Attack";
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_MeleeAttack: No AI Controller"));
		return EBTNodeResult::Failed;
	}

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(AIController->GetPawn());
	if (!Enemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_MeleeAttack: No Enemy Character"));
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_MeleeAttack: No Blackboard"));
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetActor")));
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_MeleeAttack: No Target"));
		return EBTNodeResult::Failed;
	}
	
	if (!IsInAttackRange(AIController, Target))
	{
		// Enemy is out of range - this is normal behavior, no need to log
		return EBTNodeResult::Failed;
	}

	UCombatComponent* CombatComp = Enemy->GetCombatComponent();
	if (!CombatComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_MeleeAttack: No Combat Component"));
		return EBTNodeResult::Failed;
	}

	EAttackType SelectedAttack = AttackType;
	if (AttackType == EAttackType::Random)
	{
		int32 RandomChoice = FMath::RandRange(0, 2);
		switch (RandomChoice)
		{
		case 0:
			SelectedAttack = EAttackType::Jab;
			break;
		case 1:
			SelectedAttack = EAttackType::Heavy;
			break;
		case 2:
			SelectedAttack = EAttackType::Combo;
			break;
		}
	}

	FVector DirectionToTarget = (Target->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
	Enemy->SetActorRotation(DirectionToTarget.Rotation());

	// Execute the selected attack type

	if (SelectedAttack == EAttackType::Combo)
	{
		PerformCombo(Enemy);
	}
	else
	{
		PerformAttack(Enemy, SelectedAttack);
	}

	return EBTNodeResult::Succeeded;
}

bool UBTTask_MeleeAttack::IsInAttackRange(AAIController* AIController, AActor* Target) const
{
	if (!AIController || !Target)
	{
		return false;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return false;
	}

	float Distance = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());
	return Distance <= AttackRange;
}

void UBTTask_MeleeAttack::PerformAttack(AEnemyCharacter* Enemy, EAttackType Type)
{
	if (!Enemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformAttack: No Enemy"));
		return;
	}

	UCombatComponent* CombatComp = Enemy->GetCombatComponent();
	if (!CombatComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformAttack: No CombatComponent"));
		return;
	}

	// Define attack tags - these should match your AttackDataAssets
	FGameplayTag AttackTag;
	switch (Type)
	{
	case EAttackType::Jab:
		AttackTag = FGameplayTag::RequestGameplayTag(FName("Attack.Type.Jab"), false);
		// Jab attack
		break;
	case EAttackType::Heavy:
		AttackTag = FGameplayTag::RequestGameplayTag(FName("Attack.Type.Heavy"), false);
		// Heavy attack
		break;
	default:
		AttackTag = FGameplayTag::RequestGameplayTag(FName("Attack.Type.Jab"), false);
		break;
	}

	// Check if the tag is valid
	if (!AttackTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("PerformAttack: Invalid attack tag! Make sure gameplay tags are registered"));
		return;
	}

	// Start the attack through CombatComponent which will handle the montage from AttackDataAsset
	bool bSuccess = CombatComp->StartAttack(AttackTag);
	// Attack started through CombatComponent
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformAttack: Failed to start attack. Check if AttackDataAsset is configured for tag: %s"), *AttackTag.ToString());
	}
}

void UBTTask_MeleeAttack::PerformCombo(AEnemyCharacter* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	for (int32 i = 0; i < ComboCount; ++i)
	{
		FTimerHandle ComboTimer;
		float Delay = i * ComboDelay;
		
		Enemy->GetWorld()->GetTimerManager().SetTimer(
			ComboTimer,
			[this, Enemy, i]()
			{
				EAttackType ComboAttack = (i == ComboCount - 1) ? EAttackType::Heavy : EAttackType::Jab;
				PerformAttack(Enemy, ComboAttack);
			},
			Delay,
			false
		);
	}
}