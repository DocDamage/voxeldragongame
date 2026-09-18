#include "Region/WyrmRegion01Interactable.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Region/WyrmRegion01Subsystem.h"
#include "Player/WyrmCharacter.h"

AWyrmRegion01Interactable::AWyrmRegion01Interactable()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(SceneRoot);
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
}

bool AWyrmRegion01Interactable::CanInteract(const AActor* Interactor) const
{
    if (!Interactor)
    {
        return false;
    }
    if (InteractableType == EWyrmRegion01InteractableType::CentralClaim)
    {
        UWyrmRegion01Subsystem* Region01 = UWyrmRegion01Subsystem::GetRegion01Subsystem(const_cast<AWyrmRegion01Interactable*>(this));
        if (!Region01 || !Region01->HasFact(FName(TEXT("verdance.defeated_alive"))) || Region01->HasFact(FName(TEXT("verdance.claim_broken"))))
        {
            return false;
        }
    }
    if (InteractableType == EWyrmRegion01InteractableType::ServiceCache)
    {
        UWyrmRegion01Subsystem* Region01 = UWyrmRegion01Subsystem::GetRegion01Subsystem(const_cast<AWyrmRegion01Interactable*>(this));
        if (!Region01 || !Region01->IsBondedDragonAvailable() || Region01->IsServiceCacheRecovered())
        {
            return false;
        }
    }

    const float DistSq = FVector::DistSquared(GetActorLocation(), Interactor->GetActorLocation());
    return DistSq <= (InteractionRadius * InteractionRadius);
}

bool AWyrmRegion01Interactable::Interact(AActor* Interactor)
{
    if (!CanInteract(Interactor))
    {
        return false;
    }

    UWyrmRegion01Subsystem* Region01 = UWyrmRegion01Subsystem::GetRegion01Subsystem(this);
    if (!Region01)
    {
        return false;
    }

    bool bSuccess = false;

    switch (InteractableType)
    {
    case EWyrmRegion01InteractableType::CrownNotice:
        bSuccess = Region01->ReadCrownNotice();
        break;

    case EWyrmRegion01InteractableType::AuxiliaryRestraint:
        bSuccess = Region01->DisableAuxiliaryRestraint();
        break;

    case EWyrmRegion01InteractableType::MachineEvidence:
        bSuccess = Region01->DiscoverEvidence(EWyrmRegion01Evidence::Machine);
        break;

    case EWyrmRegion01InteractableType::RecordsEvidence:
        bSuccess = Region01->DiscoverEvidence(EWyrmRegion01Evidence::Records);
        break;

    case EWyrmRegion01InteractableType::WageRecord:
        bSuccess = Region01->RecoverOptionalWageRecord();
        break;

    case EWyrmRegion01InteractableType::CentralClaim:
        if (Region01->HasFact(FName(TEXT("verdance.defeated_alive"))) && !Region01->HasFact(FName(TEXT("verdance.claim_broken"))))
        {
            bSuccess = Region01->BreakVerdanceClaim();
        }
        break;

    case EWyrmRegion01InteractableType::ServiceCache:
        bSuccess = Region01->RecoverServiceCache();
        break;
    }

    if (bSuccess)
    {
        bHasBeenInteracted = true;
    }

    return bSuccess;
}
