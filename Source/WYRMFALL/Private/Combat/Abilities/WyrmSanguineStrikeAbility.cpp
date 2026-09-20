#include "Combat/Abilities/WyrmSanguineStrikeAbility.h"
#include "Player/WyrmCharacter.h"
#include "AbilitySystemComponent.h"

UWyrmSanguineStrikeAbility::UWyrmSanguineStrikeAbility()
{
    FocusCost = 25.f;
    CooldownDuration = 12.f;
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Echo.SanguineStrike")), false);
    CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.SanguineStrike")), false);
    UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.SanguineStrike")), false);
}

bool UWyrmSanguineStrikeAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
        return Character->CanActivateSanguineStrike(Reason);
    }
    return false;
}

void UWyrmSanguineStrikeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData*)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    AWyrmCharacter* Character = Cast<AWyrmCharacter>(GetAvatarActorFromActorInfo());
    if (!Character || !Character->CommitSanguineStrike())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
