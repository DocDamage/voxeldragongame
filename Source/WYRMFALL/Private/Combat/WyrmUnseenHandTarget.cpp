#include "Combat/WyrmUnseenHandTarget.h"
#include "Components/StaticMeshComponent.h"

AWyrmUnseenHandTarget::AWyrmUnseenHandTarget()
{
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MovableProp"));
    SetRootComponent(MeshComponent);
    MeshComponent->SetMobility(EComponentMobility::Movable);
    MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetEnableGravity(false);
}

bool AWyrmUnseenHandTarget::ApplyUnseenHandImpulse(const FVector& Impulse)
{
    if (!IsEligibleForUnseenHand() || !MeshComponent || !MeshComponent->IsSimulatingPhysics()) return false;
    MeshComponent->AddImpulse(Impulse, NAME_None, true);
    return true;
}
