#pragma once

#include "CoreMinimal.h"
#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "WyrmMirrorStepAbility.generated.h"

/** GAS-authoritative Mirror Step: 20 Focus, validated displacement, 10s cooldown. */
UCLASS()
class WYRMFALL_API UWyrmMirrorStepAbility : public UWyrmGameplayAbility
{
    GENERATED_BODY()

public:
    UWyrmMirrorStepAbility();

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
