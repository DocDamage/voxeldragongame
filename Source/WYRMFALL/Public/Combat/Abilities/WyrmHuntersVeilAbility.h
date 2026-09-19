#pragma once

#include "CoreMinimal.h"
#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "WyrmHuntersVeilAbility.generated.h"

/** GAS-authoritative concealment: 25 Focus, up to 5s active, 16s cooldown. */
UCLASS()
class WYRMFALL_API UWyrmHuntersVeilAbility : public UWyrmGameplayAbility
{
    GENERATED_BODY()

public:
    UWyrmHuntersVeilAbility();

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
