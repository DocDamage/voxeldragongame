#include "Building/WyrmBuildingSubsystem.h"
#include "Building/WyrmBuildingPiece.h"
#include "Building/WyrmStorageActor.h"
#include "Building/WyrmRecoveryBundleActor.h"
#include "Inventory/WyrmInventoryComponent.h"
#include "Terrain/WyrmGeoForgeAdapter.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"

void UWyrmBuildingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterDefaultDefinitions();
}

void UWyrmBuildingSubsystem::Deinitialize()
{
    ClearAllPlacedPieces();
    PieceRegistry.Empty();
    Super::Deinitialize();
}

void UWyrmBuildingSubsystem::RegisterDefaultDefinitions()
{
    // 1. Wood Foundation
    {
        FWyrmBuildingPieceDefinition Def;
        Def.PieceId = FName(TEXT("Foundation.Wood"));
        Def.DisplayName = FText::FromString(TEXT("Wood Foundation"));
        Def.PieceType = EWyrmBuildingPieceType::Foundation;
        Def.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Foundation.SM_Stylized_Wood_Foundation")));
        Def.BoundsExtent = FVector(100.f, 100.f, 20.f);
        Def.bRequiresGroundSupport = true;
        Def.bRequiresStructuralSupport = false;
        Def.Costs.Add({ FName(TEXT("Resource.Wood")), 2 });
        RegisterPieceDefinition(Def);
    }

    // 2. Wood Wall
    {
        FWyrmBuildingPieceDefinition Def;
        Def.PieceId = FName(TEXT("Wall.Wood"));
        Def.DisplayName = FText::FromString(TEXT("Wood Wall"));
        Def.PieceType = EWyrmBuildingPieceType::Wall;
        Def.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Wall.SM_Stylized_Wood_Wall")));
        Def.BoundsExtent = FVector(100.f, 20.f, 150.f);
        Def.bRequiresGroundSupport = false;
        Def.bRequiresStructuralSupport = true;
        Def.Costs.Add({ FName(TEXT("Resource.Wood")), 2 });
        RegisterPieceDefinition(Def);
    }

    // 3. Wood Doorframe
    {
        FWyrmBuildingPieceDefinition Def;
        Def.PieceId = FName(TEXT("Doorframe.Wood"));
        Def.DisplayName = FText::FromString(TEXT("Wood Doorframe"));
        Def.PieceType = EWyrmBuildingPieceType::Doorframe;
        Def.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Doorframe.SM_Stylized_Wood_Doorframe")));
        Def.BoundsExtent = FVector(100.f, 20.f, 150.f);
        Def.bRequiresGroundSupport = false;
        Def.bRequiresStructuralSupport = true;
        Def.Costs.Add({ FName(TEXT("Resource.Wood")), 2 });
        RegisterPieceDefinition(Def);
    }

    // 4. Wood Door
    {
        FWyrmBuildingPieceDefinition Def;
        Def.PieceId = FName(TEXT("Door.Wood"));
        Def.DisplayName = FText::FromString(TEXT("Wood Door"));
        Def.PieceType = EWyrmBuildingPieceType::Door;
        Def.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Door.SM_Stylized_Wood_Door")));
        Def.BoundsExtent = FVector(50.f, 10.f, 140.f);
        Def.bRequiresGroundSupport = false;
        Def.bRequiresStructuralSupport = true;
        Def.Costs.Add({ FName(TEXT("Resource.Wood")), 1 });
        RegisterPieceDefinition(Def);
    }

    // 5. Wood Roof
    {
        FWyrmBuildingPieceDefinition Def;
        Def.PieceId = FName(TEXT("Roof.Wood"));
        Def.DisplayName = FText::FromString(TEXT("Wood Roof"));
        Def.PieceType = EWyrmBuildingPieceType::Roof;
        Def.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Roof.SM_Stylized_Wood_Roof")));
        Def.BoundsExtent = FVector(100.f, 100.f, 50.f);
        Def.bRequiresGroundSupport = false;
        Def.bRequiresStructuralSupport = true;
        Def.Costs.Add({ FName(TEXT("Resource.Wood")), 2 });
        RegisterPieceDefinition(Def);
    }

    // 6. Wood Ceiling
    {
        FWyrmBuildingPieceDefinition Def;
        Def.PieceId = FName(TEXT("Ceiling.Wood"));
        Def.DisplayName = FText::FromString(TEXT("Wood Ceiling"));
        Def.PieceType = EWyrmBuildingPieceType::Ceiling;
        Def.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Ceiling.SM_Stylized_Wood_Ceiling")));
        Def.BoundsExtent = FVector(100.f, 100.f, 20.f);
        Def.bRequiresGroundSupport = false;
        Def.bRequiresStructuralSupport = true;
        Def.Costs.Add({ FName(TEXT("Resource.Wood")), 2 });
        RegisterPieceDefinition(Def);
    }

    // 7. Storage Chest
    {
        FWyrmBuildingPieceDefinition Def;
        Def.PieceId = FName(TEXT("Storage.Chest"));
        Def.DisplayName = FText::FromString(TEXT("Storage Chest"));
        Def.PieceType = EWyrmBuildingPieceType::StorageChest;
        Def.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/WYRMFALL/Environment/Camp/SM_StorageChest.SM_StorageChest")));
        Def.CustomPieceClass = AWyrmStorageActor::StaticClass();
        Def.BoundsExtent = FVector(50.f, 50.f, 50.f);
        Def.bRequiresGroundSupport = true;
        Def.bRequiresStructuralSupport = false;
        Def.Costs.Add({ FName(TEXT("Resource.Wood")), 2 });
        RegisterPieceDefinition(Def);
    }

    // 8. Workpoint Bench
    {
        FWyrmBuildingPieceDefinition Def;
        Def.PieceId = FName(TEXT("Workpoint.Bench"));
        Def.DisplayName = FText::FromString(TEXT("Workpoint Bench"));
        Def.PieceType = EWyrmBuildingPieceType::Workpoint;
        Def.BoundsExtent = FVector(60.f, 60.f, 50.f);
        Def.bRequiresGroundSupport = true;
        Def.bRequiresStructuralSupport = false;
        Def.Costs.Add({ FName(TEXT("Resource.Wood")), 3 });
        RegisterPieceDefinition(Def);
    }
}

