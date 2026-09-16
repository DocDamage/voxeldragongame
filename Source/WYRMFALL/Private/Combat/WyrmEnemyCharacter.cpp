#include "Combat/WyrmEnemyCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagsManager.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

AWyrmEnemyCharacter::AWyrmEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
    Attributes = CreateDefaultSubobject<UWyrmAttributeSet>(TEXT("Attributes"));

    GetCapsuleComponent()->InitCapsuleSize(45.f, 75.f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    BaseWalkSpeed = 550.f;
    GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void AWyrmEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (AbilitySystem)
    {
        AbilitySystem->InitAbilityActorInfo(this, this);
        if (Attributes && !AbilitySystem->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
        {
            AbilitySystem->AddAttributeSetSubobject(Attributes.Get());
        }

        static const FGameplayTag HostileTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Combat.Team.Hostile")), false);
        if (HostileTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(HostileTag))
        {
            AbilitySystem->AddLooseGameplayTag(HostileTag);
        }

        if (Attributes)
        {
            FOnGameplayAttributeValueChange& HealthChanged =
                AbilitySystem->GetGameplayAttributeValueChangeDelegate(Attributes->GetHealthAttribute());
            HealthChanged.RemoveAll(this);
            HealthChanged.AddUObject(this, &AWyrmEnemyCharacter::HandleHealthChanged);
        }
    }

    ConfigureForRole(EnemyRole);
}

void AWyrmEnemyCharacter::ConfigureForRole(EWyrmEnemyRole InRole)
{
    EnemyRole = InRole;

    if (!Attributes)
    {
        return;
    }

    if (EnemyRole == EWyrmEnemyRole::MeleeChaser)
    {
        Attributes->InitMaxHealth(60.f);
        Attributes->InitHealth(60.f);
        Attributes->InitArmor(10.f);
        Attributes->InitPower(15.f);
        BaseWalkSpeed = 550.f;
    }
    else // RangedSkirmisher
    {
        Attributes->InitMaxHealth(50.f);
        Attributes->InitHealth(50.f);
        Attributes->InitArmor(5.f);
        Attributes->InitPower(12.f);
        BaseWalkSpeed = 400.f;
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
    }
}

bool AWyrmEnemyCharacter::AttackTarget(AActor* TargetActor)
{
    if (!TargetActor || IsDefeated() || StunRemainingTimer > 0.f || !AbilitySystem || !Attributes)
    {
        return false;
    }

    IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TargetActor);
    UAbilitySystemComponent* TargetASC = TargetASI ? TargetASI->GetAbilitySystemComponent() : nullptr;

    if (!TargetASC)
    {
        return false;
    }

    // Calculate raw damage: Enemy Power with 0.5 coefficient
    const float RawDamage = UWyrmAttributeSet::CalculateRawDamage(5.f, Attributes->GetPower(), 0.5f);
    return UWyrmMeleeAttackAbility::ApplyDamageEffect(AbilitySystem, TargetASC, RawDamage);
}

void AWyrmEnemyCharacter::ApplyStatusEffect(FGameplayTag StatusTag, float DurationSeconds, float Magnitude)
{
    if (IsDefeated())
    {
        return;
    }

    static const FGameplayTag SlowTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Slow")), false);
    static const FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Root")), false);
    static const FGameplayTag StunTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Stun")), false);

    float EffectiveDuration = DurationSeconds;
    float EffectiveMagnitude = Magnitude;

    if (bIsBoss)
    {
        // Boss resistance profile: reduces slow duration/magnitude, resists hard stun/root
        EffectiveDuration *= 0.5f;
        EffectiveMagnitude *= 0.5f;
    }

    if (StatusTag == SlowTag)
    {
        // Combining slow by highest magnitude (COM-05)
        ActiveSlowMagnitude = FMath::Max(ActiveSlowMagnitude, EffectiveMagnitude);
        SlowRemainingTimer = FMath::Max(SlowRemainingTimer, EffectiveDuration);
        if (AbilitySystem && SlowTag.IsValid())
        {
            AbilitySystem->AddLooseGameplayTag(SlowTag);
        }
    }
    else if (StatusTag == RootTag)
    {
        if (!bIsBoss)
        {
            RootRemainingTimer = FMath::Max(RootRemainingTimer, EffectiveDuration);
            if (AbilitySystem && RootTag.IsValid())
            {
                AbilitySystem->AddLooseGameplayTag(RootTag);
            }
        }
    }
    else if (StatusTag == StunTag)
    {
        if (!bIsBoss)
        {
            StunRemainingTimer = FMath::Max(StunRemainingTimer, EffectiveDuration);
            if (AbilitySystem && StunTag.IsValid())
            {
                AbilitySystem->AddLooseGameplayTag(StunTag);
            }
        }
    }

    UpdateMovementForStatus();
}

void AWyrmEnemyCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    bool bNeedsMovementUpdate = false;

    if (SlowRemainingTimer > 0.f)
    {
        SlowRemainingTimer -= DeltaSeconds;
        if (SlowRemainingTimer <= 0.f)
        {
            ActiveSlowMagnitude = 0.f;
            bNeedsMovementUpdate = true;
            if (AbilitySystem)
            {
                static const FGameplayTag SlowTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Slow")), false);
                if (SlowTag.IsValid()) { AbilitySystem->RemoveLooseGameplayTag(SlowTag); }
            }
        }
    }

    if (RootRemainingTimer > 0.f)
    {
        RootRemainingTimer -= DeltaSeconds;
        if (RootRemainingTimer <= 0.f)
        {
            bNeedsMovementUpdate = true;
            if (AbilitySystem)
            {
                static const FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Root")), false);
                if (RootTag.IsValid()) { AbilitySystem->RemoveLooseGameplayTag(RootTag); }
            }
        }
    }

    if (StunRemainingTimer > 0.f)
    {
        StunRemainingTimer -= DeltaSeconds;
        if (StunRemainingTimer <= 0.f)
        {
            bNeedsMovementUpdate = true;
            if (AbilitySystem)
            {
                static const FGameplayTag StunTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Stun")), false);
                if (StunTag.IsValid()) { AbilitySystem->RemoveLooseGameplayTag(StunTag); }
            }
        }
    }

    if (bNeedsMovementUpdate)
    {
        UpdateMovementForStatus();
    }
}

void AWyrmEnemyCharacter::UpdateMovementForStatus()
{
    if (!GetCharacterMovement())
    {
        return;
    }

    if (IsDefeated() || StunRemainingTimer > 0.f || RootRemainingTimer > 0.f)
    {
        GetCharacterMovement()->MaxWalkSpeed = 0.f;
    }
    else if (SlowRemainingTimer > 0.f && ActiveSlowMagnitude > 0.f)
    {
        const float SpeedMultiplier = FMath::Clamp(1.0f - ActiveSlowMagnitude, 0.1f, 1.0f);
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * SpeedMultiplier;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
    }
}

void AWyrmEnemyCharacter::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    if (Data.NewValue <= 0.f)
    {
        if (AbilitySystem)
        {
            static const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Dead")), false);
            if (DeadTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(DeadTag))
            {
                AbilitySystem->AddLooseGameplayTag(DeadTag);
            }
        }

        if (GetCharacterMovement())
        {
            GetCharacterMovement()->DisableMovement();
        }

        if (GetCapsuleComponent())
        {
            GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        OnEnemyDied.Broadcast(this);
    }
}

bool AWyrmEnemyCharacter::IsDefeated() const
{
    if (!Attributes)
    {
        return true;
    }

    if (Attributes->GetHealth() <= 0.f)
    {
        return true;
    }

    if (AbilitySystem)
    {
        static const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Dead")), false);
        if (DeadTag.IsValid() && AbilitySystem->HasMatchingGameplayTag(DeadTag))
        {
            return true;
        }
    }

    return false;
}

float AWyrmEnemyCharacter::GetCurrentSpeed() const
{
    return GetCharacterMovement() ? GetCharacterMovement()->MaxWalkSpeed : 0.f;
}

void AWyrmEnemyCharacter::ApplyNamedStatusEffect(FName TagName, float DurationSeconds, float Magnitude)
{
    const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
    if (Tag.IsValid())
    {
        ApplyStatusEffect(Tag, DurationSeconds, Magnitude);
    }
}

void AWyrmEnemyCharacter::SetInvulnerable(bool bInvulnerable)
{
    if (!AbilitySystem) return;
    const FGameplayTag InvulnTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Invulnerable")), false);
    if (InvulnTag.IsValid())
    {
        if (bInvulnerable)
        {
            AbilitySystem->AddLooseGameplayTag(InvulnTag);
        }
        else
        {
            AbilitySystem->RemoveLooseGameplayTag(InvulnTag);
        }
    }
}

bool AWyrmEnemyCharacter::HasMatchingGameplayTag(FName TagName) const
{
    if (!AbilitySystem) return false;
    const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
    return Tag.IsValid() && AbilitySystem->HasMatchingGameplayTag(Tag);
}

AWyrmEnemyCharacter* AWyrmEnemyCharacter::SpawnWyrmEnemy(UObject* WorldContextObject, EWyrmEnemyRole InRole, const FTransform& SpawnTransform)
{
    if (!WorldContextObject || !WorldContextObject->GetWorld())
    {
        return nullptr;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmEnemyCharacter* NewEnemy = WorldContextObject->GetWorld()->SpawnActor<AWyrmEnemyCharacter>(
        AWyrmEnemyCharacter::StaticClass(), SpawnTransform, Params);

    if (NewEnemy)
    {
        if (NewEnemy->GetAbilitySystemComponent())
        {
            NewEnemy->GetAbilitySystemComponent()->InitAbilityActorInfo(NewEnemy, NewEnemy);
            if (NewEnemy->Attributes && !NewEnemy->GetAbilitySystemComponent()->GetAttributeSet(UWyrmAttributeSet::StaticClass()))
            {
                NewEnemy->GetAbilitySystemComponent()->AddAttributeSetSubobject(NewEnemy->Attributes.Get());
            }

            static const FGameplayTag HostileTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Combat.Team.Hostile")), false);
            if (HostileTag.IsValid() && !NewEnemy->GetAbilitySystemComponent()->HasMatchingGameplayTag(HostileTag))
            {
                NewEnemy->GetAbilitySystemComponent()->AddLooseGameplayTag(HostileTag);
            }

            if (NewEnemy->Attributes)
            {
                FOnGameplayAttributeValueChange& HealthChanged =
                    NewEnemy->GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(NewEnemy->Attributes->GetHealthAttribute());
                HealthChanged.RemoveAll(NewEnemy);
                HealthChanged.AddUObject(NewEnemy, &AWyrmEnemyCharacter::HandleHealthChanged);
            }
        }
        NewEnemy->ConfigureForRole(InRole);
    }

    return NewEnemy;
}
