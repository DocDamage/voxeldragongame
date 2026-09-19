#include "Combat/Abilities/WyrmMirrorStepAbility.h"
#include "Player/WyrmCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

UWyrmMirrorStepAbility::UWyrmMirrorStepAbility()
{
    FocusCost = 20.f;
    CooldownDuration = 10.f;
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Echo.MirrorStep")), false);
    CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.MirrorStep")), false);
    UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.MirrorStep")), false);
}

bool UWyrmMirrorStepAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) ||
        !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
    {
        return false;
    }
    if (UnlockTag.IsValid() && !ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(UnlockTag))
    {
        return false;
    }
    if (const AWyrmCharacter* Character = Cast<AWyrmCharacter>(GetAvatarActorFromActorInfo()))
    {
        FString Reason;
        return Character->CanActivateMirrorStep(Character->GetPendingMirrorStepDestination(), Reason);
    }
    return false;
}

void UWyrmMirrorStepAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData*)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    AWyrmCharacter* Character = Cast<AWyrmCharacter>(GetAvatarActorFromActorInfo());
    if (!Character || !Character->CommitMirrorStep())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
