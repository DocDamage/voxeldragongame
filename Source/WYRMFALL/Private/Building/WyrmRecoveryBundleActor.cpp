#include "Building/WyrmRecoveryBundleActor.h"
#include "Components/StaticMeshComponent.h"

AWyrmRecoveryBundleActor::AWyrmRecoveryBundleActor()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(ECC_WorldStatic);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

    BundleInventory = CreateDefaultSubobject<UWyrmInventoryComponent>(TEXT("BundleInventory"));
    BundleInventory->MaxBagSlots = 64;
    BundleInventory->MaxStashSlots = 0;
}

bool AWyrmRecoveryBundleActor::ClaimAll(UWyrmInventoryComponent* PlayerBag)
{
    if (!PlayerBag || !BundleInventory)
    {
        return false;
    }

    const TArray<FWyrmItemInstance> Items = BundleInventory->GetBagItems();
    bool bAnyClaimed = false;

    for (const FWyrmItemInstance& Item : Items)
    {
        if (BundleInventory->TransferItem(PlayerBag, Item.InstanceId, Item.StackCount))
        {
            bAnyClaimed = true;
        }
    }

    if (BundleInventory->GetBagItems().IsEmpty())
    {
        Destroy();
    }

    return bAnyClaimed;
}
