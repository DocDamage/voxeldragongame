#pragma once
#include "CoreMinimal.h"
#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "WyrmEvadeAbility.generated.h"

// Tactical dodge/evade ability providing mobility and cooldown gating.
UCLASS()
class WYRMFALL_API UWyrmEvadeAbility : public UWyrmGameplayAbility
{
    GENERATED_BODY()
public:
    UWyrmEvadeAbility();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    float EvadeImpulse = 1200.f;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
