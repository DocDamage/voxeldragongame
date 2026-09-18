#include "Region/WyrmPracticeTarget.h"
#include "Player/WyrmCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameplayTagsManager.h"

AWyrmPracticeTarget::AWyrmPracticeTarget()
{
    PrimaryActorTick.bCanEverTick = false;

    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->InitCapsuleSize(40.f, 80.f);
    CapsuleComponent->SetCollisionProfileName(TEXT("BlockAll"));
    RootComponent = CapsuleComponent;
}

bool AWyrmPracticeTarget::TriggerPracticeSlow(AWyrmCharacter* TargetCharacter, float Duration, float Magnitude)
{
    if (!TargetCharacter)
    {
        return false;
    }

    bPracticeAttempted = true;
    static const FGameplayTag SlowTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Slow")), false);
    if (SlowTag.IsValid())
    {
        TargetCharacter->ApplyStatusEffect(SlowTag, Duration, Magnitude);
        return true;
    }
    return false;
}

bool AWyrmPracticeTarget::TriggerPracticeStagger(AWyrmCharacter* TargetCharacter, float Duration)
{
    if (!TargetCharacter)
    {
        return false;
    }

    bPracticeAttempted = true;
    static const FGameplayTag StaggerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Stagger")), false);
    if (StaggerTag.IsValid())
    {
        TargetCharacter->ApplyStatusEffect(StaggerTag, Duration, 1.0f);
        return true;
    }
    return false;
}
