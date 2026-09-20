#include "Region/WyrmCogspireObservationSite.h"
#include "Region/WyrmCogspireSubsystem.h"
#include "Player/WyrmCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"

AWyrmCogspireObservationSite::AWyrmCogspireObservationSite()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(SceneRoot);
    InteractionVolume->SetBoxExtent(FVector(180.f, 180.f, 140.f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    InteractionVolume->SetGenerateOverlapEvents(true);
    InteractionVolume->SetHiddenInGame(true);
}

bool AWyrmCogspireObservationSite::CanInteract(const AActor* Interactor) const
{
    if (!Interactor || !Interactor->IsA<AWyrmCharacter>() ||
        FVector::DistSquared(GetActorLocation(), Interactor->GetActorLocation()) > FMath::Square(InteractionRadius))
    {
        return false;
    }
    const UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(
        const_cast<AWyrmCogspireObservationSite*>(this));
    if (!Region)
    {
        return false;
    }
    switch (ObservationType)
    {
    case EWyrmCogspireObservationType::PublicMachinery:
        return Region->HasFact(FName(TEXT("cogspire.arrival"))) &&
            !Region->HasFact(FName(TEXT("cogspire.public_machinery_observed")));
    case EWyrmCogspireObservationType::CoercionDiversion:
        return Region->HasFact(FName(TEXT("cogspire.public_machinery_observed"))) &&
            !Region->HasFact(FName(TEXT("cogspire.coercion_diversion_observed")));
    case EWyrmCogspireObservationType::BaronAcknowledgment:
        return Region->HasFact(FName(TEXT("cogspire.coercion_diversion_observed"))) &&
            !Region->HasFact(FName(TEXT("cogspire.baron_acknowledged_diversion")));
    default:
        return false;
    }
}

bool AWyrmCogspireObservationSite::Interact(AActor* Interactor)
{
    if (!CanInteract(Interactor))
    {
        return false;
    }
    UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this);
    if (!Region)
    {
        return false;
    }
    switch (ObservationType)
    {
    case EWyrmCogspireObservationType::PublicMachinery:
        return Region->RecordPublicMachineryObserved();
    case EWyrmCogspireObservationType::CoercionDiversion:
        return Region->RecordCoercionDiversionObserved();
    case EWyrmCogspireObservationType::BaronAcknowledgment:
        return Region->RecordBaronAcknowledgment();
    default:
        return false;
    }
}
