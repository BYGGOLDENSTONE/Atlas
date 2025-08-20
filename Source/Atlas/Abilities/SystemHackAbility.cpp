#include "SystemHackAbility.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "../Data/SystemHackDataAsset.h"
#include "../Interfaces/IInteractable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

USystemHackAbility::USystemHackAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	AbilityName = "Basic System Hack";
	AbilityDescription = "Remotely hack and interact with systems and terminals";
	Cooldown = 3.0f;
	ExecutionDuration = 1.5f;
	RiskTier = EAbilityRiskTier::LowRisk;
	IntegrityCost = 0.0f;

	HackProgress = 0.0f;
	bIsHacking = false;
}

void USystemHackAbility::BeginPlay()
{
	Super::BeginPlay();
}

void USystemHackAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsHacking && CurrentHackTarget)
	{
		FSystemHackConfig Config = GetHackConfig();
		
		HackProgress += DeltaTime;
		
		// Check if hack is complete
		if (HackProgress >= Config.HackDuration)
		{
			CompleteHack();
		}
		else
		{
			// Visual feedback during hack
			if (GEngine && GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer)
			{
				FVector StartPos = GetOwnerCharacter()->GetActorLocation() + FVector(0, 0, 50);
				FVector EndPos = CurrentHackTarget->GetActorLocation();
				
				// Draw animated line
				float Alpha = HackProgress / Config.HackDuration;
				FLinearColor LinearColor = FLinearColor::LerpUsingHSV(FLinearColor::Green, FLinearColor(0.0f, 1.0f, 1.0f), Alpha);
				FColor LineColor = LinearColor.ToFColor(true);
				DrawDebugLine(GetWorld(), StartPos, EndPos, LineColor, false, 0.0f, 0, 2.0f);
				
				// Draw progress indicator
				DrawDebugString(GetWorld(), EndPos + FVector(0, 0, 100), 
					FString::Printf(TEXT("Hacking: %.0f%%"), Alpha * 100.0f),
					nullptr, FColor::Green, 0.0f);
			}
		}

		// Check if target is still valid
		if (!IsValidHackTarget(CurrentHackTarget))
		{
			CancelHack();
		}
	}
}

void USystemHackAbility::ExecuteAbility()
{
	Super::ExecuteAbility();

	AActor* Target = FindBestHackTarget();
	if (Target)
	{
		StartHacking(Target);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("System Hack: No valid targets found"));
		OnAbilityEnd();
	}
}

void USystemHackAbility::OnAbilityEnd()
{
	if (bIsHacking)
	{
		CancelHack();
	}
	
	Super::OnAbilityEnd();
}

bool USystemHackAbility::CheckAbilitySpecificConditions() const
{
	// Could add checks for player having "hacking tool" or similar
	return true;
}

