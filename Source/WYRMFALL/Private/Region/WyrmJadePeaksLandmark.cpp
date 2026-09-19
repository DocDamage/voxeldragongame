#include "Region/WyrmJadePeaksLandmark.h"
#include "Region/WyrmJadePeaksSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"

AWyrmJadePeaksLandmark::AWyrmJadePeaksLandmark()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
    TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetupAttachment(SceneRoot);
    TriggerVolume->SetSphereRadius(400.f);
    TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    TriggerVolume->SetGenerateOverlapEvents(true);
}

void AWyrmJadePeaksLandmark::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AWyrmJadePeaksLandmark::HandleOverlap);
}

void AWyrmJadePeaksLandmark::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    TriggerVisit(OtherActor);
}

bool AWyrmJadePeaksLandmark::TriggerVisit(AActor* VisitingActor)
{
    if (!VisitingActor || !VisitingActor->IsA<AWyrmCharacter>() || LandmarkId.IsNone())
    {
        return false;
    }
    if (UWyrmJadePeaksSubsystem* Region = UWyrmJadePeaksSubsystem::GetJadePeaksSubsystem(this))
    {
        return Region->VisitLandmark(LandmarkId);
    }
    return false;
}
