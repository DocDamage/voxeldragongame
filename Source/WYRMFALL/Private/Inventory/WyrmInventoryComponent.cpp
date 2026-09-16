#include "Inventory/WyrmInventoryComponent.h"
#include "Player/WyrmCharacter.h"
#include "Combat/WyrmAttributeSet.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

UWyrmInventoryComponent::UWyrmInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UWyrmInventoryComponent::AddItem(const FWyrmItemInstance& Item, FWyrmItemInstance& OutRemaining)
{
    OutRemaining = Item;
    if (!Item.IsValid())
    {
        return false;
    }

    int32 ToAdd = Item.StackCount;

    // 1. Try stacking into existing slots if stackable
    if (Item.MaxStack > 1)
    {
        for (FWyrmItemInstance& Existing : BagItems)
        {
            if (Existing.ItemId == Item.ItemId && Existing.StackCount < Existing.MaxStack)
            {
                int32 Space = Existing.MaxStack - Existing.StackCount;
                int32 Take = FMath::Min(ToAdd, Space);
                Existing.StackCount += Take;
                ToAdd -= Take;
                if (ToAdd <= 0)
                {
                    OutRemaining.StackCount = 0;
                    OnItemAdded.Broadcast(Existing, Item.StackCount);
                    return true;
                }
            }
        }
    }

    // 2. Place remaining count into new bag slots up to MaxBagSlots
    bool bKeptOriginalId = false;
    while (ToAdd > 0 && BagItems.Num() < MaxBagSlots)
    {
        int32 SlotTake = FMath::Min(ToAdd, Item.MaxStack);
        FWyrmItemInstance NewSlotItem = Item;
        if (!bKeptOriginalId && Item.InstanceId.IsValid())
        {
            NewSlotItem.InstanceId = Item.InstanceId;
            bKeptOriginalId = true;
        }
        else
        {
            NewSlotItem.InstanceId = FGuid::NewGuid();
        }
        NewSlotItem.StackCount = SlotTake;
        BagItems.Add(NewSlotItem);
        ToAdd -= SlotTake;
        OnItemAdded.Broadcast(NewSlotItem, SlotTake);
    }

    OutRemaining.StackCount = ToAdd;
    // Returns true if at least one item was added
    return ToAdd < Item.StackCount;
}

bool UWyrmInventoryComponent::RemoveItem(const FGuid& ItemInstanceId, int32 Count)
{
    if (Count <= 0)
    {
        return false;
    }

    int32 Index = BagItems.IndexOfByPredicate([&](const FWyrmItemInstance& It) { return It.InstanceId == ItemInstanceId; });
    if (Index == INDEX_NONE)
    {
        return false;
    }

    FWyrmItemInstance& Item = BagItems[Index];
    int32 Removed = FMath::Min(Count, Item.StackCount);
    Item.StackCount -= Removed;

    if (Item.StackCount <= 0)
    {
        BagItems.RemoveAt(Index);
    }

    OnItemRemoved.Broadcast(ItemInstanceId, Removed);
    return true;
}

bool UWyrmInventoryComponent::TransferToStash(const FGuid& ItemInstanceId, int32 Count)
{
    if (Count <= 0)
    {
        return false;
    }

    int32 BagIndex = BagItems.IndexOfByPredicate([&](const FWyrmItemInstance& It) { return It.InstanceId == ItemInstanceId; });
    if (BagIndex == INDEX_NONE)
    {
        return false;
    }

    const FWyrmItemInstance& SourceItem = BagItems[BagIndex];
    int32 ActualCount = FMath::Min(Count, SourceItem.StackCount);

    // Calculate capacity in Stash before modifying
    int32 AvailableSpace = 0;
    if (SourceItem.MaxStack > 1)
    {
        for (const FWyrmItemInstance& StashItem : StashItems)
        {
            if (StashItem.ItemId == SourceItem.ItemId && StashItem.StackCount < StashItem.MaxStack)
            {
                AvailableSpace += (StashItem.MaxStack - StashItem.StackCount);
            }
        }
    }
    int32 EmptySlots = MaxStashSlots - StashItems.Num();
    if (EmptySlots > 0)
    {
        AvailableSpace += EmptySlots * SourceItem.MaxStack;
    }

    if (AvailableSpace < ActualCount)
    {
        return false; // Not enough room in stash for atomic transfer
    }

    // Perform transfer
    FWyrmItemInstance StashTransferItem = SourceItem;
    StashTransferItem.StackCount = ActualCount;
    RemoveItem(ItemInstanceId, ActualCount);

    int32 ToAdd = ActualCount;
    if (StashTransferItem.MaxStack > 1)
    {
        for (FWyrmItemInstance& StashItem : StashItems)
        {
            if (StashItem.ItemId == StashTransferItem.ItemId && StashItem.StackCount < StashItem.MaxStack)
            {
                int32 Space = StashItem.MaxStack - StashItem.StackCount;
                int32 Take = FMath::Min(ToAdd, Space);
                StashItem.StackCount += Take;
                ToAdd -= Take;
                if (ToAdd <= 0)
                {
                    break;
                }
            }
        }
    }

    bool bKeptStashId = false;
    while (ToAdd > 0 && StashItems.Num() < MaxStashSlots)
    {
        int32 SlotTake = FMath::Min(ToAdd, StashTransferItem.MaxStack);
        FWyrmItemInstance NewSlotItem = StashTransferItem;
        if (!bKeptStashId && ItemInstanceId.IsValid())
        {
            NewSlotItem.InstanceId = ItemInstanceId;
            bKeptStashId = true;
        }
        else
        {
            NewSlotItem.InstanceId = FGuid::NewGuid();
        }
        NewSlotItem.StackCount = SlotTake;
        StashItems.Add(NewSlotItem);
        ToAdd -= SlotTake;
    }

    return true;
}

