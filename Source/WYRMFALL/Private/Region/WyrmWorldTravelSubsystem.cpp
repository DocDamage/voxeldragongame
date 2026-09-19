#include "Region/WyrmWorldTravelSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    const FName Region01Id(TEXT("Region01"));
    const FName JadePeaksId(TEXT("JadePeaks"));
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
    if (MapName.Contains(TEXT("L_Region01"))) return Region01Id;
    return NAME_None;
}

bool UWyrmWorldTravelSubsystem::IsAllowedRoute(FName FromRegionId, FName ToRegionId) const
{
    return (FromRegionId == Region01Id && ToRegionId == JadePeaksId) ||
           (FromRegionId == JadePeaksId && ToRegionId == Region01Id);
}

FName UWyrmWorldTravelSubsystem::DestinationMapForRegion(FName RegionId)
{
    if (RegionId == Region01Id) return FName(TEXT("/Game/WYRMFALL/World/Regions/L_Region01"));
    if (RegionId == JadePeaksId) return FName(TEXT("/Game/WYRMFALL/World/Regions/L_JadePeaks"));
    return NAME_None;
}

FName UWyrmWorldTravelSubsystem::DefaultArrivalForRegion(FName RegionId)
{
    return RegionId == JadePeaksId ? FName(TEXT("LM-JADE-ARRIVAL")) :
           RegionId == Region01Id ? FName(TEXT("LM-ARRIVAL")) : NAME_None;
}

bool UWyrmWorldTravelSubsystem::PrepareTravel(FName FromRegionId, FName ToRegionId, FName ReturnLandmarkId)
{
    if (!IsAllowedRoute(FromRegionId, ToRegionId) || ReturnLandmarkId.IsNone()) return false;
    State.ReturnRegionId = FromRegionId;
    State.ReturnLandmarkId = ReturnLandmarkId;
    State.ArrivalLandmarkId = DefaultArrivalForRegion(ToRegionId);
    PreparedDestinationRegion = ToRegionId;
    return !State.ArrivalLandmarkId.IsNone();
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
}
