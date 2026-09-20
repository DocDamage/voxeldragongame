#include "Region/WyrmCogspireArrivalTrigger.h"
#include "Region/WyrmCogspireSubsystem.h"
#include "Region/WyrmWorldTravelSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"

AWyrmCogspireArrivalTrigger::AWyrmCogspireArrivalTrigger()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
    TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetupAttachment(SceneRoot);
    TriggerVolume->SetSphereRadius(450.f);
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerVolume->SetGenerateOverlapEvents(true);
    TriggerVolume->SetHiddenInGame(true);
}

void AWyrmCogspireArrivalTrigger::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AWyrmCogspireArrivalTrigger::HandleOverlap);
}

void AWyrmCogspireArrivalTrigger::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    TriggerArrival(OtherActor);
}

bool AWyrmCogspireArrivalTrigger::TriggerArrival(AActor* VisitingActor)
{
    if (!VisitingActor || !VisitingActor->IsA<AWyrmCharacter>())
    {
        return false;
    }
    const UWyrmWorldTravelSubsystem* Travel =
        UWyrmWorldTravelSubsystem::GetWorldTravelSubsystem(this);
    if (!Travel || Travel->GetCurrentRegionId() != FName(TEXT("CogspireHarbor")) ||
        Travel->GetArrivalLandmarkId() != FName(TEXT("LM-COGSPIRE-ARRIVAL")))
    {
        return false;
    }
    if (UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this))
    {
        return Region->RecordArrival();
    }
    return false;
}
