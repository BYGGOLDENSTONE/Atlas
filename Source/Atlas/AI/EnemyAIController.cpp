#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "../Characters/EnemyCharacter.h"
#include "../Characters/PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerceptionComponent);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SetupPerceptionSystem();

	bWantsPlayerState = true;
	bSetControlRotationFromPawnOrientation = false;
}

void AEnemyAIController::SetupPerceptionSystem()
{
	if (SightConfig)
	{
		SightConfig->SightRadius = 2000.0f;
		SightConfig->LoseSightRadius = 2500.0f;
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;
		SightConfig->SetMaxAge(5.0f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
		
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

		if (AIPerceptionComponent)
		{
			AIPerceptionComponent->ConfigureSense(*SightConfig);
			AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
			AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
		}
	}
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		InitializeBlackboardData();

		if (BehaviorTree)
		{
			RunBehaviorTree(BehaviorTree);
		}
	}
}

void AEnemyAIController::OnUnPossess()
{
	Super::OnUnPossess();

	if (BehaviorTree)
	{
		UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
		if (BTComponent)
		{
			BTComponent->StopTree(EBTStopMode::Safe);
		}
	}
}

void AEnemyAIController::InitializeBlackboardData()
{
	if (BlackboardData && Blackboard)
	{
		Blackboard->InitializeBlackboard(*BlackboardData);
		
		Blackboard->SetValueAsFloat(FName("AttackRange"), AttackRange);
		Blackboard->SetValueAsFloat(FName("DefendRange"), DefendRange);
		Blackboard->SetValueAsFloat(FName("CatchSpecialRange"), CatchSpecialRange);
		Blackboard->SetValueAsBool(FName("CanUseSoulAttack"), true);
		Blackboard->SetValueAsBool(FName("IsInCombat"), false);
		Blackboard->SetValueAsObject(FName("SelfActor"), GetPawn());
	}
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !Blackboard)
	{
		return;
	}

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Actor);
	if (!PlayerCharacter)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		Blackboard->SetValueAsObject(FName("TargetActor"), Actor);
		Blackboard->SetValueAsVector(FName("LastKnownLocation"), Actor->GetActorLocation());
		Blackboard->SetValueAsBool(FName("IsInCombat"), true);
		
		float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
		Blackboard->SetValueAsFloat(FName("DistanceToTarget"), Distance);
	}
	else
	{
		Blackboard->SetValueAsVector(FName("LastKnownLocation"), Stimulus.StimulusLocation);
		
		GetWorldTimerManager().SetTimer(
			LostTargetTimer,
			[this]()
			{
				if (Blackboard && !Blackboard->GetValueAsObject(FName("TargetActor")))
				{
					Blackboard->SetValueAsBool(FName("IsInCombat"), false);
				}
			},
			3.0f,
			false
		);
	}
}

void AEnemyAIController::StartSoulAttackCooldown()
{
	bCanUseSoulAttack = false;
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(FName("CanUseSoulAttack"), false);
	}

	GetWorldTimerManager().SetTimer(
		SoulAttackCooldownTimer,
		this,
		&AEnemyAIController::ResetSoulAttackCooldown,
		SoulAttackCooldown,
		false
	);
}

void AEnemyAIController::ResetSoulAttackCooldown()
{
	bCanUseSoulAttack = true;
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(FName("CanUseSoulAttack"), true);
	}
}