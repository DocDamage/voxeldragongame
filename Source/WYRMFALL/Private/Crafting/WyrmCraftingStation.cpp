#include "Crafting/WyrmCraftingStation.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

AWyrmCraftingStation::AWyrmCraftingStation()
{
    PrimaryActorTick.bCanEverTick = false;

    StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
    RootComponent = StationMesh;
    StationMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CampfireMeshFinder(
        TEXT("/Game/WYRMFALL/Environment/Camp/SM_Campfire"));
    if (CampfireMeshFinder.Succeeded())
    {
        StationMesh->SetStaticMesh(CampfireMeshFinder.Object);
    }
}

bool AWyrmCraftingStation::IsActorInRange(const AActor* InActor) const
{
    if (!InActor)
    {
        return false;
    }

    const float Distance = FVector::Dist(GetActorLocation(), InActor->GetActorLocation());
    return Distance <= InteractionRadius;
}
