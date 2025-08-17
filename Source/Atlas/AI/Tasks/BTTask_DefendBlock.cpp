#include "BTTask_DefendBlock.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Characters/EnemyCharacter.h"
#include "../../Components/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UBTTask_DefendBlock::UBTTask_DefendBlock()
{
	NodeName = "Defend/Block";
	bNotifyTick = true;
	TimeElapsed = 0.0f;
	bIsBlocking = false;
	bParryAttempted = false;
}

EBTNodeResult::Type UBTTask_DefendBlock::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(AIController->GetPawn());
	if (!Enemy)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetActor")));
	if (!Target)
	{
		return EBTNodeResult::Failed;
	}

	TimeElapsed = 0.0f;
	bParryAttempted = false;

	EDefenseType SelectedDefense = DefenseType;
	if (DefenseType == EDefenseType::Auto)
	{
		SelectedDefense = DecideDefenseType(BlackboardComp);
	}

	switch (SelectedDefense)
	{
	case EDefenseType::Block:
		StartBlock(Enemy);
		return EBTNodeResult::InProgress;
		
	case EDefenseType::Parry:
		if (AttemptParry(Enemy))
		{
			return EBTNodeResult::Succeeded;
		}
		StartBlock(Enemy);
		return EBTNodeResult::InProgress;
		
	case EDefenseType::Dodge:
		PerformDodge(Enemy, Target);
		return EBTNodeResult::Succeeded;
		
	default:
		return EBTNodeResult::Failed;
	}
}

void UBTTask_DefendBlock::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	TimeElapsed += DeltaSeconds;

	if (TimeElapsed >= BlockDuration)
	{
		AAIController* AIController = OwnerComp.GetAIOwner();
		if (AIController)
		{
			AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(AIController->GetPawn());
			if (Enemy)
			{
				StopBlock(Enemy);
			}
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_DefendBlock::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(AIController->GetPawn());
		if (Enemy && bIsBlocking)
		{
			StopBlock(Enemy);
		}
	}
	return EBTNodeResult::Aborted;
}

void UBTTask_DefendBlock::StartBlock(AEnemyCharacter* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	UCombatComponent* CombatComp = Enemy->GetCombatComponent();
	if (CombatComp)
	{
		CombatComp->StartBlocking();
		bIsBlocking = true;
	}
}

void UBTTask_DefendBlock::StopBlock(AEnemyCharacter* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	UCombatComponent* CombatComp = Enemy->GetCombatComponent();
	if (CombatComp)
	{
		CombatComp->StopBlocking();
		bIsBlocking = false;
	}
}

bool UBTTask_DefendBlock::AttemptParry(AEnemyCharacter* Enemy)
{
	if (!Enemy || bParryAttempted)
	{
		return false;
	}

	bParryAttempted = true;
	
	float RandomValue = FMath::FRand();
	if (RandomValue <= ParrySuccessChance)
	{
		UCombatComponent* CombatComp = Enemy->GetCombatComponent();
		if (CombatComp)
		{
			CombatComp->AttemptParry();
			
			FTimerHandle ParryTimer;
			Enemy->GetWorld()->GetTimerManager().SetTimer(
				ParryTimer,
				[CombatComp]()
				{
					CombatComp->EndParryWindow();
				},
				ParryWindow,
				false
			);
			return true;
		}
	}
	
	return false;
}

void UBTTask_DefendBlock::PerformDodge(AEnemyCharacter* Enemy, AActor* Threat)
{
	if (!Enemy || !Threat)
	{
		return;
	}

	FVector DodgeDirection = (Enemy->GetActorLocation() - Threat->GetActorLocation()).GetSafeNormal();
	
	int32 DodgeChoice = FMath::RandRange(0, 2);
	switch (DodgeChoice)
	{
	case 0:
		DodgeDirection = FVector::CrossProduct(DodgeDirection, FVector::UpVector);
		break;
	case 1:
		DodgeDirection = -FVector::CrossProduct(DodgeDirection, FVector::UpVector);
		break;
	default:
		break;
	}

	FVector DodgeLocation = Enemy->GetActorLocation() + (DodgeDirection * DodgeDistance);
	Enemy->GetCharacterMovement()->Velocity = DodgeDirection * DodgeDistance * 3.0f;
}

EDefenseType UBTTask_DefendBlock::DecideDefenseType(UBlackboardComponent* BlackboardComp)
{
	if (!BlackboardComp)
	{
		return EDefenseType::Block;
	}

	float DistanceToTarget = BlackboardComp->GetValueAsFloat(FName("DistanceToTarget"));
	bool bIsUnderPressure = BlackboardComp->GetValueAsBool(FName("IsUnderPressure"));
	
	if (bIsUnderPressure && DistanceToTarget < 150.0f)
	{
		return EDefenseType::Dodge;
	}
	
	if (DistanceToTarget < 250.0f && FMath::FRand() > 0.5f)
	{
		return EDefenseType::Parry;
	}
	
	return EDefenseType::Block;
}