#include "Combat/Abilities/WyrmHuntersVeilAbility.h"
#include "Player/WyrmCharacter.h"
#include "AbilitySystemComponent.h"

UWyrmHuntersVeilAbility::UWyrmHuntersVeilAbility()
{
    FocusCost = 25.f;
    CooldownDuration = 16.f;
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Echo.HuntersVeil")), false);
    CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.HuntersVeil")), false);
    UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.HuntersVeil")), false);
}

bool UWyrmHuntersVeilAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
        return Character->CanActivateHuntersVeil(Reason);
    }
    return false;
}

void UWyrmHuntersVeilAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData*)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    AWyrmCharacter* Character = Cast<AWyrmCharacter>(GetAvatarActorFromActorInfo());
    if (!Character || !Character->CommitHuntersVeil())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
