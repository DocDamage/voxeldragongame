#include "Terrain/WyrmGeoForgeAdapter.h"
#include "GeoForgeInfiniteTerrainActor.h"
#include "GeoForgeTerrainTypes.h"
#include "GeoForgeTerrainBlueprintLibrary.h"
#include "GeoForgeTerrainSaveGame.h"
#include "Terrain/WyrmTerrainDiagnostics.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "Water/WyrmWaterVolume.h"
#include "Building/WyrmBuildingPiece.h"

namespace
{
struct FWyrmGeoForgeCellKey
{
    FIntVector ChunkCoord = FIntVector::ZeroValue;
    FIntVector LocalCell = FIntVector::ZeroValue;

    bool operator==(const FWyrmGeoForgeCellKey& Other) const
    {
        return ChunkCoord == Other.ChunkCoord && LocalCell == Other.LocalCell;
    }

    friend uint32 GetTypeHash(const FWyrmGeoForgeCellKey& Key)
    {
        return HashCombine(GetTypeHash(Key.ChunkCoord), GetTypeHash(Key.LocalCell));
    }
};
}

AWyrmGeoForgeAdapter::AWyrmGeoForgeAdapter()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
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

    for (TActorIterator<AWyrmWaterVolume> It(GetWorld()); It; ++It)
    {
        RegisterWaterVolume(*It);
    }
}

void AWyrmGeoForgeAdapter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (PendingCompletionRequests.IsEmpty())
    {
        SetActorTickEnabled(false);
        return;
    }

    PendingCompletionAgeSeconds += FMath::Max(0.f, DeltaSeconds);
    if (PendingCompletionAgeSeconds >= CompletionTimeoutSeconds)
    {
        FailPendingTerrainEdits();
        return;
    }

    TryFinalizePendingTerrainEdits();
}

void AWyrmGeoForgeAdapter::BindTerrainActor(AGeoForgeInfiniteTerrainActor* InActor)
{
    TerrainActor = InActor;
}

void AWyrmGeoForgeAdapter::RegisterWaterVolume(AWyrmWaterVolume* Volume)
{
    if (Volume)
    {
        RegisteredWaterVolumes.AddUnique(Volume);
    }
}

void AWyrmGeoForgeAdapter::UnregisterWaterVolume(AWyrmWaterVolume* Volume)
{
    if (Volume)
    {
        RegisteredWaterVolumes.Remove(Volume);
    }
}

void AWyrmGeoForgeAdapter::RegisterCampPiece(AWyrmBuildingPiece* Piece)
{
    if (Piece)
    {
        RegisteredCampPieces.AddUnique(Piece);
    }
}