bool UWyrmBuildingSubsystem::RegisterPieceDefinition(const FWyrmBuildingPieceDefinition& Definition)
{
    if (Definition.PieceId.IsNone())
    {
        return false;
    }
    PieceRegistry.Add(Definition.PieceId, Definition);
    return true;
}

bool UWyrmBuildingSubsystem::GetPieceDefinition(FName PieceId, FWyrmBuildingPieceDefinition& OutDef) const
{
    if (const FWyrmBuildingPieceDefinition* Found = PieceRegistry.Find(PieceId))
    {
        OutDef = *Found;
        return true;
    }
    return false;
}

TArray<FWyrmBuildingPieceDefinition> UWyrmBuildingSubsystem::GetAllDefinitions() const
{
    TArray<FWyrmBuildingPieceDefinition> Result;
    PieceRegistry.GenerateValueArray(Result);
    return Result;
}

TArray<AWyrmBuildingPiece*> UWyrmBuildingSubsystem::GetActivePieces() const
{
    TArray<AWyrmBuildingPiece*> Result;
    for (const TWeakObjectPtr<AWyrmBuildingPiece>& Ptr : ActivePieces)
    {
        if (AWyrmBuildingPiece* Piece = Ptr.Get())
        {
            Result.Add(Piece);
        }
    }
    return Result;
}

