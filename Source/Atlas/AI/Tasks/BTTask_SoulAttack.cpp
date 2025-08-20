#include "BTTask_SoulAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Characters/EnemyCharacter.h"
#include "../../Components/CombatComponent.h"
#include "../../Components/HitboxComponent.h"
#include "../EnemyAIController.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "GameplayTagContainer.h"

UBTTask_SoulAttack::UBTTask_SoulAttack()
{
	NodeName = "Soul Attack (Unblockable)";
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_SoulAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyAIController* AIController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	if (!AIController || !AIController->CanUseSoulAttack())
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
	if (Distance > AttackRange)
	{
		return EBTNodeResult::Failed;
	}

	FVector DirectionToTarget = (Target->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
	Enemy->SetActorRotation(DirectionToTarget.Rotation());

	UAnimInstance* AnimInstance = Enemy->GetMesh()->GetAnimInstance();
	if (AnimInstance && SoulAttackMontage)
	{
		AnimInstance->Montage_Play(SoulAttackMontage);
	}

	FTimerHandle ChargeTimer;
	Enemy->GetWorld()->GetTimerManager().SetTimer(
		ChargeTimer,
		[this, Enemy, Target, AIController]()
		{
			ExecuteSoulAttack(Enemy, Target);
			AIController->StartSoulAttackCooldown();
		},
		ChargeTime,
		false
	);

	return EBTNodeResult::Succeeded;
}

void UBTTask_SoulAttack::ExecuteSoulAttack(AEnemyCharacter* Enemy, AActor* Target)
{
	if (!Enemy || !Target)
	{
		return;
	}

	UCombatComponent* CombatComp = Enemy->GetCombatComponent();
	if (!CombatComp)
	{
		return;
	}

	USphereComponent* SoulAttackHitbox = NewObject<USphereComponent>(Enemy, USphereComponent::StaticClass());
	if (!SoulAttackHitbox)
	{
		return;
	}

	SoulAttackHitbox->SetupAttachment(Enemy->GetRootComponent());
	SoulAttackHitbox->SetSphereRadius(AttackRange);
	SoulAttackHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SoulAttackHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	SoulAttackHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SoulAttackHitbox->RegisterComponent();

	TArray<AActor*> OverlappingActors;
	SoulAttackHitbox->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (AActor* HitActor : OverlappingActors)
	{
		if (HitActor != Enemy && HitActor == Target)
		{
			// Soul Attack uses heavy attack action
			// The ability's DataAsset should define whether it's blockable
			FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag("Action.HeavyAttack");
			CombatComp->ProcessHit(HitActor, AttackTag);
			
			FVector PushDirection = (HitActor->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal();
			if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
			{
				HitCharacter->LaunchCharacter(PushDirection * 1000.0f, true, true);
			}
			break;
		}
	}

	SoulAttackHitbox->DestroyComponent();
}