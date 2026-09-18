#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagsManager.h"

UWyrmGameplayAbility::UWyrmGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

bool UWyrmGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        static const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Dead")), false);
        static const FGameplayTag StunTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Stun")), false);

        if ((DeadTag.IsValid() && ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(DeadTag)) ||
            (StunTag.IsValid() && ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(StunTag)))
        {
            return false;
        }
    }

    return true;
}

bool UWyrmGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (FocusCost <= 0.f)
    {
        return true;
    }

    if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        const UWyrmAttributeSet* Set = Cast<UWyrmAttributeSet>(
            ActorInfo->AbilitySystemComponent->GetAttributeSet(UWyrmAttributeSet::StaticClass()));
        if (Set && Set->GetFocus() >= FocusCost)
        {
            return true;
        }
    }

    return false;
}

void UWyrmGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
    if (FocusCost <= 0.f || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
    {
        return;
    }

    UWyrmAttributeSet* Set = const_cast<UWyrmAttributeSet*>(Cast<UWyrmAttributeSet>(
        ActorInfo->AbilitySystemComponent->GetAttributeSet(UWyrmAttributeSet::StaticClass())));
    if (Set)
    {
        const float NewFocus = FMath::Max(0.f, Set->GetFocus() - FocusCost);
        Set->SetCurrentFocus(NewFocus);
    }
}

const FGameplayTagContainer* UWyrmGameplayAbility::GetCooldownTags() const
{
    TempCooldownTags.Reset();
    if (CooldownTag.IsValid())
    {
        TempCooldownTags.AddTag(CooldownTag);
    }
    return &TempCooldownTags;
}

bool UWyrmGameplayAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
    const FGameplayTagContainer* CDTags = GetCooldownTags();
    if (CDTags && CDTags->Num() > 0 && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        if (ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(*CDTags))
        {
            return false;
        }
    }
    return true;
}

#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

void UWyrmGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
    if (CooldownDuration <= 0.f || !CooldownTag.IsValid() || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
    {
        return;
    }

    if (!ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(CooldownTag))
    {
        ActorInfo->AbilitySystemComponent->AddLooseGameplayTag(CooldownTag);
    }

    UGameplayEffect* CooldownGE = NewObject<UGameplayEffect>();
    CooldownGE->DurationPolicy = EGameplayEffectDurationType::HasDuration;
    CooldownGE->DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(CooldownDuration));

    UTargetTagsGameplayEffectComponent& TargetTagsComponent = CooldownGE->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
    FInheritedTagContainer TagContainerMods;
    TagContainerMods.Added.AddTag(CooldownTag);
    TagContainerMods.CombinedTags.AddTag(CooldownTag);
    TargetTagsComponent.SetAndApplyTargetTagChanges(TagContainerMods);

    ActorInfo->AbilitySystemComponent->ApplyGameplayEffectToSelf(CooldownGE, 1.f, ActorInfo->AbilitySystemComponent->MakeEffectContext());
}
