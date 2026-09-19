#pragma once

#include "CoreMinimal.h"
#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "WyrmBeastAttackAbilities.generated.h"

/**
 * Basic light claw melee attack available in Moonbound beast form.
 * - 0 Focus cost, 0 cooldown.
 * - Raw damage: 25.0
 * - Reach: 160 cm, Radius: 60 cm.
 */
UCLASS()
class WYRMFALL_API UWyrmBeastClawAbility : public UWyrmMeleeAttackAbility
{
    GENERATED_BODY()

public:
    UWyrmBeastClawAbility();
};

/**
 * Heavy pounce leap attack available in Moonbound beast form.
 * - 15 Focus cost, 4.0s cooldown.
 * - Leap distance: up to 500 cm forward with obstacle validation.
 * - Blocked paths clamp to the obstacle hit point rather than clipping through.
 * - Deals 35.0 raw damage to hostile targets within 150 cm radius of landing point.
 */
UCLASS()
class WYRMFALL_API UWyrmBeastPounceAbility : public UWyrmGameplayAbility
{
    GENERATED_BODY()

public:
    UWyrmBeastPounceAbility();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    float PounceDistance = 500.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    float ImpactRadius = 150.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    float PounceDamage = 35.f;

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
