#include "Combat/WyrmCounselorCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "Combat/Abilities/WyrmMeleeAttackAbility.h"
#include "Player/WyrmCharacter.h"
#include "Inventory/WyrmInventoryComponent.h"
#include "Region/WyrmRegion01Subsystem.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameplayTagsManager.h"
#include "Kismet/GameplayStatics.h"

AWyrmCounselorCharacter::AWyrmCounselorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsBoss = true; // Boss profile for resistances

    GetCapsuleComponent()->InitCapsuleSize(45.f, 90.f);
    BaseWalkSpeed = 500.f;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
    }
}

void AWyrmCounselorCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (Attributes)
    {
        // Draft HP 600, Armor 10, Power 20 per REGION_01.md line 161
        Attributes->InitMaxHealth(600.f);
        Attributes->InitHealth(600.f);
        Attributes->InitArmor(10.f);
        Attributes->InitPower(20.f);
    }
}

void AWyrmCounselorCharacter::ReviveCounselor(float NewHealth)
{
    if (Attributes)
    {
        Attributes->SetCurrentHealth(NewHealth);
    }

    if (AbilitySystem)
    {
        static const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Dead")), false);
        if (DeadTag.IsValid() && AbilitySystem->HasMatchingGameplayTag(DeadTag))
        {
            AbilitySystem->RemoveLooseGameplayTag(DeadTag);
        }

        static const FGameplayTag RelentlessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.RelentlessAdvance")), false);
        if (RelentlessTag.IsValid() && AbilitySystem->HasMatchingGameplayTag(RelentlessTag))
        {
            AbilitySystem->RemoveLooseGameplayTag(RelentlessTag);
        }
    }

    bInRelentlessStance = false;
    bPunishableRecovery = false;
    StanceRemainingTimer = 0.f;
    StanceCooldownTimer = 0.f;
    RecoveryRemainingTimer = 0.f;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
    }
}

bool AWyrmCounselorCharacter::CanEnterStance() const
{
    if (IsDefeated() || bInRelentlessStance || StanceCooldownTimer > 0.f || bPunishableRecovery)
    {
        return false;
    }
    return true;
}

bool AWyrmCounselorCharacter::EnterRelentlessStance()
{
    if (!CanEnterStance())
    {
        return false;
    }

    bInRelentlessStance = true;
    StanceRemainingTimer = 6.0f;
    StanceCooldownTimer = 18.0f;

    if (AbilitySystem)
    {
        static const FGameplayTag RelentlessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.RelentlessAdvance")), false);
        if (RelentlessTag.IsValid() && !AbilitySystem->HasMatchingGameplayTag(RelentlessTag))
        {
            AbilitySystem->AddLooseGameplayTag(RelentlessTag);
        }
    }

    return true;
}

bool AWyrmCounselorCharacter::PerformCounselorStrike(AActor* TargetActor)
{
    if (!TargetActor || IsDefeated() || !AbilitySystem || !Attributes)
    {
        return false;
    }

    IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TargetActor);
    UAbilitySystemComponent* TargetASC = TargetASI ? TargetASI->GetAbilitySystemComponent() : nullptr;
    if (!TargetASC)
    {
        return false;
    }

    // Draft normal strike: 14 raw damage (WeaponBase = 4.f, Power = 20.f: 4 + 0.5*20 = 14)
    const float RawDamage = UWyrmAttributeSet::CalculateRawDamage(4.f, Attributes->GetPower(), 0.5f);
    return UWyrmMeleeAttackAbility::ApplyDamageEffect(AbilitySystem, TargetASC, RawDamage);
}

void AWyrmCounselorCharacter::ApplyStatusEffect(FGameplayTag StatusTag, float DurationSeconds, float Magnitude)
{
    if (IsDefeated())
    {
        return;
    }

    static const FGameplayTag SlowTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Slow")), false);
    static const FGameplayTag StaggerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.Stagger")), false);

    // During active Relentless Advance stance, slow and light/medium stagger are completely resisted!
    if (bInRelentlessStance)
    {
        if (StatusTag == SlowTag || StatusTag == StaggerTag)
        {
            return;
        }
    }

    Super::ApplyStatusEffect(StatusTag, DurationSeconds, Magnitude);
}

void AWyrmCounselorCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bInRelentlessStance)
    {
        StanceRemainingTimer -= DeltaSeconds;
        if (StanceRemainingTimer <= 0.f)
        {
            bInRelentlessStance = false;
            StanceRemainingTimer = 0.f;

            if (AbilitySystem)
            {
                static const FGameplayTag RelentlessTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Combat.RelentlessAdvance")), false);
                if (RelentlessTag.IsValid() && AbilitySystem->HasMatchingGameplayTag(RelentlessTag))
                {
                    AbilitySystem->RemoveLooseGameplayTag(RelentlessTag);
                }
            }

            // Enter 2.0s punishable recovery window
            bPunishableRecovery = true;
            RecoveryRemainingTimer = 2.0f;
        }
    }

    if (bPunishableRecovery)
    {
        RecoveryRemainingTimer -= DeltaSeconds;
        if (RecoveryRemainingTimer <= 0.f)
        {
            bPunishableRecovery = false;
            RecoveryRemainingTimer = 0.f;
        }
    }

    if (StanceCooldownTimer > 0.f)
    {
        StanceCooldownTimer = FMath::Max(0.f, StanceCooldownTimer - DeltaSeconds);
    }
}

bool AWyrmCounselorCharacter::ResolveEncounter(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter)
    {
        return false;
    }

    // Strict idempotency: if already resolved, reject duplicate reward
    if (bEncounterResolved)
    {
        return false;
    }

    bEncounterResolved = true;

    // 1. Grant permanent usable Echo: Relentless Advance outside bag
    PlayerCharacter->LearnEcho(FName(TEXT("RelentlessAdvance")));
    bEchoGranted = true;

    // 2. Commit fact and receipt in UWyrmRegion01Subsystem
    UWorld* World = GetWorld();
    if (World)
    {
        UGameInstance* GI = World->GetGameInstance();
        if (GI)
        {
            UWyrmRegion01Subsystem* RegionSubsystem = GI->GetSubsystem<UWyrmRegion01Subsystem>();
            if (RegionSubsystem)
            {
                RegionSubsystem->CommitFact(
                    FName(TEXT("echo.relentless_advance")),
                    FName(TEXT("region01.echo.relentless_advance")));
            }
        }
    }

    // 3. Ordinary loot grant with full-bag safety (ECHO-04)
    FWyrmItemInstance CounselorShard;
    CounselorShard.InstanceId = FGuid::NewGuid();
    CounselorShard.ItemId = FName(TEXT("Item_CounselorShard"));
    CounselorShard.DisplayName = FText::FromString(TEXT("Counselor's Mask Fragment"));
    CounselorShard.ItemType = EWyrmItemType::Resource;
    CounselorShard.StackCount = 1;
    CounselorShard.MaxStack = 20;

    UWyrmInventoryComponent* Inv = PlayerCharacter->GetInventory();
    if (Inv)
    {
        FWyrmItemInstance Remaining;
        if (Inv->AddItem(CounselorShard, Remaining))
        {
            bOrdinaryLootDelivered = true;
        }
        else
        {
            // Bag was full; safely preserve loot on Counselor for subsequent claim
            PreservedLoot.Add(CounselorShard);
            bOrdinaryLootDelivered = false;
        }
    }
    else
    {
        PreservedLoot.Add(CounselorShard);
    }

    return true;
}

bool AWyrmCounselorCharacter::ClaimPreservedLoot(AWyrmCharacter* PlayerCharacter)
{
    if (!PlayerCharacter || PreservedLoot.IsEmpty())
    {
        return false;
    }

    UWyrmInventoryComponent* Inv = PlayerCharacter->GetInventory();
    if (!Inv)
    {
        return false;
    }

    TArray<FWyrmItemInstance> StillPreserved;
    for (const FWyrmItemInstance& LootItem : PreservedLoot)
    {
        FWyrmItemInstance Remaining;
        if (!Inv->AddItem(LootItem, Remaining))
        {
            StillPreserved.Add(Remaining.StackCount > 0 ? Remaining : LootItem);
        }
    }

    PreservedLoot = StillPreserved;
    if (PreservedLoot.IsEmpty())
    {
        bOrdinaryLootDelivered = true;
    }

    return true;
}