bool UWyrmInventoryComponent::TransferFromStash(const FGuid& ItemInstanceId, int32 Count)
{
    if (Count <= 0)
    {
        return false;
    }

    int32 StashIndex = StashItems.IndexOfByPredicate([&](const FWyrmItemInstance& It) { return It.InstanceId == ItemInstanceId; });
    if (StashIndex == INDEX_NONE)
    {
        return false;
    }

    const FWyrmItemInstance& StashItem = StashItems[StashIndex];
    int32 ActualCount = FMath::Min(Count, StashItem.StackCount);

    // Calculate capacity in Bag before modifying
    int32 AvailableSpace = 0;
    if (StashItem.MaxStack > 1)
    {
        for (const FWyrmItemInstance& BagIt : BagItems)
        {
            if (BagIt.ItemId == StashItem.ItemId && BagIt.StackCount < BagIt.MaxStack)
            {
                AvailableSpace += (BagIt.MaxStack - BagIt.StackCount);
            }
        }
    }
    int32 EmptySlots = MaxBagSlots - BagItems.Num();
    if (EmptySlots > 0)
    {
        AvailableSpace += EmptySlots * StashItem.MaxStack;
    }

    if (AvailableSpace < ActualCount)
    {
        return false;
    }

    FWyrmItemInstance BagTransferItem = StashItem;
    BagTransferItem.StackCount = ActualCount;

    // Remove from stash
    StashItems[StashIndex].StackCount -= ActualCount;
    if (StashItems[StashIndex].StackCount <= 0)
    {
        StashItems.RemoveAt(StashIndex);
    }

    // Add to bag
    FWyrmItemInstance Excess;
    return AddItem(BagTransferItem, Excess);
}

bool UWyrmInventoryComponent::TransferItem(UWyrmInventoryComponent* TargetInventory, const FGuid& ItemInstanceId, int32 Count)
{
    if (!TargetInventory || TargetInventory == this || Count <= 0)
    {
        return false;
    }

    int32 Index = BagItems.IndexOfByPredicate([&](const FWyrmItemInstance& It) { return It.InstanceId == ItemInstanceId; });
    if (Index == INDEX_NONE)
    {
        return false;
    }

    const FWyrmItemInstance& SourceItem = BagItems[Index];
    int32 ActualCount = FMath::Min(Count, SourceItem.StackCount);

    FWyrmItemInstance TransferInstance = SourceItem;
    TransferInstance.StackCount = ActualCount;

    FWyrmItemInstance Excess;
    // Check if target can accommodate full transfer
    if (!TargetInventory->AddItem(TransferInstance, Excess) || Excess.StackCount > 0)
    {
        // Revert any partial addition if target could not accept all
        if (Excess.StackCount < ActualCount)
        {
            int32 AddedToTarget = ActualCount - Excess.StackCount;
            TargetInventory->RemoveItem(TransferInstance.InstanceId, AddedToTarget);
        }
        return false;
    }

    RemoveItem(ItemInstanceId, ActualCount);
    return true;
}

