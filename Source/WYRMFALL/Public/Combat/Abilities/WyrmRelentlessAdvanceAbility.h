#pragma once

#include "CoreMinimal.h"
#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "WyrmRelentlessAdvanceAbility.generated.h"

/**
 * Authoritative GAS ability implementing the first Echo: Relentless Advance.
 * - Cost: 30 Focus
 * - Duration: 6.0 seconds
 * - Cooldown: 18.0 seconds
 * Suppresses movement-slow modifiers while timers continue running, resists light/medium stagger.
 * Takes incoming damage normally; no hard-control immunity or heal; no movement speed increase.
 */
UCLASS()
class WYRMFALL_API UWyrmRelentlessAdvanceAbility : public UWyrmGameplayAbility
{
    GENERATED_BODY()

public:
    UWyrmRelentlessAdvanceAbility();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Echo")
    float ActiveDuration = 6.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Echo")
    FGameplayTag RelentlessStateTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Echo")
    FGameplayTag UnlockTag;

    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr,
        const FGameplayTagContainer* TargetTags = nullptr,
        OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;
};