AActor* USystemHackAbility::FindBestHackTarget() const
{
	ACharacter* Owner = GetOwnerCharacter();
	if (!Owner)
	{
		return nullptr;
	}

	FSystemHackConfig Config = GetHackConfig();
	FVector OwnerLocation = Owner->GetActorLocation();
	FVector OwnerForward = Owner->GetActorForwardVector();

	// Find all potential targets in range
	TArray<AActor*> PotentialTargets;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UInteractable::StaticClass(), PotentialTargets);

	AActor* BestTarget = nullptr;
	float BestScore = -1.0f;

	for (AActor* Target : PotentialTargets)
	{
		if (!IsValidHackTarget(Target))
		{
			continue;
		}

		float Distance = FVector::Dist(OwnerLocation, Target->GetActorLocation());
		if (Distance > Config.HackRange)
		{
			continue;
		}

		// Check angle
		FVector ToTarget = (Target->GetActorLocation() - OwnerLocation).GetSafeNormal();
		float Angle = FMath::Acos(FVector::DotProduct(OwnerForward, ToTarget));
		float AngleDegrees = FMath::RadiansToDegrees(Angle);

		if (AngleDegrees > Config.HackAngle)
		{
			continue;
		}

		// Check line of sight if required
		if (Config.bRequireLineOfSight && !HasLineOfSight(Target))
		{
			continue;
		}

		// Calculate score (prefer closer targets in center of view)
		float DistanceScore = 1.0f - (Distance / Config.HackRange);
		float AngleScore = 1.0f - (AngleDegrees / Config.HackAngle);
		float Score = DistanceScore * 0.6f + AngleScore * 0.4f;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

bool USystemHackAbility::IsValidHackTarget(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	// Check if it implements the interactable interface
	if (!Target->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		return false;
	}

	// Check if it can be interacted with
	IInteractable* Interactable = Cast<IInteractable>(Target);
	if (!Interactable || !Interactable->CanInteract(GetOwner()))
	{
		return false;
	}

	// Check hackable tags if specified
	FSystemHackConfig Config = GetHackConfig();
	if (Config.HackableTags.Num() > 0)
	{
		bool bHasValidTag = false;
		for (const FName& Tag : Config.HackableTags)
		{
			if (Target->Tags.Contains(Tag))
			{
				bHasValidTag = true;
				break;
			}
		}
		if (!bHasValidTag)
		{
			return false;
		}
	}

	return true;
}

bool USystemHackAbility::HasLineOfSight(AActor* Target) const
{
	if (!Target || !GetOwnerCharacter())
	{
		return false;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(Target);

	FVector StartPos = GetOwnerCharacter()->GetActorLocation() + FVector(0, 0, 50);
	FVector EndPos = Target->GetActorLocation();

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartPos,
		EndPos,
		ECC_Visibility,
		QueryParams
	);

	return !bHit;
}

void USystemHackAbility::StartHacking(AActor* Target)
{
	CurrentHackTarget = Target;
	bIsHacking = true;
	HackProgress = 0.0f;

	FSystemHackConfig Config = GetHackConfig();
	ExecutionDuration = Config.HackDuration;

	// Notify target that hacking started (optional)
	if (IInteractable* Interactable = Cast<IInteractable>(Target))
	{
		// Could add a StartHack function to the interface
	}

	UE_LOG(LogTemp, Warning, TEXT("System Hack: Started hacking %s"), *Target->GetName());

	// Visual feedback
	if (GEngine && GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer)
	{
		DrawDebugSphere(GetWorld(), Target->GetActorLocation(), 50.0f, 12, FColor::Green, false, Config.HackDuration);
	}
}

void USystemHackAbility::CompleteHack()
{
	if (!CurrentHackTarget)
	{
		return;
	}

	// Trigger the interaction
	if (IInteractable* Interactable = Cast<IInteractable>(CurrentHackTarget))
	{
		Interactable->Interact(GetOwner());
		
		// Override cooldown if configured
		FSystemHackConfig Config = GetHackConfig();
		if (Config.bOverrideInteractCooldown)
		{
			// This would need to be implemented in the Interactable interface
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("System Hack: Successfully hacked %s"), *CurrentHackTarget->GetName());

	// Visual feedback
	if (GEngine && GEngine->GetNetMode(GetWorld()) != NM_DedicatedServer)
	{
		DrawDebugSphere(GetWorld(), CurrentHackTarget->GetActorLocation(), 80.0f, 12, FColor::Cyan, false, 1.0f);
	}

	CurrentHackTarget = nullptr;
	bIsHacking = false;
	HackProgress = 0.0f;

	OnAbilityEnd();
}

void USystemHackAbility::CancelHack()
{
	if (CurrentHackTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("System Hack: Hack cancelled for %s"), *CurrentHackTarget->GetName());
	}

	CurrentHackTarget = nullptr;
	bIsHacking = false;
	HackProgress = 0.0f;
}

FSystemHackConfig USystemHackAbility::GetHackConfig() const
{
	if (HackDataAsset)
	{
		return HackDataAsset->HackConfig;
	}
	return DefaultConfig;
}