#pragma once
#include "CoreMinimal.h"
#include "WyrmInventoryTypes.generated.h"

UENUM(BlueprintType)
enum class EWyrmItemType : uint8
{
    None        UMETA(DisplayName="None"),
    Weapon      UMETA(DisplayName="Weapon"),
    Armor       UMETA(DisplayName="Armor"),
    Consumable  UMETA(DisplayName="Consumable"),
    Resource    UMETA(DisplayName="Resource")
};

UENUM(BlueprintType)
enum class EWyrmEquipSlot : uint8
{
    None        UMETA(DisplayName="None"),
    MainHand    UMETA(DisplayName="Main Hand (Weapon)"),
    OffHand     UMETA(DisplayName="Off Hand (Shield)"),
    Head        UMETA(DisplayName="Head (Helmet)"),
    Chest       UMETA(DisplayName="Chest (Armor)"),
    Legs        UMETA(DisplayName="Legs (Boots)"),
    Back        UMETA(DisplayName="Back (Cloak/Weapon)")
};

UENUM(BlueprintType)
enum class EWyrmWeaponFamily : uint8
{
    Unarmed     UMETA(DisplayName="Unarmed"),
    Melee1H     UMETA(DisplayName="One-Handed Melee"),
    Melee2H     UMETA(DisplayName="Two-Handed Melee"),
    RangedBow   UMETA(DisplayName="Ranged Bow")
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmItemRoll
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    FName StatName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    float Value = 0.f;

    bool operator==(const FWyrmItemRoll& Other) const
    {
        return StatName == Other.StatName && FMath::IsNearlyEqual(Value, Other.Value);
    }
};

USTRUCT(BlueprintType)
struct WYRMFALL_API FWyrmItemInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    FGuid InstanceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    EWyrmItemType ItemType = EWyrmItemType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    EWyrmWeaponFamily WeaponFamily = EWyrmWeaponFamily::Unarmed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    EWyrmEquipSlot DefaultSlot = EWyrmEquipSlot::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    TArray<FWyrmItemRoll> RolledStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    int32 StackCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    int32 MaxStack = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    FName AttachedSocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    TSoftObjectPtr<UStaticMesh> WorldMesh;

    bool IsValid() const
    {
        return InstanceId.IsValid() && !ItemId.IsNone() && StackCount > 0;
    }

    float GetStatValue(FName InStatName) const
    {
        for (const FWyrmItemRoll& Roll : RolledStats)
        {
            if (Roll.StatName == InStatName)
            {
                return Roll.Value;
            }
        }
        return 0.f;
    }

    bool operator==(const FWyrmItemInstance& Other) const
    {
        return InstanceId == Other.InstanceId &&
               ItemId == Other.ItemId &&
               ItemType == Other.ItemType &&
               WeaponFamily == Other.WeaponFamily &&
               DefaultSlot == Other.DefaultSlot &&
               StackCount == Other.StackCount &&
               RolledStats == Other.RolledStats;
    }
};