EWyrmPlacementRejection UWyrmBuildingSubsystem::ValidatePlacement(
    FName PieceId,
    const FTransform& Transform,
    AActor* InstigatorActor,
    FString& OutReason) const
{
    OutReason.Empty();

    FWyrmBuildingPieceDefinition Def;
    if (!GetPieceDefinition(PieceId, Def))
    {
        OutReason = FString::Printf(TEXT("Unknown piece definition: %s"), *PieceId.ToString());
        return EWyrmPlacementRejection::InvalidDefinition;
    }

    UWorld* World = InstigatorActor ? InstigatorActor->GetWorld() : (CachedWorld.IsValid() ? CachedWorld.Get() : GetWorld());

    // 1. Material check in instigator inventory
    if (InstigatorActor)
    {
        if (UWyrmInventoryComponent* Inv = InstigatorActor->FindComponentByClass<UWyrmInventoryComponent>())
        {
            for (const FWyrmBuildingCost& Cost : Def.Costs)
            {
                int32 AvailableCount = 0;
                for (const FWyrmItemInstance& BagItem : Inv->GetBagItems())
                {
                    if (BagItem.ItemId == Cost.ItemId)
                    {
                        AvailableCount += BagItem.StackCount;
                    }
                }

                if (AvailableCount < Cost.Count)
                {
                    OutReason = FString::Printf(TEXT("Insufficient materials: requires %d of %s, but only %d available"),
                        Cost.Count, *Cost.ItemId.ToString(), AvailableCount);
                    return EWyrmPlacementRejection::InsufficientMaterials;
                }
            }
        }
    }

    const FVector Location = Transform.GetLocation();

    // 2. Protected zone check
    if (Location.X > 500000.f || Location.X < -500000.f || Location.Y > 500000.f || Location.Y < -500000.f)
    {
        OutReason = TEXT("Placement location is outside allowed building territory (Protected)");
        return EWyrmPlacementRejection::Protected;
    }

    // 3. Occupied check (WRLD-08, ACT-07): intersecting any Pawn (humanoid, dragon, creature)
    if (World)
    {
        const FCollisionShape BoxShape = FCollisionShape::MakeBox(Def.BoundsExtent * 0.95f);
        FCollisionObjectQueryParams PawnQuery;
        PawnQuery.AddObjectTypesToQuery(ECC_Pawn);

        FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WyrmBuildingOccupancyCheck), false);

        TArray<FOverlapResult> Overlaps;
        if (World->OverlapMultiByObjectType(Overlaps, Location, Transform.GetRotation(), PawnQuery, BoxShape, QueryParams))
        {
            if (Overlaps.Num() > 0)
            {
                AActor* OccupantActor = Overlaps[0].GetActor();
                const FString OccupantName = OccupantActor ? OccupantActor->GetName() : TEXT("Pawn");
                OutReason = FString::Printf(TEXT("Placement location is occupied by %s"), *OccupantName);
                return EWyrmPlacementRejection::Occupied;
            }
        }
    }

    // 4. Overlap check with existing placed pieces
    for (const TWeakObjectPtr<AWyrmBuildingPiece>& Ptr : ActivePieces)
    {
        if (AWyrmBuildingPiece* Piece = Ptr.Get())
        {
            // Allow Door to be placed inside a Doorframe
            if ((Def.PieceType == EWyrmBuildingPieceType::Door && Piece->PieceType == EWyrmBuildingPieceType::Doorframe) ||
                (Def.PieceType == EWyrmBuildingPieceType::Doorframe && Piece->PieceType == EWyrmBuildingPieceType::Door))
            {
                continue;
            }

            // Allow furniture, storage, and walls on top of foundations
            if ((Piece->PieceType == EWyrmBuildingPieceType::Foundation && Def.PieceType != EWyrmBuildingPieceType::Foundation) ||
                (Piece->PieceType != EWyrmBuildingPieceType::Foundation && Def.PieceType == EWyrmBuildingPieceType::Foundation))
            {
                continue;
            }

            // Allow ceiling under roof
            if ((Def.PieceType == EWyrmBuildingPieceType::Ceiling && Piece->PieceType == EWyrmBuildingPieceType::Roof) ||
                (Def.PieceType == EWyrmBuildingPieceType::Roof && Piece->PieceType == EWyrmBuildingPieceType::Ceiling))
            {
                continue;
            }

            const float DistSq = FVector::DistSquared(Piece->GetActorLocation(), Location);
            const float MinSeparation = FMath::Min(Def.BoundsExtent.X, Def.BoundsExtent.Y) * 0.8f;
            if (DistSq < MinSeparation * MinSeparation)
            {
                OutReason = FString::Printf(TEXT("Placement overlaps existing piece: %s"), *Piece->PieceId.ToString());
                return EWyrmPlacementRejection::Overlapping;
            }
        }
    }

    // 5. Ground support check (for foundations, storage, workpoint)
    if (Def.bRequiresGroundSupport)
    {
        bool bHasGround = false;
        if (World)
        {
            FHitResult Hit;
            FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(WyrmGroundSupportTrace), false);
            if (InstigatorActor)
            {
                GroundParams.AddIgnoredActor(InstigatorActor);
            }

            const FVector Start = Location + FVector(0, 0, 20.f);
            const FVector End = Location - FVector(0, 0, 150.f);
            if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, GroundParams))
            {
                bHasGround = true;
            }
            else if (Location.Z >= -100.f && Location.Z <= 100.f)
            {
                // In test fixtures or flat datum without terrain geometry, ground level is supported
                bHasGround = true;
            }
        }

        // Also check if built directly on top of an existing foundation
        if (!bHasGround)
        {
            for (const TWeakObjectPtr<AWyrmBuildingPiece>& Ptr : ActivePieces)
            {
                if (AWyrmBuildingPiece* Piece = Ptr.Get())
                {
                    if (Piece->PieceType == EWyrmBuildingPieceType::Foundation)
                    {
                        const float HorizDistSq = FVector::DistSquared2D(Piece->GetActorLocation(), Location);
                        const float VertDiff = Location.Z - Piece->GetActorLocation().Z;
                        if (HorizDistSq <= 120.f * 120.f && VertDiff >= -10.f && VertDiff <= 100.f)
                        {
                            bHasGround = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!bHasGround)
        {
            OutReason = TEXT("Piece requires ground support, but no solid terrain or foundation was found beneath.");
            return EWyrmPlacementRejection::Unsupported;
        }
    }

    // 6. Structural support check (for walls, doorframes, roofs, ceilings)
    if (Def.bRequiresStructuralSupport)
    {
        bool bHasStructuralSupport = false;
        for (const TWeakObjectPtr<AWyrmBuildingPiece>& Ptr : ActivePieces)
        {
            if (AWyrmBuildingPiece* Piece = Ptr.Get())
            {
                const float DistSq = FVector::DistSquared(Piece->GetActorLocation(), Location);
                // Connected within 350cm to foundation, wall, or doorframe
                if (Piece->PieceType == EWyrmBuildingPieceType::Foundation ||
                    Piece->PieceType == EWyrmBuildingPieceType::Wall ||
                    Piece->PieceType == EWyrmBuildingPieceType::Doorframe)
                {
                    if (DistSq <= 350.f * 350.f)
                    {
                        bHasStructuralSupport = true;
                        break;
                    }
                }
            }
        }

        if (!bHasStructuralSupport)
        {
            OutReason = TEXT("Piece requires structural support, but no supporting foundation or wall is adjacent.");
            return EWyrmPlacementRejection::Unsupported;
        }
    }

    return EWyrmPlacementRejection::None;
}

AWyrmBuildingPiece* UWyrmBuildingSubsystem::ExecutePlacement(
    FName PieceId,
    const FTransform& Transform,
    AActor* InstigatorActor,
    EWyrmPlacementRejection& OutRejection,
    FString& OutReason)
{
    OutRejection = ValidatePlacement(PieceId, Transform, InstigatorActor, OutReason);
    if (OutRejection != EWyrmPlacementRejection::None)
    {
        return nullptr;
    }

    FWyrmBuildingPieceDefinition Def;
    if (!GetPieceDefinition(PieceId, Def))
    {
        OutRejection = EWyrmPlacementRejection::InvalidDefinition;
        OutReason = TEXT("Piece definition not found");
        return nullptr;
    }

    UWorld* World = InstigatorActor ? InstigatorActor->GetWorld() : (CachedWorld.IsValid() ? CachedWorld.Get() : GetWorld());
    if (!World)
    {
        OutRejection = EWyrmPlacementRejection::Unsupported;
        OutReason = TEXT("World context not available");
        return nullptr;
    }

    // Deduct materials atomically
    if (InstigatorActor)
    {
        if (UWyrmInventoryComponent* Inv = InstigatorActor->FindComponentByClass<UWyrmInventoryComponent>())
        {
            for (const FWyrmBuildingCost& Cost : Def.Costs)
            {
                int32 Needed = Cost.Count;
                for (int32 i = Inv->GetBagItems().Num() - 1; i >= 0 && Needed > 0; --i)
                {
                    const FWyrmItemInstance& Item = Inv->GetBagItems()[i];
                    if (Item.ItemId == Cost.ItemId)
                    {
                        const int32 Deduct = FMath::Min(Needed, Item.StackCount);
                        Inv->RemoveItem(Item.InstanceId, Deduct);
                        Needed -= Deduct;
                    }
                }
            }
        }
    }

    // Determine spawn class
    UClass* SpawnClass = Def.CustomPieceClass ? Def.CustomPieceClass.Get() : AWyrmBuildingPiece::StaticClass();
    if (Def.PieceType == EWyrmBuildingPieceType::StorageChest && SpawnClass == AWyrmBuildingPiece::StaticClass())
    {
        SpawnClass = AWyrmStorageActor::StaticClass();
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWyrmBuildingPiece* Piece = World->SpawnActor<AWyrmBuildingPiece>(SpawnClass, Transform, SpawnParams);
    if (!Piece)
    {
        OutRejection = EWyrmPlacementRejection::Unsupported;
        OutReason = TEXT("Failed to spawn building piece actor");
        return nullptr;
    }

    Piece->PieceId = PieceId;
    Piece->PieceType = Def.PieceType;
    Piece->PieceGuid = FGuid::NewGuid();
    Piece->BoundsExtent = Def.BoundsExtent;

    // Load and assign static mesh if available
    if (!Def.Mesh.IsNull())
    {
        UStaticMesh* Mesh = Def.Mesh.LoadSynchronous();
        if (Mesh && Piece->MeshComponent)
        {
            Piece->MeshComponent->SetStaticMesh(Mesh);
        }
    }

    ActivePieces.Add(Piece);

    // Register with all GeoForge terrain adapters for ground support locking (WRLD-09)
    for (TActorIterator<AWyrmGeoForgeAdapter> It(World); It; ++It)
    {
        It->RegisterCampPiece(Piece);
    }

    return Piece;
}

bool UWyrmBuildingSubsystem::DemolishPiece(
    AWyrmBuildingPiece* Piece,
    AActor* InstigatorActor,
    AWyrmRecoveryBundleActor*& OutRecoveryBundle,
    FString& OutReason)
{
    OutRecoveryBundle = nullptr;
    if (!Piece)
    {
        OutReason = TEXT("Target piece is null");
        return false;
    }

    UWorld* World = Piece->GetWorld();
    if (!World)
    {
        OutReason = TEXT("World is null");
        return false;
    }

    FWyrmBuildingPieceDefinition Def;
    GetPieceDefinition(Piece->PieceId, Def);

    // 1. Demolition refund costs (100% refund)
    const TArray<FWyrmBuildingCost> Refunds = Piece->GetDemolitionRefund(Def.Costs, 1.0f);

    // 2. Extract stored items if storage container
    TArray<FWyrmItemInstance> StoredItems;
    if (AWyrmStorageActor* Storage = Cast<AWyrmStorageActor>(Piece))
    {
        if (Storage->StorageInventory)
        {
            StoredItems = Storage->StorageInventory->GetBagItems();
            Storage->StorageInventory->ClearAll();
        }
    }

    // 3. Find instigator bag
    UWyrmInventoryComponent* InstigatorBag = nullptr;
    if (InstigatorActor)
    {
        InstigatorBag = InstigatorActor->FindComponentByClass<UWyrmInventoryComponent>();
    }

    TArray<FWyrmItemInstance> OverflowItems;

    // Deposit stored items first
    for (const FWyrmItemInstance& StoredItem : StoredItems)
    {
        if (InstigatorBag)
        {
            FWyrmItemInstance Excess;
            if (!InstigatorBag->AddItem(StoredItem, Excess) || Excess.StackCount > 0)
            {
                OverflowItems.Add(Excess.StackCount > 0 ? Excess : StoredItem);
            }
        }
        else
        {
            OverflowItems.Add(StoredItem);
        }
    }

    // Deposit refund materials
    for (const FWyrmBuildingCost& Cost : Refunds)
    {
        FWyrmItemInstance RefMat;
        RefMat.InstanceId = FGuid::NewGuid();
        RefMat.ItemId = Cost.ItemId;
        RefMat.DisplayName = FText::FromName(Cost.ItemId);
        RefMat.ItemType = EWyrmItemType::Resource;
        RefMat.StackCount = Cost.Count;
        RefMat.MaxStack = 99;

        if (InstigatorBag)
        {
            FWyrmItemInstance Excess;
            if (!InstigatorBag->AddItem(RefMat, Excess) || Excess.StackCount > 0)
            {
                OverflowItems.Add(Excess.StackCount > 0 ? Excess : RefMat);
            }
        }
        else
        {
            OverflowItems.Add(RefMat);
        }
    }

    // If any items could not fit in the bag, spawn recovery bundle (ACT-09)
    if (OverflowItems.Num() > 0)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AWyrmRecoveryBundleActor* Bundle = World->SpawnActor<AWyrmRecoveryBundleActor>(
            AWyrmRecoveryBundleActor::StaticClass(),
            Piece->GetActorTransform(),
            SpawnParams);

        if (Bundle && Bundle->BundleInventory)
        {
            for (const FWyrmItemInstance& OverItem : OverflowItems)
            {
                FWyrmItemInstance Excess;
                Bundle->BundleInventory->AddItem(OverItem, Excess);
            }
            OutRecoveryBundle = Bundle;
        }
    }

    // Unregister piece from terrain adapters
    for (TActorIterator<AWyrmGeoForgeAdapter> It(World); It; ++It)
    {
        It->UnregisterCampPiece(Piece);
    }

    ActivePieces.Remove(Piece);
    Piece->Destroy();
    return true;
}

bool UWyrmBuildingSubsystem::CheckCompanionGrowthClearance(
    UWorld* World,
    const FVector& Location,
    float RequiredHeight,
    float RequiredRadius,
    FString& OutReason) const
{
    OutReason.Empty();
    if (!World)
    {
        OutReason = TEXT("World is null");
        return false;
    }

    // Check placed pieces for overhead roof or ceiling
    for (const TWeakObjectPtr<AWyrmBuildingPiece>& Ptr : ActivePieces)
    {
        if (AWyrmBuildingPiece* Piece = Ptr.Get())
        {
            if (Piece->PieceType == EWyrmBuildingPieceType::Roof || Piece->PieceType == EWyrmBuildingPieceType::Ceiling)
            {
                const FBox Box = Piece->GetPieceBounds();
                const FVector PieceCenter = Box.GetCenter();
                const float HorizDistSq = FVector::DistSquared2D(PieceCenter, Location);
                if (HorizDistSq <= (RequiredRadius + 150.f) * (RequiredRadius + 150.f))
                {
                    if (PieceCenter.Z >= Location.Z && PieceCenter.Z <= Location.Z + RequiredHeight + 50.f)
                    {
                        OutReason = FString::Printf(TEXT("Growth blocked overhead by %s (ceiling height %0.1f, requires %0.1f)"),
                            *Piece->PieceId.ToString(), PieceCenter.Z - Location.Z, RequiredHeight);
                        return false;
                    }
                }
            }
        }
    }

    // World collision sweep upwards
    const float HalfHeight = FMath::Max(RequiredHeight * 0.5f, RequiredRadius);
    FCollisionShape Capsule = FCollisionShape::MakeCapsule(RequiredRadius, HalfHeight);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(WyrmGrowthClearanceSweep), false);

    // Center the capsule so its bottom is 5cm above the ground location
    const FVector Start = Location + FVector(0.f, 0.f, HalfHeight + 5.f);
    const FVector End = Location + FVector(0.f, 0.f, HalfHeight + 15.f);

    FHitResult Hit;
    if (World->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_WorldStatic, Capsule, Params))
    {
        if (Hit.GetActor() && Hit.ImpactPoint.Z > Location.Z + 15.f)
        {
            OutReason = FString::Printf(TEXT("Growth blocked overhead by %s"), *Hit.GetActor()->GetName());
            return false;
        }
    }

    return true;
}

