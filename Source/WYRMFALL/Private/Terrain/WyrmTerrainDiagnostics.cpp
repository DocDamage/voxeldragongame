#include "Terrain/WyrmTerrainDiagnostics.h"
#include "AI/NavigationSystemBase.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

int32 UWyrmTerrainDiagnostics::RefreshNavigationDataForActor(AActor* TerrainActor)
{
    if (!IsValid(TerrainActor)) { return 0; }
    TInlineComponentArray<UActorComponent*> Components(TerrainActor);
    int32 Submitted = 0;
    for (UActorComponent* Component : Components)
    {
        if (IsValid(Component) && Component->IsRegistered() && Component->CanEverAffectNavigation())
        {
            FNavigationSystem::UpdateComponentData(*Component);
            ++Submitted;
        }
    }
    return Submitted;
}

bool UWyrmTerrainDiagnostics::ProjectNavigationPoint(AActor* WorldActor, FVector Point, FVector QueryExtent, FVector& Projected)
{
    Projected = FVector::ZeroVector;
    return IsValid(WorldActor) && UNavigationSystemV1::K2_ProjectPointToNavigation(
        WorldActor, Point, Projected, nullptr, nullptr, QueryExtent);
}

int32 UWyrmTerrainDiagnostics::FindCompleteNavigationPathPointCount(
    AActor* WorldActor, FVector Start, FVector End)
{
    if (!IsValid(WorldActor)) { return 0; }
    UNavigationSystemV1* Navigation = UNavigationSystemV1::GetCurrent(WorldActor->GetWorld());
    if (!Navigation) { return 0; }
    UNavigationPath* Path = Navigation->FindPathToLocationSynchronously(
        WorldActor->GetWorld(), Start, End, WorldActor);
    return Path && Path->IsValid() && !Path->IsPartial() ? Path->PathPoints.Num() : 0;
}

bool UWyrmTerrainDiagnostics::IsNavigationBuildPending(AActor* WorldActor)
{
    // Missing navigation is not a completed build.
    return !IsValid(WorldActor) || !UNavigationSystemV1::GetNavigationSystem(WorldActor)
        || UNavigationSystemV1::IsNavigationBeingBuiltOrLocked(WorldActor);
}
