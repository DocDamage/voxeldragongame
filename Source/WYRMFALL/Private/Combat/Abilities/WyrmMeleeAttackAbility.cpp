#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "Combat/WyrmAttributeSet.h"
#include "Player/WyrmCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

UWyrmMeleeAttackAbility::UWyrmMeleeAttackAbility()
{
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Melee.Basic")), false);
}

bool UWyrmMeleeAttackAbility::ApplyDamageEffect(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, float InRawDamage)
{
    if (!SourceASC || !TargetASC || InRawDamage <= 0.f)
    {
        return false;
    }

    UGameplayEffect* DamageGE = NewObject<UGameplayEffect>();
    DamageGE->DurationPolicy = EGameplayEffectDurationType::Instant;

    FGameplayModifierInfo ModInfo;
    ModInfo.Attribute = UWyrmAttributeSet::GetIncomingDamageAttribute();
    ModInfo.ModifierOp = EGameplayModOp::Additive;
    ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(InRawDamage));
    DamageGE->Modifiers.Add(ModInfo);

    FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
    Context.AddInstigator(SourceASC->GetAvatarActor(), SourceASC->GetAvatarActor());

    SourceASC->ApplyGameplayEffectToTarget(DamageGE, TargetASC, 1.f, Context);
    return true;
}

bool UWyrmMeleeAttackAbility::ApplyDamageEffectWithArmorPenetration(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, float InRawDamage, float ArmorIgnoreFraction)
{
    if (!SourceASC || !TargetASC || InRawDamage <= 0.f)
    {
        return false;
    }

    const UWyrmAttributeSet* SourceAttributes = Cast<UWyrmAttributeSet>(
        SourceASC->GetAttributeSet(UWyrmAttributeSet::StaticClass()));
    const UWyrmAttributeSet* TargetAttributes = Cast<UWyrmAttributeSet>(
        TargetASC->GetAttributeSet(UWyrmAttributeSet::StaticClass()));
    if (!TargetAttributes)
    {
        return ApplyDamageEffect(SourceASC, TargetASC, InRawDamage);
    }

    const float SafeIgnore = FMath::Clamp(ArmorIgnoreFraction, 0.f, 1.f);
    const float AttackerLevel = SourceAttributes ? SourceAttributes->GetCharacterLevel() : 1.f;
    const float FullArmor = TargetAttributes->GetArmor();
    const float DesiredDamage = UWyrmAttributeSet::CalculateMitigatedDamage(
        InRawDamage, FullArmor * (1.f - SafeIgnore), AttackerLevel);
    const float FullArmorMultiplier = 1.f - UWyrmAttributeSet::CalculatePhysicalMitigation(FullArmor, AttackerLevel);
    const float AdjustedRawDamage = FullArmorMultiplier > KINDA_SMALL_NUMBER
        ? DesiredDamage / FullArmorMultiplier
        : InRawDamage;
    return ApplyDamageEffect(SourceASC, TargetASC, AdjustedRawDamage);
}

bool UWyrmMeleeAttackAbility::ApplyPeriodicDamageEffect(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, float TotalRawDamage, float Duration, float Period)
{
    if (!SourceASC || !TargetASC || TotalRawDamage <= 0.f || Duration <= 0.f || Period <= 0.f)
    {
        return false;
    }

    const int32 TickCount = FMath::Max(1, FMath::RoundToInt(Duration / Period));
    UGameplayEffect* DamageGE = NewObject<UGameplayEffect>();
    DamageGE->DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DamageGE->DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Duration));
    DamageGE->Period = FScalableFloat(Period);
    DamageGE->bExecutePeriodicEffectOnApplication = false;

    FGameplayModifierInfo ModInfo;
    ModInfo.Attribute = UWyrmAttributeSet::GetIncomingDamageAttribute();
    ModInfo.ModifierOp = EGameplayModOp::Additive;
    ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(TotalRawDamage / static_cast<float>(TickCount)));
    DamageGE->Modifiers.Add(ModInfo);

    FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
    Context.AddInstigator(SourceASC->GetAvatarActor(), SourceASC->GetAvatarActor());
    SourceASC->ApplyGameplayEffectToTarget(DamageGE, TargetASC, 1.f, Context);
    return true;
}

