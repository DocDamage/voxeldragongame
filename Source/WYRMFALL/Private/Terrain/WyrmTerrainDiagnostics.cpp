#include "Terrain/WyrmTerrainDiagnostics.h"
#include "AI/NavigationSystemBase.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
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

bool UWyrmTerrainDiagnostics::IsNavigationBuildPending(AActor* WorldActor)
{
    // Missing navigation is not a completed build.
    return !IsValid(WorldActor) || !UNavigationSystemV1::GetNavigationSystem(WorldActor)
        || UNavigationSystemV1::IsNavigationBeingBuiltOrLocked(WorldActor);
}
