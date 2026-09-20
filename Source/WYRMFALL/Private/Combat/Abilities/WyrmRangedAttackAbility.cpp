#include "Combat/Abilities/WyrmRangedAttackAbility.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"

UWyrmRangedAttackAbility::UWyrmRangedAttackAbility()
{
    ProjectileClass = AWyrmProjectile::StaticClass();
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Ranged.Basic")), false);
}

AWyrmProjectile* UWyrmRangedAttackAbility::SpawnProjectile(AActor* Avatar, UAbilitySystemComponent* SourceASC, float InRawDamage)
{
    if (!Avatar || !Avatar->GetWorld() || !ProjectileClass)
    {
        return nullptr;
    }

    UWorld* World = Avatar->GetWorld();
    const FVector Forward = Avatar->GetActorForwardVector();
    const FVector SpawnLoc = Avatar->GetActorLocation() + Forward * SpawnForwardOffset + FVector(0.f, 0.f, 30.f);
    const FRotator SpawnRot = Avatar->GetActorRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Avatar;
    SpawnParams.Instigator = Cast<APawn>(Avatar);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AWyrmProjectile* Proj = World->SpawnActor<AWyrmProjectile>(ProjectileClass, SpawnLoc, SpawnRot, SpawnParams);
    if (Proj)
    {
        Proj->InitializeProjectile(Avatar, SourceASC, InRawDamage, Forward, !bIsSecondary);
    }
    return Proj;
}

void UWyrmRangedAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAbilitySystemComponent* SourceASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
    AActor* Avatar = GetAvatarActorFromActorInfo();

    if (SourceASC && Avatar)
    {
        float InstigatorPower = 20.f;
        const UWyrmAttributeSet* AttrSet = Cast<UWyrmAttributeSet>(
            SourceASC->GetAttributeSet(UWyrmAttributeSet::StaticClass()));
        if (AttrSet)
        {
            InstigatorPower = AttrSet->GetPower();
        }

        const float RawDamage = UWyrmAttributeSet::CalculateRawDamage(WeaponBase, InstigatorPower, PowerCoefficient);
        SpawnProjectile(Avatar, SourceASC, RawDamage);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UWyrmPrimaryRangedAbility::UWyrmPrimaryRangedAbility()
{
    WeaponBase = 10.f;
    PowerCoefficient = 0.50f;
    FocusCost = 0.f;
    CooldownDuration = 0.f;
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Ranged.Basic")), false);
    bIsSecondary = false;
}

UWyrmSecondaryRangedAbility::UWyrmSecondaryRangedAbility()
{
    WeaponBase = 10.f;
    PowerCoefficient = 0.90f;
    FocusCost = 20.f; // 20 Focus cost (COM-04)
    CooldownDuration = 5.0f; // 5-second cooldown
    CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Ranged.Secondary")), false);
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Ranged.Secondary")), false);
    bIsSecondary = true;
}
