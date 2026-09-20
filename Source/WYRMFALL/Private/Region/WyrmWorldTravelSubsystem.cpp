#include "Region/WyrmWorldTravelSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Save/WyrmSaveSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Dragon/WyrmDragonCharacter.h"
#include "Terrain/WyrmGeoForgeAdapter.h"
#include "EngineUtils.h"

namespace
{
    const FName Region01Id(TEXT("Region01"));
    const FName JadePeaksId(TEXT("JadePeaks"));
    const FName GloamingMarchesId(TEXT("GloamingMarches"));
}

void UWyrmWorldTravelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    State = FWyrmWorldTravelSaveRecord();
    State.CurrentRegionId = RegionIdForMapName(GetWorld() ? GetWorld()->GetMapName() : FString());
}

UWyrmWorldTravelSubsystem* UWyrmWorldTravelSubsystem::GetWorldTravelSubsystem(UObject* WorldContextObject)
{
    if (!WorldContextObject || !GEngine) return nullptr;
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
    {
        if (UGameInstance* GI = World->GetGameInstance()) return GI->GetSubsystem<UWyrmWorldTravelSubsystem>();
    }
    return nullptr;
}

FName UWyrmWorldTravelSubsystem::RegionIdForMapName(const FString& MapName)
{
    if (MapName.Contains(TEXT("L_JadePeaks"))) return JadePeaksId;
    if (MapName.Contains(TEXT("L_GloamingMarches"))) return GloamingMarchesId;
    if (MapName.Contains(TEXT("L_Region01"))) return Region01Id;
    return NAME_None;
}

bool UWyrmWorldTravelSubsystem::IsAllowedRoute(FName FromRegionId, FName ToRegionId) const
{
    return (FromRegionId == Region01Id && ToRegionId == JadePeaksId) ||
           (FromRegionId == JadePeaksId && ToRegionId == Region01Id) ||
           (FromRegionId == Region01Id && ToRegionId == GloamingMarchesId) ||
           (FromRegionId == GloamingMarchesId && ToRegionId == Region01Id);
}

FName UWyrmWorldTravelSubsystem::DestinationMapForRegion(FName RegionId)
{
    if (RegionId == Region01Id) return FName(TEXT("/Game/WYRMFALL/World/Regions/L_Region01"));
    if (RegionId == JadePeaksId) return FName(TEXT("/Game/WYRMFALL/World/Regions/L_JadePeaks"));
    if (RegionId == GloamingMarchesId) return FName(TEXT("/Game/WYRMFALL/World/Regions/L_GloamingMarches"));
    return NAME_None;
}

FName UWyrmWorldTravelSubsystem::DefaultArrivalForRegion(FName RegionId)
{
    return RegionId == JadePeaksId ? FName(TEXT("LM-JADE-ARRIVAL")) :
           RegionId == GloamingMarchesId ? FName(TEXT("LM-GLOAMING-ARRIVAL")) :
           RegionId == Region01Id ? FName(TEXT("LM-ARRIVAL")) : NAME_None;
}

FName UWyrmWorldTravelSubsystem::ExpectedReturnLandmark(FName FromRegionId, FName ToRegionId)
{
    if (FromRegionId == JadePeaksId && ToRegionId == Region01Id) return FName(TEXT("LM-JADE-RETURN"));
    if (FromRegionId == GloamingMarchesId && ToRegionId == Region01Id) return FName(TEXT("LM-GLOAMING-RETURN"));
    if (FromRegionId == Region01Id && (ToRegionId == JadePeaksId || ToRegionId == GloamingMarchesId))
        return FName(TEXT("LM-ARRIVAL"));
    return NAME_None;
}

bool UWyrmWorldTravelSubsystem::PrepareTravel(FName FromRegionId, FName ToRegionId, FName ReturnLandmarkId)
{
    if (!IsAllowedRoute(FromRegionId, ToRegionId) ||
        ReturnLandmarkId != ExpectedReturnLandmark(FromRegionId, ToRegionId)) return false;
    State.ReturnRegionId = FromRegionId;
    State.ReturnLandmarkId = ReturnLandmarkId;
    State.ArrivalLandmarkId = DefaultArrivalForRegion(ToRegionId);
    PreparedDestinationRegion = ToRegionId;
    return !State.ArrivalLandmarkId.IsNone();
}