bool UWyrmMeleeAttackAbility::ApplyEligibleWeaponDamageEffect(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, float InRawDamage, bool bIsMeleeHit)
{
    if (!SourceASC || !TargetASC || InRawDamage <= 0.f)
    {
        return false;
    }

    AWyrmCharacter* SourceCharacter = Cast<AWyrmCharacter>(SourceASC->GetAvatarActor());
    const UWyrmAttributeSet* TargetAttributes = Cast<UWyrmAttributeSet>(
        TargetASC->GetAttributeSet(UWyrmAttributeSet::StaticClass()));
    const float SanguineBonusDamage = SourceCharacter ? SourceCharacter->GetSanguineStrikeBonusDamage() : 0.f;
    const float DeathmarkBonusDamage = SourceCharacter ? SourceCharacter->GetDeathmarkBonusDamage(TargetASC) : 0.f;
    const float CarversPrecisionWoundDamage = SourceCharacter && bIsMeleeHit
        ? SourceCharacter->GetCarversPrecisionWoundDamage()
        : 0.f;
    const bool bUseCarversPrecision = CarversPrecisionWoundDamage > 0.f;
    const float HealthBefore = TargetAttributes ? TargetAttributes->GetCurrentHealth() : 0.f;
    const float ShieldBefore = TargetAttributes ? TargetAttributes->GetCurrentShield() : 0.f;
    const float DirectRawDamage = InRawDamage + SanguineBonusDamage + DeathmarkBonusDamage;
    const bool bAppliedDirectDamage = bUseCarversPrecision
        ? ApplyDamageEffectWithArmorPenetration(SourceASC, TargetASC, DirectRawDamage, 0.30f)
        : ApplyDamageEffect(SourceASC, TargetASC, DirectRawDamage);
    if (!bAppliedDirectDamage)
    {
        return false;
    }

    if (SourceCharacter && TargetAttributes)
    {
        const float ActualHealthDamage = FMath::Max(0.f, HealthBefore - TargetAttributes->GetCurrentHealth());
        const float ActualShieldDamage = FMath::Max(0.f, ShieldBefore - TargetAttributes->GetCurrentShield());
        if (ActualHealthDamage + ActualShieldDamage > 0.f)
        {
            // Snapshot only the basic strike's pre-mitigation base. The delayed
            // repeat excludes Sanguine and every other proc contribution.
            SourceCharacter->QueueSecondTurnRepeat(TargetASC, InRawDamage);
            if (SanguineBonusDamage > 0.f)
            {
                // The strike is consumed by a shielded hit, but only damage
                // that reached Health can contribute to its healing.
                SourceCharacter->ConsumeSanguineStrike(ActualHealthDamage);
            }
            if (DeathmarkBonusDamage > 0.f)
            {
                SourceCharacter->ConsumeDeathmark(TargetASC);
            }
            if (bUseCarversPrecision && SourceCharacter->ConsumeCarversPrecision())
            {
                ApplyPeriodicDamageEffect(SourceASC, TargetASC, CarversPrecisionWoundDamage, 3.f, 1.f);
            }
        }
    }
    return true;
}

int32 UWyrmMeleeAttackAbility::ExecuteHitboxSweep(TArray<AActor*>& OutHitActors)
{
    OutHitActors.Reset();
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar || !Avatar->GetWorld())
    {
        return 0;
    }

    UWorld* World = Avatar->GetWorld();
    const FVector StartLoc = Avatar->GetActorLocation() + FVector(0.f, 0.f, 20.f);
    const FVector Forward = Avatar->GetActorForwardVector();
    const FVector EndLoc = StartLoc + Forward * AttackReach;

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Avatar);

    World->SweepMultiByChannel(Hits, StartLoc, EndLoc, FQuat::Identity, ECC_Pawn, Sphere, Params);

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor && HitActor != Avatar && !OutHitActors.Contains(HitActor))
        {
            OutHitActors.Add(HitActor);
        }
    }

    return OutHitActors.Num();
}

void UWyrmMeleeAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

        TArray<AActor*> HitActors;
        ExecuteHitboxSweep(HitActors);

        static const FGameplayTag PlayerTeamTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Combat.Team.Player")), false);

        for (AActor* TargetActor : HitActors)
        {
            if (!TargetActor)
            {
                continue;
            }

            IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor);
            UAbilitySystemComponent* TargetASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;

            if (TargetASC)
            {
                // Friendly fire prevention (COM-02): do not damage members of the same team
                if (PlayerTeamTag.IsValid() &&
                    SourceASC->HasMatchingGameplayTag(PlayerTeamTag) &&
                    TargetASC->HasMatchingGameplayTag(PlayerTeamTag))
                {
                    continue;
                }

                if (bIsSecondary)
                {
                    ApplyDamageEffect(SourceASC, TargetASC, RawDamage);
                }
                else
                {
                    ApplyEligibleWeaponDamageEffect(SourceASC, TargetASC, RawDamage, true);
                }
            }
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UWyrmPrimaryMeleeAbility::UWyrmPrimaryMeleeAbility()
{
    WeaponBase = 10.f;
    PowerCoefficient = 0.5f;
    FocusCost = 0.f;
    CooldownDuration = 0.f;
    bIsSecondary = false;
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Melee.Basic")), false);
}

UWyrmSecondaryMeleeAbility::UWyrmSecondaryMeleeAbility()
{
    WeaponBase = 10.f;
    PowerCoefficient = 0.9f;
    FocusCost = 20.f; // 20 Focus cost (COM-04)
    CooldownDuration = 5.0f; // 5-second cooldown
    CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Melee.Secondary")), false);
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Melee.Secondary")), false);
    bIsSecondary = true;
}
