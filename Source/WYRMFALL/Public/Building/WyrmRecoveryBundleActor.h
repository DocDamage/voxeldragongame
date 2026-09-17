#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/WyrmInventoryComponent.h"
#include "WyrmRecoveryBundleActor.generated.h"

class UStaticMeshComponent;

/**
 * Authoritative recovery bundle actor spawned on demolition overflow (ACT-09).
 * Holds all uncollected items and refund materials safely until player claims them.
 * Automatically destroys itself when all items are claimed. Zero item loss or duplication.
 */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Building")
class WYRMFALL_API AWyrmRecoveryBundleActor : public AActor
{
    GENERATED_BODY()

public:
    AWyrmRecoveryBundleActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Building|Components")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Building|Recovery")
    TObjectPtr<UWyrmInventoryComponent> BundleInventory;

    /** Claims as many items as player bag can hold; if completely empty, destroys bundle (ACT-09) */
    UFUNCTION(BlueprintCallable, Category="Building|Recovery")
    bool ClaimAll(UWyrmInventoryComponent* PlayerBag);

    UFUNCTION(BlueprintPure, Category="Building|Recovery")
    UWyrmInventoryComponent* GetBundleInventory() const { return BundleInventory; }
};
