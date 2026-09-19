#pragma once

#include "CoreMinimal.h"
#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "WyrmMoonboundFormAbility.generated.h"

/**
 * Authoritative GAS ability implementing the second Echo: Moonbound Form.
 * - Cost: 40 Focus
 * - Duration: 12.0 seconds
 * - Cooldown: 35.0 seconds
 * Transforms the player character into a genuine beast form with real wolf/beast mesh,
 * beast combat kit (claw basic attack, pounce heavy attack), preserving identity,
 * health, inventory, and permanent Mutable appearance recipe.
 */
UCLASS()
class WYRMFALL_API UWyrmMoonboundFormAbility : public UWyrmGameplayAbility
{
    GENERATED_BODY()

public:
    UWyrmMoonboundFormAbility();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Echo")
    float ActiveDuration = 12.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Echo")
    FGameplayTag MoonboundStateTag;

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
