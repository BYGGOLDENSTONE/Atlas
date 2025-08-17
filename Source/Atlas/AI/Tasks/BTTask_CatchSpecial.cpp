#include "BTTask_CatchSpecial.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Characters/EnemyCharacter.h"
#include "../../Components/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UBTTask_CatchSpecial::UBTTask_CatchSpecial()
{
	NodeName = "Catch Special (Anti-Kiting)";
	bNotifyTick = true;
	DashTimeElapsed = 0.0f;
	bDashComplete = false;
}

EBTNodeResult::Type UBTTask_CatchSpecial::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	float Distance = FVector::Dist(Enemy->GetActorLocation(), Target->GetActorLocation());
	if (Distance > MaxDashDistance || Distance < 200.0f)
	{
		return EBTNodeResult::Failed;
	}

	bool bTargetIsKiting = BlackboardComp->GetValueAsBool(FName("TargetIsKiting"));
	if (!bTargetIsKiting && Distance < 500.0f)
	{
		return EBTNodeResult::Failed;
	}

	CachedEnemy = Enemy;
	CachedTarget = Target;
	DashTimeElapsed = 0.0f;
	bDashComplete = false;

	InitiateDash(Enemy, Target);
	return EBTNodeResult::InProgress;
}

void UBTTask_CatchSpecial::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (bDashComplete)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	DashTimeElapsed += DeltaSeconds;

	if (!CachedEnemy || !CachedTarget)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	float DashDuration = MaxDashDistance / DashSpeed;
	float Alpha = FMath::Clamp(DashTimeElapsed / DashDuration, 0.0f, 1.0f);
	
	FVector CurrentLocation = FMath::Lerp(StartLocation, TargetLocation, Alpha);
	CachedEnemy->SetActorLocation(CurrentLocation, true);

	CheckCatchCollision();

	if (Alpha >= 1.0f)
	{
		bDashComplete = true;
		CachedEnemy->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	}
}

void UBTTask_CatchSpecial::InitiateDash(AEnemyCharacter* Enemy, AActor* Target)
{
	if (!Enemy || !Target)
	{
		return;
	}

	StartLocation = Enemy->GetActorLocation();
	FVector Direction = (Target->GetActorLocation() - StartLocation).GetSafeNormal();
	float DistanceToTarget = FVector::Dist(StartLocation, Target->GetActorLocation());
	float DashDistance = FMath::Min(DistanceToTarget + 100.0f, MaxDashDistance);
	
	TargetLocation = StartLocation + (Direction * DashDistance);

	Enemy->SetActorRotation(Direction.Rotation());

	UAnimInstance* AnimInstance = Enemy->GetMesh()->GetAnimInstance();
	if (AnimInstance && DashMontage)
	{
		AnimInstance->Montage_Play(DashMontage);
	}

	Enemy->GetCharacterMovement()->Velocity = Direction * DashSpeed;
}

void UBTTask_CatchSpecial::CheckCatchCollision()
{
	if (!CachedEnemy || !CachedTarget || bDashComplete)
	{
		return;
	}

	float DistanceToTarget = FVector::Dist(CachedEnemy->GetActorLocation(), CachedTarget->GetActorLocation());
	
	if (DistanceToTarget <= CatchRadius)
	{
		OnCatchSuccess(CachedTarget);
		bDashComplete = true;
	}
}

void UBTTask_CatchSpecial::OnCatchSuccess(AActor* CaughtTarget)
{
	if (!CaughtTarget || !CachedEnemy)
	{
		return;
	}

	ACharacter* TargetCharacter = Cast<ACharacter>(CaughtTarget);
	if (!TargetCharacter)
	{
		return;
	}

	TargetCharacter->GetCharacterMovement()->DisableMovement();
	
	UCombatComponent* EnemyCombat = CachedEnemy->GetCombatComponent();
	if (EnemyCombat)
	{
		FGameplayTagContainer CatchTags;
		CatchTags.AddTag(FGameplayTag::RequestGameplayTag("Combat.Status.Stunned"));
		EnemyCombat->DealDamageToTarget(CaughtTarget, 10.0f, CatchTags);
	}

	FTimerHandle StunTimer;
	CachedEnemy->GetWorld()->GetTimerManager().SetTimer(
		StunTimer,
		[TargetCharacter]()
		{
			if (TargetCharacter)
			{
				TargetCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			}
		},
		StunDuration,
		false
	);

	UAnimInstance* AnimInstance = CachedEnemy->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.2f);
	}
}