#pragma once
#include "CoreMinimal.h"
#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "WyrmMeleeAttackAbility.generated.h"

// Authoritative melee attack ability performing hitbox sweeps and applying GAS damage.
UCLASS()
class WYRMFALL_API UWyrmMeleeAttackAbility : public UWyrmGameplayAbility
{
    GENERATED_BODY()
public:
    UWyrmMeleeAttackAbility();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    float WeaponBase = 10.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    float PowerCoefficient = 0.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    float AttackReach = 150.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    float AttackRadius = 50.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    bool bIsSecondary = false;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    UFUNCTION(BlueprintCallable, Category="Combat")
    int32 ExecuteHitboxSweep(TArray<AActor*>& OutHitActors);

    static bool ApplyDamageEffect(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, float InRawDamage);
};

// Light basic melee attack (0 Focus cost, 0 cooldown)
UCLASS()
class WYRMFALL_API UWyrmPrimaryMeleeAbility : public UWyrmMeleeAttackAbility
{
    GENERATED_BODY()
public:
    UWyrmPrimaryMeleeAbility();
};

// Heavy secondary melee attack (20 Focus cost, 5s cooldown, 0.9 power coefficient)
UCLASS()
class WYRMFALL_API UWyrmSecondaryMeleeAbility : public UWyrmMeleeAttackAbility
{
    GENERATED_BODY()
public:
    UWyrmSecondaryMeleeAbility();
};