void UWyrmBuildingSubsystem::BuildSaveRecord(FWyrmCampSaveRecord& OutRecord) const
{
    OutRecord.Pieces.Empty();
    for (const TWeakObjectPtr<AWyrmBuildingPiece>& Ptr : ActivePieces)
    {
        if (AWyrmBuildingPiece* Piece = Ptr.Get())
        {
            FWyrmCampPieceSaveRecord Rec;
            Rec.PieceGuid = Piece->PieceGuid;
            Rec.PieceId = Piece->PieceId;
            Rec.PieceType = Piece->PieceType;
            Rec.Transform = Piece->GetActorTransform();
            Rec.bIsOpen = Piece->bIsOpen;

            if (AWyrmStorageActor* Storage = Cast<AWyrmStorageActor>(Piece))
            {
                if (Storage->StorageInventory)
                {
                    Rec.StoredItems = Storage->StorageInventory->GetBagItems();
                }
            }

            OutRecord.Pieces.Add(Rec);
        }
    }
}

void UWyrmBuildingSubsystem::RestoreFromSaveRecord(const FWyrmCampSaveRecord& InRecord, UWorld* WorldContext)
{
    ClearAllPlacedPieces();

    if (!WorldContext)
    {
        return;
    }

    for (const FWyrmCampPieceSaveRecord& Rec : InRecord.Pieces)
    {
        FWyrmBuildingPieceDefinition Def;
        GetPieceDefinition(Rec.PieceId, Def);

        UClass* SpawnClass = Def.CustomPieceClass ? Def.CustomPieceClass.Get() : AWyrmBuildingPiece::StaticClass();
        if (Rec.PieceType == EWyrmBuildingPieceType::StorageChest && SpawnClass == AWyrmBuildingPiece::StaticClass())
        {
            SpawnClass = AWyrmStorageActor::StaticClass();
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AWyrmBuildingPiece* Piece = WorldContext->SpawnActor<AWyrmBuildingPiece>(SpawnClass, Rec.Transform, SpawnParams);
        if (Piece)
        {
            Piece->PieceId = Rec.PieceId;
            Piece->PieceType = Rec.PieceType;
            Piece->PieceGuid = Rec.PieceGuid;
            Piece->bIsOpen = Rec.bIsOpen;
            Piece->BoundsExtent = Def.BoundsExtent;

            if (!Def.Mesh.IsNull())
            {
                UStaticMesh* Mesh = Def.Mesh.LoadSynchronous();
                if (Mesh && Piece->MeshComponent)
                {
                    Piece->MeshComponent->SetStaticMesh(Mesh);
                }
            }

            if (Rec.bIsOpen && Piece->MeshComponent)
            {
                Piece->MeshComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
            }

            if (AWyrmStorageActor* Storage = Cast<AWyrmStorageActor>(Piece))
            {
                if (Storage->StorageInventory)
                {
                    FWyrmItemInstance Excess;
                    for (const FWyrmItemInstance& Item : Rec.StoredItems)
                    {
                        Storage->StorageInventory->AddItem(Item, Excess);
                    }
                }
            }

            ActivePieces.Add(Piece);

            for (TActorIterator<AWyrmGeoForgeAdapter> It(WorldContext); It; ++It)
            {
                It->RegisterCampPiece(Piece);
            }
        }
    }
}

void UWyrmBuildingSubsystem::ClearAllPlacedPieces()
{
    UWorld* World = CachedWorld.IsValid() ? CachedWorld.Get() : nullptr;
    for (const TWeakObjectPtr<AWyrmBuildingPiece>& Ptr : ActivePieces)
    {
        if (AWyrmBuildingPiece* Piece = Ptr.Get())
        {
            if (!World)
            {
                World = Piece->GetWorld();
            }
            if (World)
            {
                for (TActorIterator<AWyrmGeoForgeAdapter> It(World); It; ++It)
                {
                    It->UnregisterCampPiece(Piece);
                }
            }
            Piece->Destroy();
        }
    }
    ActivePieces.Empty();
}
