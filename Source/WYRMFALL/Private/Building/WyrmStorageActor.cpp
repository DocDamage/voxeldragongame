#include "Building/WyrmStorageActor.h"

AWyrmStorageActor::AWyrmStorageActor()
{
    PieceType = EWyrmBuildingPieceType::StorageChest;
    BoundsExtent = FVector(60.f, 60.f, 60.f);

    StorageInventory = CreateDefaultSubobject<UWyrmInventoryComponent>(TEXT("StorageInventory"));
    StorageInventory->MaxBagSlots = 16;
    StorageInventory->MaxStashSlots = 0;
}

bool AWyrmStorageActor::TransferToStorage(UWyrmInventoryComponent* SourceBag, const FGuid& ItemInstanceId, int32 Count)
{
    if (!SourceBag || !StorageInventory)
    {
        return false;
    }
    return SourceBag->TransferItem(StorageInventory, ItemInstanceId, Count);
}

bool AWyrmStorageActor::TransferFromStorage(UWyrmInventoryComponent* TargetBag, const FGuid& ItemInstanceId, int32 Count)
{
    if (!TargetBag || !StorageInventory)
    {
        return false;
    }
    return StorageInventory->TransferItem(TargetBag, ItemInstanceId, Count);
}
