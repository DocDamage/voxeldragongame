#pragma once
#include "CoreMinimal.h"
#include "Combat/Abilities/WyrmGameplayAbility.h"
#include "Combat/Projectiles/WyrmProjectile.h"
#include "WyrmRangedAttackAbility.generated.h"

// Authoritative ranged ability spawning projectiles and applying GAS damage.
UCLASS()
class WYRMFALL_API UWyrmRangedAttackAbility : public UWyrmGameplayAbility
{
    GENERATED_BODY()
public:
    UWyrmRangedAttackAbility();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    float WeaponBase = 10.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    float PowerCoefficient = 0.5f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    TSubclassOf<AWyrmProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Combat")
    float SpawnForwardOffset = 80.f;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    AWyrmProjectile* SpawnProjectile(AActor* Avatar, UAbilitySystemComponent* SourceASC, float InRawDamage);
};

// Basic ranged attack (0 Focus, 0 cooldown, 0.50 power coefficient)
UCLASS()
class WYRMFALL_API UWyrmPrimaryRangedAbility : public UWyrmRangedAttackAbility
{
    GENERATED_BODY()
public:
    UWyrmPrimaryRangedAbility();
};

// Heavy secondary ranged attack (20 Focus, 5.0s cooldown, 0.90 power coefficient)
UCLASS()
class WYRMFALL_API UWyrmSecondaryRangedAbility : public UWyrmRangedAttackAbility
{
    GENERATED_BODY()
public:
    UWyrmSecondaryRangedAbility();
};
