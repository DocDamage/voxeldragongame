#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "Combat/WyrmAttributeSet.h"
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

                ApplyDamageEffect(SourceASC, TargetASC, RawDamage);
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

