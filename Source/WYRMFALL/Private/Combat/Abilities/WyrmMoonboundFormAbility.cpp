#include "Combat/Abilities/WyrmMoonboundFormAbility.h"
#include "Player/WyrmCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

UWyrmMoonboundFormAbility::UWyrmMoonboundFormAbility()
{
    FocusCost = 40.0f;
    CooldownDuration = 35.0f;
    ActiveDuration = 12.0f;

    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Echo.MoonboundForm")), false);
    CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Echo.MoonboundForm")), false);
    MoonboundStateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.MoonboundForm")), false);
    UnlockTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Echo.MoonboundForm")), false);
}

bool UWyrmMoonboundFormAbility::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
    {
        return false;
    }

    UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

    // Check permanent unlock prerequisite
    if (UnlockTag.IsValid() && !ASC->HasMatchingGameplayTag(UnlockTag))
    {
        return false;
    }

    // Check control/form transition state
    static const FGameplayTag TransitionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Control.Transition")), false);
    if (TransitionTag.IsValid() && ASC->HasMatchingGameplayTag(TransitionTag))
    {
        return false;
    }

    // Check hard stun
    static const FGameplayTag StunTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Stun")), false);
    if (StunTag.IsValid() && ASC->HasMatchingGameplayTag(StunTag))
    {
        return false;
    }

    // Check character specific preconditions (grounded, not in vehicle/dragon, etc.)
    AActor* Avatar = GetAvatarActorFromActorInfo();
    AWyrmCharacter* Character = Cast<AWyrmCharacter>(Avatar);
    if (Character)
    {
        FString Reason;
        if (!Character->CanActivateMoonboundForm(Reason))
        {
            return false;
        }
    }

    return true;
}

void UWyrmMoonboundFormAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
    if (ASC && MoonboundStateTag.IsValid())
    {
        ASC->AddLooseGameplayTag(MoonboundStateTag);
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    AWyrmCharacter* Character = Cast<AWyrmCharacter>(Avatar);
    if (Character)
    {
        Character->ActivateMoonboundForm(ActiveDuration);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
