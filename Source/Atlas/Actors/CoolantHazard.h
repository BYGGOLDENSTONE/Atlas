#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Abilities/CoolantSprayAbility.h"
#include "CoolantHazard.generated.h"

UCLASS()
class ATLAS_API ACoolantHazard : public AActor
{
	GENERATED_BODY()

public:
	ACoolantHazard();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void Initialize(const FCoolantSprayConfig& Config, AActor* InInstigator);

protected:
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void ApplySlipEffect(class ACharacter* Character);
	void RemoveSlipEffect(class ACharacter* Character);
	void UpdateAffectedActors(float DeltaTime);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* HazardCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UDecalComponent* HazardDecal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UParticleSystemComponent* HazardParticles;

private:
	FCoolantSprayConfig SprayConfig;
	
	UPROPERTY()
	AActor* HazardInstigator;

	UPROPERTY()
	TMap<ACharacter*, float> AffectedCharacters;

	float RemainingDuration;
	float TickTimer;
};