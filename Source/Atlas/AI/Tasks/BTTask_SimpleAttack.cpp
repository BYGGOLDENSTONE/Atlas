#include "BTTask_SimpleAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Characters/EnemyCharacter.h"
#include "../../Components/CombatComponent.h"
#include "../../Components/HealthComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

UBTTask_SimpleAttack::UBTTask_SimpleAttack()
{
	NodeName = "Simple Attack (Test)";
}

EBTNodeResult::Type UBTTask_SimpleAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetActor")));
	if (!Target)
	{
		return EBTNodeResult::Failed;
	}

	float Distance = FVector::Dist(Enemy->GetActorLocation(), Target->GetActorLocation());
	if (Distance > AttackRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("SimpleAttack: Out of range. Distance: %f, Range: %f"), Distance, AttackRange);
		return EBTNodeResult::Failed;
	}

	// Face target
	FVector DirectionToTarget = (Target->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
	Enemy->SetActorRotation(DirectionToTarget.Rotation());

	// Direct damage application for testing
	UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();
	if (TargetHealth)
	{
		TargetHealth->TakeDamage(Damage, Enemy);
		UE_LOG(LogTemp, Warning, TEXT("SimpleAttack: SUCCESS! Dealt %f damage to target"), Damage);
		
		// Visual feedback
		DrawDebugLine(
			Enemy->GetWorld(),
			Enemy->GetActorLocation() + FVector(0, 0, 50),
			Target->GetActorLocation() + FVector(0, 0, 50),
			FColor::Red,
			false,
			1.0f,
			0,
			5.0f
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SimpleAttack: Target has no HealthComponent!"));
	}

	// Play simple attack animation if available
	UCombatComponent* CombatComp = Enemy->GetCombatComponent();
	if (CombatComp)
	{
		// Trigger attack state using the correct tag
		CombatComp->StartAttack(FGameplayTag::RequestGameplayTag("Attack.Type.Jab"));
		
		// End attack after delay
		FTimerHandle EndAttackTimer;
		Enemy->GetWorld()->GetTimerManager().SetTimer(
			EndAttackTimer,
			[CombatComp]()
			{
				if (CombatComp)
				{
					CombatComp->EndAttack();
				}
			},
			0.5f,
			false
		);
	}

	return EBTNodeResult::Succeeded;
}