void AWyrmGeoForgeAdapter::UnregisterCampPiece(AWyrmBuildingPiece* Piece)
{
    if (Piece)
    {
        RegisteredCampPieces.Remove(Piece);
    }
    RegisteredCampPieces.RemoveAll([](const TWeakObjectPtr<AWyrmBuildingPiece>& P) {
        return !P.IsValid() || !IsValid(P.Get());
    });
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
    LastRejectionReason.Empty();

    if (!Request.IsWellFormed())
    {
        LastRejectionReason = TEXT("MalformedRequest");
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

    int32 AppliedEditCount = 0;
    float CellVolumeCm3 = 0.f;

    if (Request.Operation == EWyrmTerrainEditOperation::Add)
    {
        // Occupied fill protection: do not bury or clip characters
        if (IsVolumeOccupied_Implementation(Request.WorldCenter, Request.RadiusCm))
        {
            return EWyrmTerrainSubmitResult::Rejected;
        }

        if (!TerrainActor.IsValid())
        {
            return EWyrmTerrainSubmitResult::Unsupported;
        }

        FGeoForgeBlockSpecification BlockSpecification;
        AppliedEditCount = TerrainActor->AddSphereWithBlock(
            Request.WorldCenter, Request.RadiusCm, BlockSpecification);
    }
    else if (Request.Operation == EWyrmTerrainEditOperation::Remove)
    {
        // Water basin boundary protection (WRLD-11):
        for (const TWeakObjectPtr<AWyrmWaterVolume>& WaterVolPtr : RegisteredWaterVolumes)
        {
            if (AWyrmWaterVolume* WaterVol = WaterVolPtr.Get())
            {
                FString OutRejectionReason;
                if (!WaterVol->ValidateTerrainEdit(Request, OutRejectionReason))
                {
                    LastRejectionReason = OutRejectionReason;
                    UE_LOG(LogTemp, Warning, TEXT("[WyrmTerrain] Edit %s rejected by water volume: %s"),
                        *Request.ActionId.ToString(), *OutRejectionReason);
                    return EWyrmTerrainSubmitResult::Rejected;
                }
            }
        }

        // Camp ground support protection (WRLD-09):
        for (auto It = RegisteredCampPieces.CreateIterator(); It; ++It)
        {
            if (!It->IsValid() || !IsValid(It->Get()))
            {
                It.RemoveCurrent();
                continue;
            }

            AWyrmBuildingPiece* Piece = It->Get();
            const FBox SupportBox = Piece->GetSupportBounds();
            if (!SupportBox.IsValid)
            {
                continue;
            }

            const FBox RequestBox(
                Request.WorldCenter - FVector(Request.RadiusCm),
                Request.WorldCenter + FVector(Request.RadiusCm));

            if (SupportBox.Intersect(RequestBox))
            {
                LastRejectionReason = TEXT("RejectionReason_CampSupport");
                UE_LOG(LogTemp, Warning, TEXT("[WyrmTerrain] Edit %s rejected: RejectionReason_CampSupport beneath piece %s"),
                    *Request.ActionId.ToString(), *Piece->PieceId.ToString());
                return EWyrmTerrainSubmitResult::Rejected;
            }
        }

        if (!TerrainActor.IsValid())
        {
            return EWyrmTerrainSubmitResult::Unsupported;
        }

        const int32 FilledBefore = CountFilledCellsInSphere(
            Request.WorldCenter, Request.RadiusCm, CellVolumeCm3);
        if (FilledBefore == INDEX_NONE)
        {
            return EWyrmTerrainSubmitResult::Rejected;
        }

        TerrainActor->DigSphere(Request.WorldCenter, Request.RadiusCm);

        float IgnoredCellVolume = 0.f;
        const int32 FilledAfter = CountFilledCellsInSphere(
            Request.WorldCenter, Request.RadiusCm, IgnoredCellVolume);
        if (FilledAfter == INDEX_NONE)
        {
            return EWyrmTerrainSubmitResult::Rejected;
        }
        AppliedEditCount = FMath::Max(0, FilledBefore - FilledAfter);

        FWyrmVoxelYield Yield;
        Yield.ActionId = Request.ActionId;
        Yield.ResourceId = FName(TEXT("Resource.Dirt"));
        Yield.ExtractedCount = AppliedEditCount;
        Yield.VolumeExtractedCm3 = AppliedEditCount * CellVolumeCm3;
        Yield.bDuplicatePrevented = false;
        ActionYields.Add(Request.ActionId, Yield);
    }

    ProcessedActionIds.Add(Request.ActionId);

    // A no-op edit is a completed transaction with zero yield and no geometry or
    // navigation work to wait for.
    if (AppliedEditCount == 0)
    {
        OnTerrainEditCompleted.Broadcast(Request, true);
        OnTerrainCollisionReady.Broadcast(Request.ActionId);
        OnTerrainNavReady.Broadcast(Request.ActionId);
        return EWyrmTerrainSubmitResult::Completed;
    }

    // Advance the configured synchronous budget, then inspect GeoForge's real
    // queues. Any remaining work is completed from Tick instead of being falsely
    // reported as ready.
    TerrainActor->RefreshLoadedChunkVisuals();
    PendingCompletionRequests.Add(Request);
    bNavigationRefreshSubmitted = false;
    PendingCompletionAgeSeconds = 0.f;
    SetActorTickEnabled(true);

    return TryFinalizePendingTerrainEdits()
        ? EWyrmTerrainSubmitResult::Completed
        : EWyrmTerrainSubmitResult::Queued;
}

bool AWyrmGeoForgeAdapter::IsTerrainGeometryReady() const
{
    if (!TerrainActor.IsValid())
    {
        return false;
    }

    const FGeoForgeTerrainRuntimeRenderStats Stats = TerrainActor->GetRuntimeRenderStats();
    const bool bQueuesIdle = Stats.QueuedChunkGenerationCount == 0
        && Stats.QueuedChunkRebuildCount == 0
        && Stats.ChunkGenerationJobsInFlight == 0
        && Stats.ChunkMeshJobsInFlight == 0
        && Stats.PendingChunkApplyCount == 0;

    if (Stats.WorldShape == EGeoForgeWorldShape::CubeSpherePlanet)
    {
        return bQueuesIdle && Stats.bPlanetReadyForGameplay
            && !Stats.bPlanetMeshBuildInFlight
            && !Stats.bPlanetMeshApplyInProgress
            && !Stats.bPlanetApplyQueued;
    }
    return bQueuesIdle;
}

bool AWyrmGeoForgeAdapter::TryFinalizePendingTerrainEdits()
{
    if (PendingCompletionRequests.IsEmpty())
    {
        SetActorTickEnabled(false);
        return true;
    }

    if (!IsTerrainGeometryReady())
    {
        return false;
    }

    if (!bNavigationRefreshSubmitted)
    {
        UWyrmTerrainDiagnostics::RefreshNavigationDataForActor(TerrainActor.Get());
        bNavigationRefreshSubmitted = true;
    }

    if (UWyrmTerrainDiagnostics::IsNavigationBuildPending(TerrainActor.Get()))
    {
        return false;
    }

    TArray<FWyrmTerrainEditRequest> CompletedRequests = MoveTemp(PendingCompletionRequests);
    PendingCompletionRequests.Reset();
    bNavigationRefreshSubmitted = false;
    PendingCompletionAgeSeconds = 0.f;
    SetActorTickEnabled(false);

    for (const FWyrmTerrainEditRequest& CompletedRequest : CompletedRequests)
    {
        OnTerrainEditCompleted.Broadcast(CompletedRequest, true);
        OnTerrainCollisionReady.Broadcast(CompletedRequest.ActionId);
        OnTerrainNavReady.Broadcast(CompletedRequest.ActionId);
    }
    return true;
}

int32 AWyrmGeoForgeAdapter::CountFilledCellsInSphere(
    const FVector& Center, float RadiusCm, float& OutCellVolumeCm3) const
{
    OutCellVolumeCm3 = 0.f;
    if (!TerrainActor.IsValid() || !FMath::IsFinite(RadiusCm) || RadiusCm <= 0.f)
    {
        return INDEX_NONE;
    }

    const float CellSize = TerrainActor->GetResolvedTerrainCellSizeInWorldUnits();
    if (!FMath::IsFinite(CellSize) || CellSize <= KINDA_SMALL_NUMBER)
    {
        return INDEX_NONE;
    }

    OutCellVolumeCm3 = CellSize * CellSize * CellSize;
    const float SampleStep = FMath::Max(1.f, CellSize * 0.5f);
    const int32 HalfSteps = FMath::CeilToInt(RadiusCm / SampleStep) + 2;
    if (HalfSteps > 64)
    {
        return INDEX_NONE;
    }

    TSet<FWyrmGeoForgeCellKey> VisitedCells;
    int32 FilledCount = 0;
    const float QueryRadius = RadiusCm + CellSize;
    for (int32 X = -HalfSteps; X <= HalfSteps; ++X)
    {
        for (int32 Y = -HalfSteps; Y <= HalfSteps; ++Y)
        {
            for (int32 Z = -HalfSteps; Z <= HalfSteps; ++Z)
            {
                const FVector SampleOffset(X * SampleStep, Y * SampleStep, Z * SampleStep);
                if (SampleOffset.SizeSquared() > FMath::Square(QueryRadius))
                {
                    continue;
                }

                FGeoForgeTerrainCellHitInfo CellInfo;
                if (!TerrainActor->QueryExactTerrainCellAtWorldLocation(
                        Center + SampleOffset, CellInfo, true)
                    || !CellInfo.bIsValid
                    || FVector::DistSquared(CellInfo.CellCenterWorld, Center) > FMath::Square(RadiusCm + KINDA_SMALL_NUMBER))
                {
                    continue;
                }

                const FWyrmGeoForgeCellKey CellKey{CellInfo.ChunkCoord, CellInfo.LocalCell};
                if (!VisitedCells.Contains(CellKey))
                {
                    VisitedCells.Add(CellKey);
                    if (CellInfo.bFilled)
                    {
                        ++FilledCount;
                    }
                }
            }
        }
    }
    return FilledCount;
}

void AWyrmGeoForgeAdapter::FailPendingTerrainEdits()
{
    TArray<FWyrmTerrainEditRequest> FailedRequests = MoveTemp(PendingCompletionRequests);
    PendingCompletionRequests.Reset();
    bNavigationRefreshSubmitted = false;
    PendingCompletionAgeSeconds = 0.f;
    SetActorTickEnabled(false);

    for (const FWyrmTerrainEditRequest& FailedRequest : FailedRequests)
    {
        OnTerrainEditCompleted.Broadcast(FailedRequest, false);
    }
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
    OutBytes.Reset();
    if (!TerrainActor.IsValid() || HasPendingTerrainEdits())
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
    if (!TerrainActor.IsValid() || InBytes.Num() == 0 || HasPendingTerrainEdits())
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
