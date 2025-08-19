#include "AnimationManagerComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

UAnimationManagerComponent::UAnimationManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAnimationManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter && OwnerCharacter->GetMesh())
    {
        AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
    }
}

bool UAnimationManagerComponent::PlayAnimationByTag(const FGameplayTag& AnimationTag, float PlayRate, FName StartSection)
{
    UAnimMontage* Montage = GetMontageByTag(AnimationTag);
    if (!Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("No montage found for tag: %s"), *AnimationTag.ToString());
        return false;
    }
    
    FAnimationRequest Request;
    Request.Montage = Montage;
    Request.AnimationTag = AnimationTag;
    Request.PlayRate = PlayRate;
    Request.StartSection = StartSection;
    
    return PlayAnimationRequest(Request);
}

bool UAnimationManagerComponent::PlayMontage(UAnimMontage* Montage, float PlayRate, FName StartSection)
{
    if (!Montage)
    {
        return false;
    }
    
    FAnimationRequest Request;
    Request.Montage = Montage;
    Request.PlayRate = PlayRate;
    Request.StartSection = StartSection;
    
    return PlayAnimationRequest(Request);
}

bool UAnimationManagerComponent::PlayAnimationRequest(const FAnimationRequest& Request)
{
    if (!AnimInstance || !Request.Montage)
    {
        return false;
    }
    
    if (!CanPlayAnimation(Request))
    {
        return false;
    }
    
    if (Request.bStopAllMontages)
    {
        StopAllMontages(0.1f);
    }
    
    float MontageLength = AnimInstance->Montage_Play(Request.Montage, Request.PlayRate, 
        EMontagePlayReturnType::MontageLength, Request.StartPosition);
    
    if (MontageLength > 0.0f)
    {
        if (Request.StartSection != NAME_None)
        {
            AnimInstance->Montage_JumpToSection(Request.StartSection, Request.Montage);
        }
        
        CurrentRequest = Request;
        CurrentPriority = Request.Priority;
        bCanInterruptCurrent = Request.bCanBeInterrupted;
        
        BindMontageEvents(Request.Montage);
        UpdateAnimationState(true, Request.Montage, Request.AnimationTag);
        
        OnAnimationMontageStarted.Broadcast(Request.Montage, Request.AnimationTag);
        return true;
    }
    
    return false;
}

void UAnimationManagerComponent::StopCurrentMontage(float BlendOutTime)
{
    if (!AnimInstance || !CurrentMontage)
    {
        return;
    }
    
    AnimInstance->Montage_Stop(BlendOutTime, CurrentMontage);
    UpdateAnimationState(false);
}

void UAnimationManagerComponent::StopAllMontages(float BlendOutTime)
{
    if (!AnimInstance)
    {
        return;
    }
    
    AnimInstance->StopAllMontages(BlendOutTime);
    UpdateAnimationState(false);
}

bool UAnimationManagerComponent::IsPlayingAnimationTag(const FGameplayTag& AnimationTag) const
{
    return bIsPlayingAnimation && CurrentAnimationTag == AnimationTag;
}

float UAnimationManagerComponent::GetCurrentMontagePosition() const
{
    if (!AnimInstance || !CurrentMontage)
    {
        return 0.0f;
    }
    
    return AnimInstance->Montage_GetPosition(CurrentMontage);
}

float UAnimationManagerComponent::GetCurrentMontageLength() const
{
    if (!CurrentMontage)
    {
        return 0.0f;
    }
    
    return CurrentMontage->GetPlayLength();
}

void UAnimationManagerComponent::SetMontagePosition(float Position)
{
    if (!AnimInstance || !CurrentMontage)
    {
        return;
    }
    
    AnimInstance->Montage_SetPosition(CurrentMontage, Position);
}

void UAnimationManagerComponent::SetPlayRate(float PlayRate)
{
    if (!AnimInstance || !CurrentMontage)
    {
        return;
    }
    
    AnimInstance->Montage_SetPlayRate(CurrentMontage, PlayRate);
}

void UAnimationManagerComponent::PauseMontage()
{
    if (!AnimInstance || !CurrentMontage)
    {
        return;
    }
    
    AnimInstance->Montage_Pause(CurrentMontage);
}

void UAnimationManagerComponent::ResumeMontage()
{
    if (!AnimInstance || !CurrentMontage)
    {
        return;
    }
    
    AnimInstance->Montage_Resume(CurrentMontage);
}

