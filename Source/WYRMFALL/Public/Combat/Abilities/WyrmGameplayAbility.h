#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Combat/WyrmCombatTypes.h"
#include "WyrmGameplayAbility.generated.h"

// Base gameplay ability for WYRMFALL providing Focus cost, cooldown commit, and status gating.
UCLASS()
class WYRMFALL_API UWyrmGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()
public:
    UWyrmGameplayAbility();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost")
    float FocusCost = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cooldown")
    float CooldownDuration = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cooldown")
    FGameplayTag CooldownTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability")
    FGameplayTag AbilityTag;

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
    virtual const FGameplayTagContainer* GetCooldownTags() const override;
    virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
    mutable FGameplayTagContainer TempCooldownTags;
};