bool UWyrmWorldTravelSubsystem::PrepareTravelWithSnapshot(FName FromRegionId, FName ToRegionId,
    FName ReturnLandmarkId, const FString& RecoverySlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor)
{
    LastTravelFailureReason.Reset();
    AWyrmGeoForgeAdapter* Adapter = Cast<AWyrmGeoForgeAdapter>(TerrainProviderActor);
    if (!Character || !Adapter || RecoverySlotName.IsEmpty())
    {
        LastTravelFailureReason = TEXT("MissingRecoveryContext");
        return false;
    }
    if (!IsAllowedRoute(FromRegionId, ToRegionId) ||
        ReturnLandmarkId != ExpectedReturnLandmark(FromRegionId, ToRegionId) ||
        DefaultArrivalForRegion(ToRegionId).IsNone())
    {
        LastTravelFailureReason = TEXT("RouteNotAllowlisted");
        return false;
    }
    if (!State.CurrentRegionId.IsNone() && State.CurrentRegionId != FromRegionId)
    {
        LastTravelFailureReason = TEXT("SourceRegionMismatch");
        return false;
    }
    if (Character->IsMoonboundActive() || Character->IsMoonboundReturnPending() ||
        Character->IsSanguineStrikePrimed() || Character->IsSecondTurnPrimed() ||
        Character->HasPendingSecondTurnRepeat())
    {
        LastTravelFailureReason = TEXT("UnsafeCharacterTransient");
        return false;
    }
    if (Adapter->HasPendingTerrainEdits())
    {
        LastTravelFailureReason = TEXT("TerrainMutationPending");
        return false;
    }
    if (UWorld* World = Character->GetWorld())
    {
        for (TActorIterator<AWyrmDragonCharacter> It(World); It; ++It)
        {
            if ((*It)->IsTransitioningForm() || (*It)->IsDirectlyControlled() ||
                ((*It)->IsRiderMounted() && (*It)->IsInFlight()))
            {
                LastTravelFailureReason = TEXT("UnsafeDragonTransient");
                return false;
            }
        }
    }
    if (!UWyrmSaveSubsystem::SaveSnapshotToSlot(
        RecoverySlotName, Character, TerrainProviderActor, Character->GetWorld()))
    {
        LastTravelFailureReason = TEXT("RecoverySnapshotFailed");
        return false;
    }
    if (!PrepareTravel(FromRegionId, ToRegionId, ReturnLandmarkId))
    {
        LastTravelFailureReason = TEXT("TravelPreparationFailed");
        return false;
    }
    return true;
}

bool UWyrmWorldTravelSubsystem::OpenPreparedDestination()
{
    const FName Map = DestinationMapForRegion(PreparedDestinationRegion);
    UWorld* World = GetWorld();
    if (!World || Map.IsNone()) return false;
    UGameplayStatics::OpenLevel(World, Map);
    return true;
}

bool UWyrmWorldTravelSubsystem::CompleteArrival(FName RegionId, FName ArrivalLandmarkId)
{
    if (RegionId.IsNone() || ArrivalLandmarkId != DefaultArrivalForRegion(RegionId)) return false;
    if (!PreparedDestinationRegion.IsNone() && PreparedDestinationRegion != RegionId) return false;
    State.CurrentRegionId = RegionId;
    State.ArrivalLandmarkId = ArrivalLandmarkId;
    PreparedDestinationRegion = NAME_None;
    return true;
}

bool UWyrmWorldTravelSubsystem::BeginTravelRecovery(const FString& RecoverySlotName)
{
    LastTravelFailureReason.Reset();
    if (RecoverySlotName.IsEmpty() || !UWyrmSaveSubsystem::DoesSaveExist(RecoverySlotName))
    {
        LastTravelFailureReason = TEXT("RecoverySnapshotMissing");
        return false;
    }
    UWyrmSaveGame* Snapshot = Cast<UWyrmSaveGame>(UGameplayStatics::LoadGameFromSlot(RecoverySlotName, 0));
    if (!Snapshot || !UWyrmSaveSubsystem::IsSchemaVersionSupported(Snapshot->SchemaVersion) ||
        DestinationMapForRegion(Snapshot->WorldTravelRecord.CurrentRegionId).IsNone())
    {
        LastTravelFailureReason = TEXT("RecoverySnapshotInvalid");
        return false;
    }
    PendingRecoverySlotName = RecoverySlotName;
    PendingRecoveryRegion = Snapshot->WorldTravelRecord.CurrentRegionId;
    PreparedDestinationRegion = NAME_None;

    const FName ActiveRegion = RegionIdForMapName(GetWorld() ? GetWorld()->GetMapName() : FString());
    if (ActiveRegion != PendingRecoveryRegion)
    {
        UGameplayStatics::OpenLevel(GetWorld(), DestinationMapForRegion(PendingRecoveryRegion));
    }
    return true;
}

bool UWyrmWorldTravelSubsystem::CompleteTravelRecovery(AWyrmCharacter* Character, AActor* TerrainProviderActor)
{
    if (!Character || !Character->GetWorld() || !Cast<AWyrmGeoForgeAdapter>(TerrainProviderActor) ||
        PendingRecoverySlotName.IsEmpty() || PendingRecoveryRegion.IsNone() ||
        RegionIdForMapName(Character->GetWorld()->GetMapName()) != PendingRecoveryRegion)
    {
        LastTravelFailureReason = TEXT("RecoverySourceNotReady");
        return false;
    }
    const bool bRestored = UWyrmSaveSubsystem::LoadSnapshotFromSlot(
        PendingRecoverySlotName, Character, TerrainProviderActor, Character->GetWorld());
    if (!bRestored)
    {
        LastTravelFailureReason = TEXT("RecoveryApplyFailed");
        return false;
    }
    PendingRecoverySlotName.Reset();
    PendingRecoveryRegion = NAME_None;
    LastTravelFailureReason.Reset();
    return true;
}

void UWyrmWorldTravelSubsystem::RestoreFromSaveRecord(const FWyrmWorldTravelSaveRecord& InRecord)
{
    State = InRecord;
    if (DestinationMapForRegion(State.CurrentRegionId).IsNone()) State.CurrentRegionId = NAME_None;
    if (!State.ReturnRegionId.IsNone() && DestinationMapForRegion(State.ReturnRegionId).IsNone())
    {
        State.ReturnRegionId = NAME_None;
        State.ReturnLandmarkId = NAME_None;
    }
    PreparedDestinationRegion = NAME_None;
    LastTravelFailureReason.Reset();
}
