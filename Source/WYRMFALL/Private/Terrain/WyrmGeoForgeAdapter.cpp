#include "Terrain/WyrmGeoForgeAdapter.h"
#include "GeoForgeInfiniteTerrainActor.h"
#include "GeoForgeTerrainBlueprintLibrary.h"
#include "GeoForgeTerrainSaveGame.h"
#include "Terrain/WyrmTerrainDiagnostics.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"

AWyrmGeoForgeAdapter::AWyrmGeoForgeAdapter()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AWyrmGeoForgeAdapter::BeginPlay()
{
    Super::BeginPlay();

    if (!TerrainActor.IsValid())
    {
        AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AGeoForgeInfiniteTerrainActor::StaticClass());
        if (FoundActor)
        {
            TerrainActor = Cast<AGeoForgeInfiniteTerrainActor>(FoundActor);
        }
    }
}

void AWyrmGeoForgeAdapter::BindTerrainActor(AGeoForgeInfiniteTerrainActor* InActor)
{
    TerrainActor = InActor;
}

FWyrmTerrainCapabilities AWyrmGeoForgeAdapter::GetTerrainCapabilities_Implementation() const
{
    FWyrmTerrainCapabilities Caps;
    Caps.bSmoothRemove = true;
    Caps.bSmoothAdd = true;
    Caps.bCollisionCompletion = true;
    Caps.bNewSurfaceNavigation = true;
    Caps.bPersistentEdits = true;
    return Caps;
}

bool AWyrmGeoForgeAdapter::IsVolumeOccupied_Implementation(const FVector& Center, float RadiusCm) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FCollisionShape Sphere = FCollisionShape::MakeSphere(RadiusCm);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(WyrmOccupiedFillCheck), false);
    if (TerrainActor.IsValid())
    {
        Params.AddIgnoredActor(TerrainActor.Get());
    }
    Params.AddIgnoredActor(this);

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

    TArray<FOverlapResult> Overlaps;
    const bool bHit = World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity, ObjectParams, Sphere, Params);
    return bHit && Overlaps.Num() > 0;
}

EWyrmTerrainSubmitResult AWyrmGeoForgeAdapter::SubmitTerrainEdit_Implementation(const FWyrmTerrainEditRequest& Request)
{
    if (!Request.IsWellFormed())
    {
        return EWyrmTerrainSubmitResult::Rejected;
    }

    // Duplicate callback/action ID protection
    if (ProcessedActionIds.Contains(Request.ActionId))
    {
        FWyrmVoxelYield PrevYield;
        PrevYield.ActionId = Request.ActionId;
        PrevYield.bDuplicatePrevented = true;
        ActionYields.Add(Request.ActionId, PrevYield);
        return EWyrmTerrainSubmitResult::Rejected;
    }

    if (!TerrainActor.IsValid())
    {
        return EWyrmTerrainSubmitResult::Unsupported;
    }

    if (Request.Operation == EWyrmTerrainEditOperation::Add)
    {
        // Occupied fill protection: do not bury or clip characters
        if (IsVolumeOccupied_Implementation(Request.WorldCenter, Request.RadiusCm))
        {
            return EWyrmTerrainSubmitResult::Rejected;
        }

        TerrainActor->AddSphere(Request.WorldCenter, Request.RadiusCm);
    }
    else if (Request.Operation == EWyrmTerrainEditOperation::Remove)
    {
        const float VolumeCm3 = (4.0f / 3.0f) * PI * FMath::Pow(Request.RadiusCm, 3.0f);
        const int32 Count = FMath::Max(1, FMath::RoundToInt(VolumeCm3 / 500000.0f));

        FWyrmVoxelYield Yield;
        Yield.ActionId = Request.ActionId;
        Yield.ResourceId = FName(TEXT("Resource.Dirt"));
        Yield.ExtractedCount = Count;
        Yield.VolumeExtractedCm3 = VolumeCm3;
        Yield.bDuplicatePrevented = false;
        ActionYields.Add(Request.ActionId, Yield);

        TerrainActor->DigSphere(Request.WorldCenter, Request.RadiusCm);
    }

    // Authoritative synchronous visual refresh
    TerrainActor->RefreshLoadedChunkVisuals();

    // Authoritative navigation submission
    UWyrmTerrainDiagnostics::RefreshNavigationDataForActor(TerrainActor.Get());

    ProcessedActionIds.Add(Request.ActionId);

    OnTerrainEditCompleted.Broadcast(Request, true);
    OnTerrainCollisionReady.Broadcast(Request.ActionId);
    OnTerrainNavReady.Broadcast(Request.ActionId);

    return EWyrmTerrainSubmitResult::Completed;
}

bool AWyrmGeoForgeAdapter::GetLastYield_Implementation(const FGuid& ActionId, FWyrmVoxelYield& OutYield) const
{
    const FWyrmVoxelYield* Found = ActionYields.Find(ActionId);
    if (Found)
    {
        OutYield = *Found;
        return true;
    }
    return false;
}

bool AWyrmGeoForgeAdapter::BuildSavePayload(TArray<uint8>& OutBytes)
{
    if (!TerrainActor.IsValid())
    {
        return false;
    }

    UGeoForgeTerrainSaveGame* SaveObj = Cast<UGeoForgeTerrainSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UGeoForgeTerrainSaveGame::StaticClass()));
    if (!SaveObj)
    {
        return false;
    }

    UGeoForgeTerrainBlueprintLibrary::BuildTerrainSaveData(TerrainActor.Get(), SaveObj->TerrainSaveData);
    return UGameplayStatics::SaveGameToMemory(SaveObj, OutBytes);
}

bool AWyrmGeoForgeAdapter::ApplySavePayload(const TArray<uint8>& InBytes)
{
    if (!TerrainActor.IsValid() || InBytes.Num() == 0)
    {
        return false;
    }

    UGeoForgeTerrainSaveGame* LoadedObj = Cast<UGeoForgeTerrainSaveGame>(
        UGameplayStatics::LoadGameFromMemory(InBytes));
    if (!LoadedObj)
    {
        return false;
    }

    const bool bSuccess = UGeoForgeTerrainBlueprintLibrary::ApplyTerrainSaveData(
        TerrainActor.Get(), LoadedObj->TerrainSaveData);
    if (bSuccess)
    {
        TerrainActor->RefreshLoadedChunkVisuals();
        UWyrmTerrainDiagnostics::RefreshNavigationDataForActor(TerrainActor.Get());
    }
    return bSuccess;
}