bool UWyrmInventoryComponent::EquipItem(const FGuid& ItemInstanceId, EWyrmEquipSlot Slot)
{
    int32 BagIndex = BagItems.IndexOfByPredicate([&](const FWyrmItemInstance& It) { return It.InstanceId == ItemInstanceId; });
    if (BagIndex == INDEX_NONE)
    {
        return false;
    }

    FWyrmItemInstance ItemToEquip = BagItems[BagIndex];
    EWyrmEquipSlot TargetSlot = (Slot != EWyrmEquipSlot::None) ? Slot : ItemToEquip.DefaultSlot;
    if (TargetSlot == EWyrmEquipSlot::None)
    {
        return false;
    }

    // If slot is already occupied, swap with currently equipped item
    if (EquippedItems.Contains(TargetSlot))
    {
        FWyrmItemInstance OldItem = EquippedItems[TargetSlot];
        ApplyItemStats(OldItem, false);
        UpdateMeshAttachment(TargetSlot, OldItem, false);
        EquippedItems.Remove(TargetSlot);
        OnItemUnequipped.Broadcast(TargetSlot);

        // Put old item into the bag slot that is being vacated
        BagItems[BagIndex] = OldItem;
    }
    else
    {
        BagItems.RemoveAt(BagIndex);
    }

    EquippedItems.Add(TargetSlot, ItemToEquip);
    ApplyItemStats(ItemToEquip, true);
    UpdateMeshAttachment(TargetSlot, ItemToEquip, true);
    OnItemEquipped.Broadcast(TargetSlot, ItemToEquip);
    return true;
}

bool UWyrmInventoryComponent::UnequipItem(EWyrmEquipSlot Slot)
{
    if (!EquippedItems.Contains(Slot))
    {
        return false;
    }

    if (BagItems.Num() >= MaxBagSlots)
    {
        // Bag full; cannot unequip without dropping or overflowing
        return false;
    }

    FWyrmItemInstance Item = EquippedItems[Slot];
    EquippedItems.Remove(Slot);
    ApplyItemStats(Item, false);
    UpdateMeshAttachment(Slot, Item, false);

    BagItems.Add(Item);
    OnItemUnequipped.Broadcast(Slot);
    return true;
}

bool UWyrmInventoryComponent::GetEquippedItem(EWyrmEquipSlot Slot, FWyrmItemInstance& OutItem) const
{
    if (const FWyrmItemInstance* Found = EquippedItems.Find(Slot))
    {
        OutItem = *Found;
        return true;
    }
    OutItem = FWyrmItemInstance();
    return false;
}

bool UWyrmInventoryComponent::IsSlotEquipped(EWyrmEquipSlot Slot) const
{
    return EquippedItems.Contains(Slot);
}

float UWyrmInventoryComponent::GetEquippedStatBonus(FName StatName) const
{
    float Total = 0.f;
    for (const auto& Kvp : EquippedItems)
    {
        Total += Kvp.Value.GetStatValue(StatName);
    }
    return Total;
}

float UWyrmInventoryComponent::GetItemStatValue(const FWyrmItemInstance& Item, FName StatName)
{
    return Item.GetStatValue(StatName);
}

void UWyrmInventoryComponent::ClearAll()
{
    for (const auto& Kvp : EquippedItems)
    {
        ApplyItemStats(Kvp.Value, false);
        UpdateMeshAttachment(Kvp.Key, Kvp.Value, false);
    }
    EquippedItems.Empty();
    BagItems.Empty();
    StashItems.Empty();
}

void UWyrmInventoryComponent::RestoreInventoryState(
    const TArray<FWyrmItemInstance>& InBag,
    const TArray<FWyrmItemInstance>& InStash,
    const TMap<EWyrmEquipSlot, FWyrmItemInstance>& InEquipped)
{
    ClearAll();
    BagItems = InBag;
    StashItems = InStash;
    for (const auto& Kvp : InEquipped)
    {
        EquippedItems.Add(Kvp.Key, Kvp.Value);
        ApplyItemStats(Kvp.Value, true);
        UpdateMeshAttachment(Kvp.Key, Kvp.Value, true);
    }
}

void UWyrmInventoryComponent::ApplyItemStats(const FWyrmItemInstance& Item, bool bApply)
{
    AWyrmCharacter* Character = Cast<AWyrmCharacter>(GetOwner());
    if (!Character)
    {
        return;
    }

    UWyrmAttributeSet* Attrs = Character->GetAttributes();
    if (!Attrs)
    {
        return;
    }

    const float Sign = bApply ? 1.0f : -1.0f;

    for (const FWyrmItemRoll& Roll : Item.RolledStats)
    {
        if (Roll.StatName == FName(TEXT("Power")))
        {
            float NewVal = Attrs->GetCurrentPower() + (Sign * Roll.Value);
            Attrs->SetCurrentPower(FMath::Max(0.f, NewVal));
        }
        else if (Roll.StatName == FName(TEXT("Armor")))
        {
            float NewVal = Attrs->GetCurrentArmor() + (Sign * Roll.Value);
            Attrs->SetCurrentArmor(FMath::Max(0.f, NewVal));
        }
        else if (Roll.StatName == FName(TEXT("MaxHealth")))
        {
            float NewMaxHealth = Attrs->GetCurrentMaxHealth() + (Sign * Roll.Value);
            Attrs->SetCurrentMaxHealth(FMath::Max(1.f, NewMaxHealth));
            if (bApply)
            {
                Attrs->SetCurrentHealth(Attrs->GetCurrentHealth() + Roll.Value);
            }
            else
            {
                Attrs->SetCurrentHealth(FMath::Min(Attrs->GetCurrentHealth(), Attrs->GetCurrentMaxHealth()));
            }
        }
    }
}

