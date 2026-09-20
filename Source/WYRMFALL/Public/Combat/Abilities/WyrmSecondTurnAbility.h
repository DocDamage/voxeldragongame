#pragma once

#include "CoreMinimal.h"
#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "WyrmSecondTurnAbility.generated.h"

/** Primes the next eligible basic strike for one delayed, nonrecursive repeat. */
UCLASS()
class WYRMFALL_API UWyrmSecondTurnAbility : public UWyrmGameplayAbility
{
    GENERATED_BODY()

public:
    UWyrmSecondTurnAbility();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Echo")
    FGameplayTag UnlockTag;

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr,
        const FGameplayTagContainer* TargetTags = nullptr,
        OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;
};
