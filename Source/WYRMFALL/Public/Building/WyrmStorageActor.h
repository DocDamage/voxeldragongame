#pragma once

#include "CoreMinimal.h"
#include "Building/WyrmBuildingPiece.h"
#include "Inventory/WyrmInventoryComponent.h"
#include "WyrmStorageActor.generated.h"

/**
 * Authoritative storage container actor (e.g. Storage Chest) in camp.
 * Hosts a single UWyrmInventoryComponent as the storage inventory authority.
 */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Building")
class WYRMFALL_API AWyrmStorageActor : public AWyrmBuildingPiece
{
    GENERATED_BODY()

public:
    AWyrmStorageActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Building|Storage")
    TObjectPtr<UWyrmInventoryComponent> StorageInventory;

    /** Atomically transfers an item from player bag into this storage container (ACT-08) */
    UFUNCTION(BlueprintCallable, Category="Building|Storage")
    bool TransferToStorage(UWyrmInventoryComponent* SourceBag, const FGuid& ItemInstanceId, int32 Count = 1);

    /** Atomically transfers an item from this storage container into player bag (ACT-08) */
    UFUNCTION(BlueprintCallable, Category="Building|Storage")
    bool TransferFromStorage(UWyrmInventoryComponent* TargetBag, const FGuid& ItemInstanceId, int32 Count = 1);

    UFUNCTION(BlueprintPure, Category="Building|Storage")
    UWyrmInventoryComponent* GetStorageInventory() const { return StorageInventory; }
};
