#include "Region/WyrmCogspireCogfangEncounter.h"
#include "Region/WyrmCogspireSubsystem.h"
#include "Dragon/WyrmDragonCharacter.h"
#include "Player/WyrmCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "EngineUtils.h"

AWyrmCogspireCogfangEncounter::AWyrmCogspireCogfangEncounter()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
    EncounterVolume = CreateDefaultSubobject<USphereComponent>(TEXT("EncounterVolume"));
    EncounterVolume->SetupAttachment(SceneRoot);
    EncounterVolume->SetSphereRadius(EncounterRadius);
    EncounterVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    EncounterVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    EncounterVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    EncounterVolume->SetGenerateOverlapEvents(true);
    EncounterVolume->SetHiddenInGame(true);
}

void AWyrmCogspireCogfangEncounter::BeginPlay()
{
    Super::BeginPlay();
    EncounterVolume->SetSphereRadius(EncounterRadius);
    if (!Cogfang)
    {
        for (TActorIterator<AWyrmDragonCharacter> It(GetWorld()); It; ++It)
        {
            if (It->DragonId == FName(TEXT("Cogfang")))
            {
                Cogfang = *It;
                break;
            }
        }
    }
    if (Cogfang)
    {
        Cogfang->OnDragonDefeated.AddDynamic(this, &AWyrmCogspireCogfangEncounter::HandleDragonDefeated);
    }
}

bool AWyrmCogspireCogfangEncounter::CanBeginEncounter(const AWyrmCharacter* Player) const
{
    if (!Player || !Cogfang || FVector::DistSquared(GetActorLocation(), Player->GetActorLocation()) >
        FMath::Square(EncounterRadius))
    {
        return false;
    }
    const UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(
        const_cast<AWyrmCogspireCogfangEncounter*>(this));
    return Region && Region->HasFact(FName(TEXT("cogspire.baron_acknowledged_diversion"))) &&
        !Region->HasFact(FName(TEXT("cogspire.cogfang_encounter_started"))) &&
        Cogfang->DragonId == FName(TEXT("Cogfang")) && Cogfang->HasSupportedRigProfile() &&
        Cogfang->GetDragonRole() == EWyrmDragonRole::HostileBoss;
}

bool AWyrmCogspireCogfangEncounter::BeginEncounter(AWyrmCharacter* Player)
{
    if (!CanBeginEncounter(Player))
    {
        return false;
    }
    if (UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this))
    {
        return Region->RecordCogfangEncounterStarted(Cogfang);
    }
    return false;
}

void AWyrmCogspireCogfangEncounter::HandleDragonDefeated(AWyrmDragonCharacter* DefeatedDragon)
{
    if (DefeatedDragon == Cogfang)
    {
        if (UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this))
        {
            Region->RecordCogfangLivingDefeat(Cogfang);
        }
    }
}

bool AWyrmCogspireCogfangEncounter::InteractShutdownGovernor(AWyrmCharacter* Player)
{
    if (!Player || !Cogfang || FVector::DistSquared(GetActorLocation(), Player->GetActorLocation()) >
        FMath::Square(GovernorInteractionRadius))
    {
        return false;
    }
    if (UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this))
    {
        if (!Region->HasFact(FName(TEXT("cogspire.cogfang_defeated_alive"))))
        {
            Region->RecordCogfangLivingDefeat(Cogfang);
        }
        return Region->ShutdownCoercionGovernor(Cogfang);
    }
    return false;
}

bool AWyrmCogspireCogfangEncounter::InteractVoluntaryBond(AWyrmCharacter* Player)
{
    if (!Player || !Cogfang || FVector::DistSquared(Cogfang->GetActorLocation(), Player->GetActorLocation()) >
        FMath::Square(500.f))
    {
        return false;
    }
    if (UWyrmCogspireSubsystem* Region = UWyrmCogspireSubsystem::GetCogspireSubsystem(this))
    {
        return Region->BondCogfang(Cogfang, Player);
    }
    return false;
}
