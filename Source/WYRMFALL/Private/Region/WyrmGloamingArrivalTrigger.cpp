#include "Region/WyrmGloamingArrivalTrigger.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"

AWyrmGloamingArrivalTrigger::AWyrmGloamingArrivalTrigger()
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

void AWyrmGloamingArrivalTrigger::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AWyrmGloamingArrivalTrigger::HandleOverlap);
}

void AWyrmGloamingArrivalTrigger::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    TriggerArrival(OtherActor);
}

bool AWyrmGloamingArrivalTrigger::TriggerArrival(AActor* VisitingActor)
{
    if (!VisitingActor || !VisitingActor->IsA<AWyrmCharacter>())
    {
        return false;
    }
    if (UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this))
    {
        return Region->RecordArrival();
    }
    return false;
}