bool UAnimationManagerComponent::CanInterruptCurrentAnimation() const
{
    return !bIsPlayingAnimation || bCanInterruptCurrent;
}

void UAnimationManagerComponent::RegisterMontage(const FGameplayTag& AnimationTag, UAnimMontage* Montage)
{
    if (Montage)
    {
        MontageMap.Add(AnimationTag, Montage);
    }
}

UAnimMontage* UAnimationManagerComponent::GetMontageByTag(const FGameplayTag& AnimationTag) const
{
    if (MontageMap.Contains(AnimationTag))
    {
        return MontageMap[AnimationTag];
    }
    return nullptr;
}

void UAnimationManagerComponent::HandleHitReaction(float Damage, const FVector& HitDirection)
{
    FGameplayTag HitTag = Damage > 50.0f ? 
        FGameplayTag::RequestGameplayTag("Animation.HitReaction.Heavy") : 
        FGameplayTag::RequestGameplayTag("Animation.HitReaction.Light");
    
    PlayAnimationByTag(HitTag, 1.0f);
}

void UAnimationManagerComponent::HandleStagger(float Duration)
{
    FGameplayTag StaggerTag = FGameplayTag::RequestGameplayTag("Animation.Stagger");
    PlayAnimationByTag(StaggerTag, 1.0f);
}

void UAnimationManagerComponent::HandleBlock(bool bSuccess)
{
    FGameplayTag BlockTag = bSuccess ? 
        FGameplayTag::RequestGameplayTag("Animation.Block.Success") : 
        FGameplayTag::RequestGameplayTag("Animation.Block.Impact");
    
    PlayAnimationByTag(BlockTag, 1.0f);
}

void UAnimationManagerComponent::HandleDeath()
{
    FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag("Animation.Death");
    
    FAnimationRequest Request;
    Request.Montage = GetMontageByTag(DeathTag);
    Request.AnimationTag = DeathTag;
    Request.bCanBeInterrupted = false;
    Request.Priority = 100;
    
    PlayAnimationRequest(Request);
}

void UAnimationManagerComponent::OnMontageEndedCallback(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == CurrentMontage)
    {
        UnbindMontageEvents(Montage);
        OnAnimationMontageEnded.Broadcast(Montage, bInterrupted);
        UpdateAnimationState(false);
    }
}

void UAnimationManagerComponent::OnMontageBlendOutCallback(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == CurrentMontage)
    {
        OnAnimationMontageBlendOut.Broadcast(Montage);
    }
}

void UAnimationManagerComponent::BindMontageEvents(UAnimMontage* Montage)
{
    if (!AnimInstance || !Montage)
    {
        return;
    }
    
    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &UAnimationManagerComponent::OnMontageEndedCallback);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
    
    FOnMontageBlendingOutStarted BlendOutDelegate;
    BlendOutDelegate.BindUObject(this, &UAnimationManagerComponent::OnMontageBlendOutCallback);
    AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, Montage);
}

void UAnimationManagerComponent::UnbindMontageEvents(UAnimMontage* Montage)
{
    if (!AnimInstance || !Montage)
    {
        return;
    }
    
    // Clear delegates by passing empty delegates
    FOnMontageEnded EmptyEndDelegate;
    AnimInstance->Montage_SetEndDelegate(EmptyEndDelegate, Montage);
    
    FOnMontageBlendingOutStarted EmptyBlendDelegate;
    AnimInstance->Montage_SetBlendingOutDelegate(EmptyBlendDelegate, Montage);
}

bool UAnimationManagerComponent::CanPlayAnimation(const FAnimationRequest& Request) const
{
    if (!bIsPlayingAnimation)
    {
        return true;
    }
    
    if (Request.Priority >= CurrentPriority && bCanInterruptCurrent)
    {
        return true;
    }
    
    return false;
}

void UAnimationManagerComponent::UpdateAnimationState(bool bPlaying, UAnimMontage* Montage, const FGameplayTag& Tag)
{
    bIsPlayingAnimation = bPlaying;
    CurrentMontage = Montage;
    CurrentAnimationTag = Tag;
    
    if (!bPlaying)
    {
        CurrentPriority = 0;
        bCanInterruptCurrent = true;
        CurrentRequest = FAnimationRequest();
    }
}