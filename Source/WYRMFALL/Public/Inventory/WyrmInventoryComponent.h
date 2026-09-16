#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/WyrmInventoryTypes.h"
#include "WyrmInventoryComponent.generated.h"

class AWyrmCharacter;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWyrmOnItemAdded, const FWyrmItemInstance&, Item, int32, CountAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWyrmOnItemRemoved, const FGuid&, ItemInstanceId, int32, CountRemoved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWyrmOnItemEquipped, EWyrmEquipSlot, Slot, const FWyrmItemInstance&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWyrmOnItemUnequipped, EWyrmEquipSlot, Slot);

// Single authoritative inventory and equipment component for player character.
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WYRMFALL_API UWyrmInventoryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UWyrmInventoryComponent();

    // --- Capacity ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Capacity")
    int32 MaxBagSlots = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Capacity")
    int32 MaxStashSlots = 50;

    // --- Events ---
    UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
    FWyrmOnItemAdded OnItemAdded;

    UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
    FWyrmOnItemRemoved OnItemRemoved;

    UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
    FWyrmOnItemEquipped OnItemEquipped;

    UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
    FWyrmOnItemUnequipped OnItemUnequipped;

    // --- Item Operations ---
    /** Adds item into bag with stacking and overflow protection. If full, returns remaining in OutRemaining (COM-07) */
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool AddItem(const FWyrmItemInstance& Item, FWyrmItemInstance& OutRemaining);

    /** Removes item from bag */
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool RemoveItem(const FGuid& ItemInstanceId, int32 Count = 1);

    /** Transfers item from Bag to Stash atomically */
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool TransferToStash(const FGuid& ItemInstanceId, int32 Count = 1);

    /** Transfers item from Stash to Bag atomically */
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool TransferFromStash(const FGuid& ItemInstanceId, int32 Count = 1);

    /** Transfers item between two separate inventory components atomically */
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool TransferItem(UWyrmInventoryComponent* TargetInventory, const FGuid& ItemInstanceId, int32 Count = 1);

    /** Equips item from bag to designated or default slot, updating GAS attributes and mesh (COM-06) */
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool EquipItem(const FGuid& ItemInstanceId, EWyrmEquipSlot Slot = EWyrmEquipSlot::None);

    /** Unequips item from slot back to bag, reverting GAS attributes and detaching mesh (COM-06) */
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool UnequipItem(EWyrmEquipSlot Slot);

    UFUNCTION(BlueprintPure, Category="Inventory")
    bool GetEquippedItem(EWyrmEquipSlot Slot, FWyrmItemInstance& OutItem) const;

    UFUNCTION(BlueprintPure, Category="Inventory")
    bool IsSlotEquipped(EWyrmEquipSlot Slot) const;

    UFUNCTION(BlueprintPure, Category="Inventory")
    const TArray<FWyrmItemInstance>& GetBagItems() const { return BagItems; }

    UFUNCTION(BlueprintPure, Category="Inventory")
    const TArray<FWyrmItemInstance>& GetStashItems() const { return StashItems; }

    UFUNCTION(BlueprintPure, Category="Inventory")
    const TMap<EWyrmEquipSlot, FWyrmItemInstance>& GetEquippedItems() const { return EquippedItems; }

    UFUNCTION(BlueprintPure, Category="Inventory")
    float GetEquippedStatBonus(FName StatName) const;

    UFUNCTION(BlueprintPure, Category="Inventory")
    static float GetItemStatValue(const FWyrmItemInstance& Item, FName StatName);

    UFUNCTION(BlueprintCallable, Category="Inventory")
    void ClearAll();

    /** Restores complete inventory state from save record */
    UFUNCTION(BlueprintCallable, Category="Inventory")
    void RestoreInventoryState(
        const TArray<FWyrmItemInstance>& InBag,
        const TArray<FWyrmItemInstance>& InStash,
        const TMap<EWyrmEquipSlot, FWyrmItemInstance>& InEquipped);

    // --- Generation & Factory Helper (COM-06) ---
    UFUNCTION(BlueprintCallable, Category="Inventory")
    static FWyrmItemInstance RollRandomItem(FName ItemId, EWyrmItemType Type, int32 Level, EWyrmEquipSlot Slot = EWyrmEquipSlot::None);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory|State")
    TArray<FWyrmItemInstance> BagItems;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory|State")
    TArray<FWyrmItemInstance> StashItems;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory|State")
    TMap<EWyrmEquipSlot, FWyrmItemInstance> EquippedItems;

    UPROPERTY(Transient)
    TMap<EWyrmEquipSlot, TObjectPtr<UStaticMeshComponent>> EquippedMeshComponents;

    void ApplyItemStats(const FWyrmItemInstance& Item, bool bApply);
    void UpdateMeshAttachment(EWyrmEquipSlot Slot, const FWyrmItemInstance& Item, bool bAttach);
    FName GetDefaultSocketForSlot(EWyrmEquipSlot Slot) const;
};
