#include "Combat/Abilities/WyrmSecondTurnAbility.h"
#include "Player/WyrmCharacter.h"
#include "AbilitySystemComponent.h"

UWyrmSecondTurnAbility::UWyrmSecondTurnAbility()
{
    FocusCost = 25.f;
    CooldownDuration = 14.f;
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Echo.SecondTurn")), false);
    CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.SecondTurn")), false);
    UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.SecondTurn")), false);
}

bool UWyrmSecondTurnAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
        return Character->CanActivateSecondTurn(Reason);
    }
    return false;
}

void UWyrmSecondTurnAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData*)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    AWyrmCharacter* Character = Cast<AWyrmCharacter>(GetAvatarActorFromActorInfo());
    if (!Character || !Character->CommitSecondTurn())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