void UWyrmInventoryComponent::UpdateMeshAttachment(EWyrmEquipSlot Slot, const FWyrmItemInstance& Item, bool bAttach)
{
    AWyrmCharacter* Character = Cast<AWyrmCharacter>(GetOwner());
    if (!Character)
    {
        return;
    }

    if (!bAttach)
    {
        if (TObjectPtr<UStaticMeshComponent>* FoundComp = EquippedMeshComponents.Find(Slot))
        {
            if (*FoundComp)
            {
                (*FoundComp)->DestroyComponent();
            }
            EquippedMeshComponents.Remove(Slot);
        }
        return;
    }

    FName SocketName = Item.AttachedSocketName.IsNone() ? GetDefaultSocketForSlot(Slot) : Item.AttachedSocketName;

    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Character);
    if (MeshComp)
    {
        MeshComp->RegisterComponent();
        if (Item.WorldMesh.IsValid())
        {
            MeshComp->SetStaticMesh(Item.WorldMesh.Get());
        }
        Character->AttachEquipmentMesh(MeshComp, SocketName);
        EquippedMeshComponents.Add(Slot, MeshComp);
    }
}

FName UWyrmInventoryComponent::GetDefaultSocketForSlot(EWyrmEquipSlot Slot) const
{
    switch (Slot)
    {
    case EWyrmEquipSlot::MainHand:
        return FName(TEXT("Hand_Right"));
    case EWyrmEquipSlot::OffHand:
        return FName(TEXT("Hand_Left"));
    case EWyrmEquipSlot::Back:
        return FName(TEXT("Back_Weapon"));
    case EWyrmEquipSlot::Head:
        return FName(TEXT("headSocket"));
    case EWyrmEquipSlot::Chest:
        return FName(TEXT("spine_02Socket"));
    default:
        return NAME_None;
    }
}

FWyrmItemInstance UWyrmInventoryComponent::RollRandomItem(FName ItemId, EWyrmItemType Type, int32 Level, EWyrmEquipSlot Slot)
{
    FWyrmItemInstance Item;
    Item.InstanceId = FGuid::NewGuid();
    Item.ItemId = ItemId;
    Item.ItemType = Type;
    Item.StackCount = 1;
    Item.MaxStack = 1;

    switch (Type)
    {
    case EWyrmItemType::Weapon:
        Item.DefaultSlot = (Slot != EWyrmEquipSlot::None) ? Slot : EWyrmEquipSlot::MainHand;
        Item.DisplayName = FText::FromString(FString::Printf(TEXT("Forged Blade Lv.%d"), Level));
        Item.AttachedSocketName = FName(TEXT("Hand_Right"));
        {
            FWyrmItemRoll PowerRoll;
            PowerRoll.StatName = FName(TEXT("Power"));
            PowerRoll.Value = 10.f + (Level * 3.5f);
            Item.RolledStats.Add(PowerRoll);
        }
        break;

    case EWyrmItemType::Armor:
        Item.DefaultSlot = (Slot != EWyrmEquipSlot::None) ? Slot : EWyrmEquipSlot::Chest;
        Item.DisplayName = FText::FromString(FString::Printf(TEXT("Plate Cuirass Lv.%d"), Level));
        Item.AttachedSocketName = FName(TEXT("spine_02Socket"));
        {
            FWyrmItemRoll ArmorRoll;
            ArmorRoll.StatName = FName(TEXT("Armor"));
            ArmorRoll.Value = 5.f + (Level * 2.0f);
            Item.RolledStats.Add(ArmorRoll);

            FWyrmItemRoll HealthRoll;
            HealthRoll.StatName = FName(TEXT("MaxHealth"));
            HealthRoll.Value = 20.f + (Level * 10.0f);
            Item.RolledStats.Add(HealthRoll);
        }
        break;

    case EWyrmItemType::Consumable:
        Item.DefaultSlot = EWyrmEquipSlot::None;
        Item.DisplayName = FText::FromString(TEXT("Wyrm Elixir"));
        Item.MaxStack = 20;
        break;

    case EWyrmItemType::Resource:
        Item.DefaultSlot = EWyrmEquipSlot::None;
        Item.DisplayName = FText::FromString(TEXT("Obsidian Ore"));
        Item.MaxStack = 99;
        break;

    default:
        break;
    }

    return Item;
}
