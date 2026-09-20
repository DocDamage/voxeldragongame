#pragma once

#include "CoreMinimal.h"
#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "WyrmSanguineStrikeAbility.generated.h"

/** Primes one eligible basic weapon hit: 25 Focus, 4s window, 12s cooldown. */
UCLASS()
class WYRMFALL_API UWyrmSanguineStrikeAbility : public UWyrmGameplayAbility
{
    GENERATED_BODY()

public:
    UWyrmSanguineStrikeAbility();

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
