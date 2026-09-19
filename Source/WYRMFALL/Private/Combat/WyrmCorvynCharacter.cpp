#include "Combat/WyrmCorvynCharacter.h"
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

AWyrmCorvynCharacter::AWyrmCorvynCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsBoss = true;

    GetCapsuleComponent()->InitCapsuleSize(50.f, 95.f);
    BaseWalkSpeed = 550.f;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
    }
}

void AWyrmCorvynCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (Attributes)
    {
        // Boss profile: HP 750, Armor 12, Power 22
        Attributes->InitMaxHealth(750.f);
        Attributes->InitHealth(750.f);
        Attributes->InitArmor(12.f);
        Attributes->InitPower(22.f);
    }
}

void AWyrmCorvynCharacter::ReviveCorvyn(float NewHealth)
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
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
    }
}

bool AWyrmCorvynCharacter::PerformBeastStrike(AActor* TargetActor)
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

    // Beast strike: 15 raw damage (WeaponBase = 4.f, Power = 22.f: 4 + 0.5*22 = 15)
    const float RawDamage = UWyrmAttributeSet::CalculateRawDamage(4.f, Attributes->GetPower(), 0.5f);
    return UWyrmMeleeAttackAbility::ApplyDamageEffect(AbilitySystem, TargetASC, RawDamage);
}

bool AWyrmCorvynCharacter::PerformBeastPounce(AActor* TargetActor)
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

    // Beast pounce: 25 raw damage
    const float RawDamage = 25.f;
    return UWyrmMeleeAttackAbility::ApplyDamageEffect(AbilitySystem, TargetASC, RawDamage);
}

bool AWyrmCorvynCharacter::ResolveEncounterHostile(AWyrmCharacter* PlayerCharacter)
{
    return InternalResolve(PlayerCharacter, false);
}

bool AWyrmCorvynCharacter::ResolveEncounterMercy(AWyrmCharacter* PlayerCharacter)
{
    return InternalResolve(PlayerCharacter, true);
}

bool AWyrmCorvynCharacter::InternalResolve(AWyrmCharacter* PlayerCharacter, bool bIsMercy)
{
    if (!PlayerCharacter)
    {
        return false;
    }

    // Strict idempotency: reject duplicate rewards
    if (bEncounterResolved)
    {
        return false;
    }

    bEncounterResolved = true;
    bHostileResolved = !bIsMercy;
    bMercyResolved = bIsMercy;

    // 1. Grant permanent usable Echo: Moonbound Form outside bag
    PlayerCharacter->LearnEcho(FName(TEXT("MoonboundForm")));
    bEchoGranted = true;

    // 2. Commit distinct narrative facts with equivalent power receipt in UWyrmRegion01Subsystem
    UWorld* World = GetWorld();
    if (World)
    {
        UGameInstance* GI = World->GetGameInstance();
        if (GI)
        {
            UWyrmRegion01Subsystem* RegionSubsystem = GI->GetSubsystem<UWyrmRegion01Subsystem>();
            if (RegionSubsystem)
            {
                // Always commit the signature echo unlock
                RegionSubsystem->CommitFact(
                    FName(TEXT("echo.moonbound_form")),
                    FName(TEXT("region01.echo.moonbound_form")));

                // Commit specific narrative path fact
                if (bIsMercy)
                {
                    RegionSubsystem->CommitFact(
                        FName(TEXT("corvyn.cured")),
                        FName(TEXT("region01.corvyn.cured")));
                }
                else
                {
                    RegionSubsystem->CommitFact(
                        FName(TEXT("corvyn.defeated_hostile")),
                        FName(TEXT("region01.corvyn.defeated_hostile")));
                }
            }
        }
    }

    // 3. Ordinary loot grant with full-bag safety
    FWyrmItemInstance CorvynRelic;
    CorvynRelic.InstanceId = FGuid::NewGuid();
    CorvynRelic.ItemId = FName(TEXT("Item_CorvynRelic"));
    CorvynRelic.DisplayName = FText::FromString(TEXT("Corvyn's Silver Crest"));
    CorvynRelic.ItemType = EWyrmItemType::Resource;
    CorvynRelic.StackCount = 1;
    CorvynRelic.MaxStack = 20;

    UWyrmInventoryComponent* Inv = PlayerCharacter->GetInventory();
    if (Inv)
    {
        FWyrmItemInstance Remaining;
        if (Inv->AddItem(CorvynRelic, Remaining))
        {
            bOrdinaryLootDelivered = true;
        }
        else
        {
            // Bag was full; safely preserve loot on Corvyn for subsequent claim
            PreservedLoot.Add(CorvynRelic);
            bOrdinaryLootDelivered = false;
        }
    }
    else
    {
        PreservedLoot.Add(CorvynRelic);
    }

    return true;
}

bool AWyrmCorvynCharacter::ClaimPreservedLoot(AWyrmCharacter* PlayerCharacter)
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
