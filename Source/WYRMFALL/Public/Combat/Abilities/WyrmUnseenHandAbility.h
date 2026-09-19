#pragma once

#include "CoreMinimal.h"
#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "WyrmUnseenHandAbility.generated.h"

/** GAS-authoritative shove: 25 Focus, eligible light targets only, 8s cooldown. */
UCLASS()
class WYRMFALL_API UWyrmUnseenHandAbility : public UWyrmGameplayAbility
{
    GENERATED_BODY()

public:
    UWyrmUnseenHandAbility();

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
