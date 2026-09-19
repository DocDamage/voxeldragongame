#include "Region/WyrmGloamingAshgraveSeal.h"
#include "Region/WyrmGloamingSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"

AWyrmGloamingAshgraveSeal::AWyrmGloamingAshgraveSeal()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
    InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
    InteractionVolume->SetupAttachment(SceneRoot);
    InteractionVolume->SetBoxExtent(FVector(160.f, 160.f, 140.f));
    InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    InteractionVolume->SetGenerateOverlapEvents(true);
    InteractionVolume->SetHiddenInGame(true);
}

bool AWyrmGloamingAshgraveSeal::CanInteract(const AActor* Interactor) const
{
    if (!Interactor || FVector::DistSquared(GetActorLocation(), Interactor->GetActorLocation()) > FMath::Square(InteractionRadius))
    {
        return false;
    }
    const UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(
        const_cast<AWyrmGloamingAshgraveSeal*>(this));
    return Region && Region->HasFact(FName(TEXT("gloaming.arrival"))) &&
        !Region->HasFact(FName(TEXT("gloaming.ashgrave_extraction_seal_resolved")));
}

bool AWyrmGloamingAshgraveSeal::Interact(AActor* Interactor)
{
    if (!CanInteract(Interactor))
    {
        return false;
    }
    if (UWyrmGloamingSubsystem* Region = UWyrmGloamingSubsystem::GetGloamingSubsystem(this))
    {
        return Region->ResolveAshgraveExtractionSeal();
    }
    return false;
}
