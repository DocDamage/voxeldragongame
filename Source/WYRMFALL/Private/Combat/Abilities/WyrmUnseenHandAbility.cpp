#include "Combat/Abilities/WyrmUnseenHandAbility.h"
#include "Player/WyrmCharacter.h"
#include "GameplayTagsManager.h"

UWyrmUnseenHandAbility::UWyrmUnseenHandAbility()
{
    FocusCost = 25.f;
    CooldownDuration = 8.f;
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Echo.UnseenHand")), false);
    CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.UnseenHand")), false);
    UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.UnseenHand")), false);
}

bool UWyrmUnseenHandAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) ||
        !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid()) return false;
    if (UnlockTag.IsValid() && !ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(UnlockTag)) return false;
    if (const AWyrmCharacter* Character = Cast<AWyrmCharacter>(GetAvatarActorFromActorInfo()))
    {
        FString Reason;
        return Character->CanActivateUnseenHand(Character->GetPendingUnseenHandTarget(), Reason);
    }
    return false;
}

void UWyrmUnseenHandAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData*)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    AWyrmCharacter* Character = Cast<AWyrmCharacter>(GetAvatarActorFromActorInfo());
    if (!Character || !Character->CommitUnseenHand())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
