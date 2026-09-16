#include "Combat/Abilities/WyrmEvadeAbility.h"
#include "GameFramework/Character.h"

UWyrmEvadeAbility::UWyrmEvadeAbility()
{
    FocusCost = 0.f;
    CooldownDuration = 1.25f; // 1.25s recovery per combat spec
    CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Cooldown.Utility.Evade")), false);
    AbilityTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Ability.Utility.Evade")), false);
}

void UWyrmEvadeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    ACharacter* Char = Cast<ACharacter>(Avatar);
    if (Char)
    {
        FVector LaunchDir = Char->GetLastMovementInputVector();
        if (LaunchDir.IsNearlyZero())
        {
            LaunchDir = -Char->GetActorForwardVector(); // Disengage backward
        }
        LaunchDir.Z = 0.f;
        LaunchDir.Normalize();

        Char->LaunchCharacter(LaunchDir * EvadeImpulse + FVector(0.f, 0.f, 150.f), true, true);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
