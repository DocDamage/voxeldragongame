#include "Region/WyrmRegion01Landmark.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Region/WyrmRegion01Subsystem.h"
#include "Player/WyrmCharacter.h"

AWyrmRegion01Landmark::AWyrmRegion01Landmark()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetupAttachment(SceneRoot);
    TriggerVolume->SetSphereRadius(TriggerRadius);
    TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    TriggerVolume->SetGenerateOverlapEvents(true);
}

void AWyrmRegion01Landmark::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (TriggerVolume)
    {
        TriggerVolume->SetSphereRadius(TriggerRadius);
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AWyrmRegion01Landmark::HandleOverlap);
    }
}

void AWyrmRegion01Landmark::HandleOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    TriggerVisit(OtherActor);
}

bool AWyrmRegion01Landmark::TriggerVisit(AActor* VisitingActor)
{
    if (!VisitingActor || !VisitingActor->IsA<AWyrmCharacter>())
    {
        return false;
    }

    UWyrmRegion01Subsystem* Region01 = UWyrmRegion01Subsystem::GetRegion01Subsystem(this);
    if (!Region01 || LandmarkId.IsNone())
    {
        return false;
    }

    bool bSuccess = Region01->VisitLandmark(LandmarkId);

    // Reaching daylight at Tamsin's site from the heart chamber commits the exit fact.
    if (LandmarkId == FName(TEXT("LM-TAMSIN")) && !Region01->HasFact(FName(TEXT("heart.exit_reached"))))
    {
        Region01->RecordHeartExitReached();
    }

    return bSuccess;
}

bool AWyrmRegion01Landmark::IsLandmarkAvailable() const
{
    UWyrmRegion01Subsystem* Region01 = UWyrmRegion01Subsystem::GetRegion01Subsystem(GetWorld());
    return Region01 ? Region01->IsLandmarkCurrentlyAvailable(LandmarkId) : false;
}

bool AWyrmRegion01Landmark::HasBeenVisited() const
{
    UWyrmRegion01Subsystem* Region01 = UWyrmRegion01Subsystem::GetRegion01Subsystem(GetWorld());
    return Region01 ? Region01->HasVisitedLandmark(LandmarkId) : false;
}
