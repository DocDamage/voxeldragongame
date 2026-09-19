#include "Combat/Abilities/WyrmBeastAttackAbilities.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"

UWyrmBeastClawAbility::UWyrmBeastClawAbility()
{
    FocusCost = 0.f;
    CooldownDuration = 0.f;
    WeaponBase = 25.f;
    PowerCoefficient = 0.0f; // Flat 25.0 beast claw damage
    AttackReach = 160.f;
    AttackRadius = 60.f;

    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Combat.BeastClaw")), false);
}

UWyrmBeastPounceAbility::UWyrmBeastPounceAbility()
{
    FocusCost = 15.f;
    CooldownDuration = 4.0f;
    PounceDistance = 500.f;
    ImpactRadius = 150.f;
    PounceDamage = 35.f;

    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Combat.BeastPounce")), false);
    CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Combat.BeastPounce")), false);
}

bool UWyrmBeastPounceAbility::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    AWyrmCharacter* Character = Cast<AWyrmCharacter>(Avatar);
    if (Character && Character->IsMoonboundReturnPending())
    {
        // Suppress beast attacks while return is blocked to prevent infinite combat extension
        return false;
    }

    return true;
}

void UWyrmBeastPounceAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    ACharacter* Character = Cast<ACharacter>(Avatar);
    if (!Character)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    UWorld* World = Character->GetWorld();
    if (!World)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    const FVector Start = Character->GetActorLocation();
    const FVector Forward = Character->GetActorForwardVector();
    FVector TargetLanding = Start + Forward * PounceDistance;

    // Validate path and check for obstacles along the leap trajectory
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Character);

    bool bHit = World->SweepSingleByChannel(
        HitResult,
        Start,
        TargetLanding,
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeSphere(40.f),
        Params);

    if (bHit && HitResult.bBlockingHit)
    {
        // Clamp landing location safely before the obstacle
        TargetLanding = HitResult.Location - Forward * 30.f;
    }

    // Move character to landing position
    Character->SetActorLocation(TargetLanding, true);

    // Apply impact area damage to nearby enemies
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(Character);
    TArray<AActor*> OverlappingActors;

    UKismetSystemLibrary::SphereOverlapActors(
        World,
        TargetLanding,
        ImpactRadius,
        ObjectTypes,
        nullptr,
        IgnoredActors,
        OverlappingActors);

    UAbilitySystemComponent* SourceASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
    if (SourceASC)
    {
        for (AActor* TargetActor : OverlappingActors)
        {
            if (TargetActor && TargetActor != Character)
            {
                IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TargetActor);
                UAbilitySystemComponent* TargetASC = TargetASI ? TargetASI->GetAbilitySystemComponent() : nullptr;
                if (TargetASC)
                {
                    UWyrmMeleeAttackAbility::ApplyDamageEffect(SourceASC, TargetASC, PounceDamage);
                }
            }
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
