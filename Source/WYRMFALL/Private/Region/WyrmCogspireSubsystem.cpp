#include "Region/WyrmCogspireSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Dragon/WyrmDragonCharacter.h"
#include "Player/WyrmCharacter.h"

namespace WyrmCogspireFacts
{
    const FName Arrival(TEXT("cogspire.arrival"));
    const FName ArrivalReceipt(TEXT("cogspire.arrival.recorded"));
    const FName PublicMachinery(TEXT("cogspire.public_machinery_observed"));
    const FName PublicMachineryReceipt(TEXT("cogspire.public_machinery.observed"));
    const FName CoercionDiversion(TEXT("cogspire.coercion_diversion_observed"));
    const FName CoercionDiversionReceipt(TEXT("cogspire.coercion_diversion.observed"));
    const FName BaronAcknowledgment(TEXT("cogspire.baron_acknowledged_diversion"));
    const FName BaronAcknowledgmentReceipt(TEXT("cogspire.baron.diversion_acknowledged"));
    const FName CogfangEncounter(TEXT("cogspire.cogfang_encounter_started"));
    const FName CogfangEncounterReceipt(TEXT("cogspire.cogfang.encounter_started"));
    const FName CogfangDefeated(TEXT("cogspire.cogfang_defeated_alive"));
    const FName CogfangDefeatedReceipt(TEXT("cogspire.cogfang.living_defeat"));
    const FName CoercionShutdown(TEXT("cogspire.coercion_governor_shutdown"));
    const FName CoercionShutdownReceipt(TEXT("cogspire.engine.coercion_governor_shutdown"));
    const FName CogfangBonded(TEXT("cogspire.cogfang_bonded"));
    const FName CogfangBondedReceipt(TEXT("cogspire.cogfang.voluntary_bond"));
}

void UWyrmCogspireSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ResetCogspireState();
}

UWyrmCogspireSubsystem* UWyrmCogspireSubsystem::GetCogspireSubsystem(UObject* WorldContextObject)
{
    if (!WorldContextObject || !GEngine)
    {
        return nullptr;
    }
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UWyrmCogspireSubsystem>();
        }
    }
    return nullptr;
}

void UWyrmCogspireSubsystem::ResetCogspireState()
{
    KnownFacts.Reset();
    FactReceipts.Reset();
    bCoercionGovernorActive = true;
    bCivicMachineryOperational = true;
}

bool UWyrmCogspireSubsystem::RecordArrival()
{
    return CommitFact(WyrmCogspireFacts::Arrival, WyrmCogspireFacts::ArrivalReceipt);
}

bool UWyrmCogspireSubsystem::RecordPublicMachineryObserved()
{
    return HasFact(WyrmCogspireFacts::Arrival) &&
        CommitFact(WyrmCogspireFacts::PublicMachinery, WyrmCogspireFacts::PublicMachineryReceipt);
}

bool UWyrmCogspireSubsystem::RecordCoercionDiversionObserved()
{
    return HasFact(WyrmCogspireFacts::PublicMachinery) &&
        CommitFact(WyrmCogspireFacts::CoercionDiversion, WyrmCogspireFacts::CoercionDiversionReceipt);
}

bool UWyrmCogspireSubsystem::RecordBaronAcknowledgment()
{
    return HasFact(WyrmCogspireFacts::CoercionDiversion) &&
        CommitFact(WyrmCogspireFacts::BaronAcknowledgment, WyrmCogspireFacts::BaronAcknowledgmentReceipt);
}

bool UWyrmCogspireSubsystem::RecordCogfangEncounterStarted(AWyrmDragonCharacter* Cogfang)
{
    if (!HasFact(WyrmCogspireFacts::BaronAcknowledgment) || !Cogfang ||
        Cogfang->DragonId != FName(TEXT("Cogfang")) || !Cogfang->HasSupportedRigProfile() ||
        Cogfang->GetDragonRole() != EWyrmDragonRole::HostileBoss)
    {
        return false;
    }
    return CommitFact(WyrmCogspireFacts::CogfangEncounter, WyrmCogspireFacts::CogfangEncounterReceipt);
}

bool UWyrmCogspireSubsystem::RecordCogfangLivingDefeat(AWyrmDragonCharacter* Cogfang)
{
    if (!HasFact(WyrmCogspireFacts::CogfangEncounter) || !Cogfang ||
        Cogfang->DragonId != FName(TEXT("Cogfang")) || !Cogfang->HasSupportedRigProfile() ||
        Cogfang->GetDragonRole() != EWyrmDragonRole::DefeatedAlive)
    {
        return false;
    }
    return CommitFact(WyrmCogspireFacts::CogfangDefeated, WyrmCogspireFacts::CogfangDefeatedReceipt);
}

bool UWyrmCogspireSubsystem::ShutdownCoercionGovernor(AWyrmDragonCharacter* Cogfang)
{
    if (!bCoercionGovernorActive || !bCivicMachineryOperational ||
        !HasFact(WyrmCogspireFacts::CogfangDefeated) || !Cogfang ||
        Cogfang->DragonId != FName(TEXT("Cogfang")) ||
        Cogfang->GetDragonRole() != EWyrmDragonRole::DefeatedAlive ||
        HasFact(WyrmCogspireFacts::CoercionShutdown) ||
        HasReceipt(WyrmCogspireFacts::CoercionShutdownReceipt))
    {
        return false;
    }
    bCoercionGovernorActive = false;
    if (!CommitFact(WyrmCogspireFacts::CoercionShutdown, WyrmCogspireFacts::CoercionShutdownReceipt))
    {
        bCoercionGovernorActive = true;
        return false;
    }
    return true;
}

bool UWyrmCogspireSubsystem::BondCogfang(AWyrmDragonCharacter* Cogfang, AWyrmCharacter* Humanoid)
{
    if (bCoercionGovernorActive || !bCivicMachineryOperational ||
        !HasFact(WyrmCogspireFacts::CoercionShutdown) || !Cogfang || !Humanoid ||
        Cogfang->DragonId != FName(TEXT("Cogfang")) || !Cogfang->HasSupportedRigProfile() ||
        Cogfang->GetDragonRole() != EWyrmDragonRole::DefeatedAlive || Cogfang->HasBondReceipt() ||
        HasFact(WyrmCogspireFacts::CogfangBonded) || HasReceipt(WyrmCogspireFacts::CogfangBondedReceipt))
    {
        return false;
    }
    return Cogfang->BondWithHumanoid(Humanoid) &&
        CommitFact(WyrmCogspireFacts::CogfangBonded, WyrmCogspireFacts::CogfangBondedReceipt);
}

bool UWyrmCogspireSubsystem::CommitFact(FName FactId, FName ReceiptId)
{
    if (FactId.IsNone() || ReceiptId.IsNone() || HasFact(FactId) || HasReceipt(ReceiptId))
    {
        return false;
    }
    KnownFacts.Add(FactId);
    FactReceipts.Add(ReceiptId);
    return true;
}

bool UWyrmCogspireSubsystem::HasFact(FName FactId) const
{
    return KnownFacts.Contains(FactId);
}

bool UWyrmCogspireSubsystem::HasReceipt(FName ReceiptId) const
{
    return FactReceipts.Contains(ReceiptId);
}
