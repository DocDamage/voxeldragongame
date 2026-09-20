#include "Region/WyrmCogspireSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Dragon/WyrmDragonCharacter.h"
#include "Player/WyrmCharacter.h"
#include "Region/WyrmWorldTravelSubsystem.h"

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
    const FName RegionComplete(TEXT("cogspire.region_complete"));
    const FName RegionCompleteReceipt(TEXT("cogspire.region.completion_committed"));
    const FName HouseMarkVictim(TEXT("cogspire.house_mark.victim_examined"));
    const FName HouseMarkVictimReceipt(TEXT("cogspire.house_mark.victim_evidence_recorded"));
    const FName HouseMarkRelic(TEXT("cogspire.house_mark.relic_trade_traced"));
    const FName HouseMarkRelicReceipt(TEXT("cogspire.house_mark.relic_trade_evidence_recorded"));
    const FName HouseMarkIdentity(TEXT("cogspire.house_mark.identified"));
    const FName HouseMarkIdentityReceipt(TEXT("cogspire.house_mark.identity_confirmed"));
    const FName HouseMarkConfrontation(TEXT("cogspire.house_mark.confrontation_started"));
    const FName HouseMarkConfrontationReceipt(TEXT("cogspire.house_mark.confrontation_committed"));
    const FName HouseMarkDefeated(TEXT("cogspire.house_mark.defeated"));
    const FName HouseMarkDefeatedReceipt(TEXT("cogspire.house_mark.defeat_committed"));
    const FName Deathmark(TEXT("echo.deathmark"));
    const FName DeathmarkReceipt(TEXT("cogspire.house_mark.deathmark_manifested"));
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

bool UWyrmCogspireSubsystem::CanCompleteRegion(
    const AWyrmDragonCharacter* Cogfang, const UWyrmWorldTravelSubsystem* Travel) const
{
    static const FName CogspireHarbor(TEXT("CogspireHarbor"));
    static const FName Region01(TEXT("Region01"));
    static const FName CogspireArrival(TEXT("LM-COGSPIRE-ARRIVAL"));
    return HasFact(WyrmCogspireFacts::CogfangBonded) &&
        HasReceipt(WyrmCogspireFacts::CogfangBondedReceipt) &&
        !HasFact(WyrmCogspireFacts::RegionComplete) &&
        !HasReceipt(WyrmCogspireFacts::RegionCompleteReceipt) &&
        !bCoercionGovernorActive && bCivicMachineryOperational &&
        Cogfang && Cogfang->DragonId == FName(TEXT("Cogfang")) &&
        Cogfang->HasSupportedRigProfile() && Cogfang->HasBondReceipt() &&
        Cogfang->GetDragonRole() == EWyrmDragonRole::AlliedCompanion &&
        Travel && Travel->GetCurrentRegionId() == CogspireHarbor &&
        Travel->GetArrivalLandmarkId() == CogspireArrival &&
        Travel->IsAllowedRoute(CogspireHarbor, Region01);
}

bool UWyrmCogspireSubsystem::RecordRegionalCompletion(
    AWyrmDragonCharacter* Cogfang, UWyrmWorldTravelSubsystem* Travel)
{
    return CanCompleteRegion(Cogfang, Travel) &&
        CommitFact(WyrmCogspireFacts::RegionComplete, WyrmCogspireFacts::RegionCompleteReceipt);
}

bool UWyrmCogspireSubsystem::RecordHouseMarkVictimEvidence()
{
    return HasFact(WyrmCogspireFacts::Arrival) &&
        CommitFact(WyrmCogspireFacts::HouseMarkVictim, WyrmCogspireFacts::HouseMarkVictimReceipt);
}

bool UWyrmCogspireSubsystem::RecordHouseMarkRelicEvidence()
{
    return HasFact(WyrmCogspireFacts::HouseMarkVictim) &&
        CommitFact(WyrmCogspireFacts::HouseMarkRelic, WyrmCogspireFacts::HouseMarkRelicReceipt);
}

bool UWyrmCogspireSubsystem::RecordHouseMarkIdentity()
{
    return HasFact(WyrmCogspireFacts::HouseMarkRelic) &&
        CommitFact(WyrmCogspireFacts::HouseMarkIdentity, WyrmCogspireFacts::HouseMarkIdentityReceipt);
}

bool UWyrmCogspireSubsystem::RecordHouseMarkConfrontation()
{
    return HasFact(WyrmCogspireFacts::HouseMarkIdentity) &&
        CommitFact(WyrmCogspireFacts::HouseMarkConfrontation, WyrmCogspireFacts::HouseMarkConfrontationReceipt);
}

bool UWyrmCogspireSubsystem::RecordHouseMarkDefeat()
{
    return HasFact(WyrmCogspireFacts::HouseMarkConfrontation) &&
        CommitFact(WyrmCogspireFacts::HouseMarkDefeated, WyrmCogspireFacts::HouseMarkDefeatedReceipt);
}

bool UWyrmCogspireSubsystem::RecordDeathmarkUnlock()
{
    return HasFact(WyrmCogspireFacts::HouseMarkDefeated) &&
        CommitFact(WyrmCogspireFacts::Deathmark, WyrmCogspireFacts::DeathmarkReceipt);
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

void UWyrmCogspireSubsystem::BuildSaveRecord(FWyrmCogspireSaveRecord& OutRecord) const
{
    OutRecord.KnownFacts = KnownFacts;
    OutRecord.FactReceipts = FactReceipts;
    OutRecord.bCoercionGovernorActive = bCoercionGovernorActive;
    OutRecord.bCivicMachineryOperational = bCivicMachineryOperational;
}

void UWyrmCogspireSubsystem::RestoreFromSaveRecord(const FWyrmCogspireSaveRecord& InRecord)
{
    KnownFacts = InRecord.KnownFacts;
    FactReceipts = InRecord.FactReceipts;
    bCoercionGovernorActive = InRecord.bCoercionGovernorActive;
    bCivicMachineryOperational = InRecord.bCivicMachineryOperational;
    NormalizeRestoredState();
}

void UWyrmCogspireSubsystem::NormalizeRestoredState()
{
    TSet<FName> UniqueFacts;
    KnownFacts.RemoveAll([&UniqueFacts](FName Fact)
    {
        return Fact.IsNone() || (!UniqueFacts.Contains(Fact) ? (UniqueFacts.Add(Fact), false) : true);
    });

    TSet<FName> UniqueReceipts;
    FactReceipts.RemoveAll([&UniqueReceipts](FName Receipt)
    {
        return Receipt.IsNone() || (!UniqueReceipts.Contains(Receipt) ? (UniqueReceipts.Add(Receipt), false) : true);
    });

    const FName OrderedFacts[] = {
        WyrmCogspireFacts::Arrival,
        WyrmCogspireFacts::PublicMachinery,
        WyrmCogspireFacts::CoercionDiversion,
        WyrmCogspireFacts::BaronAcknowledgment,
        WyrmCogspireFacts::CogfangEncounter,
        WyrmCogspireFacts::CogfangDefeated,
        WyrmCogspireFacts::CoercionShutdown,
        WyrmCogspireFacts::CogfangBonded,
        WyrmCogspireFacts::RegionComplete};
    const FName OrderedReceipts[] = {
        WyrmCogspireFacts::ArrivalReceipt,
        WyrmCogspireFacts::PublicMachineryReceipt,
        WyrmCogspireFacts::CoercionDiversionReceipt,
        WyrmCogspireFacts::BaronAcknowledgmentReceipt,
        WyrmCogspireFacts::CogfangEncounterReceipt,
        WyrmCogspireFacts::CogfangDefeatedReceipt,
        WyrmCogspireFacts::CoercionShutdownReceipt,
        WyrmCogspireFacts::CogfangBondedReceipt,
        WyrmCogspireFacts::RegionCompleteReceipt};

    bool bChainValid = true;
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(OrderedFacts); ++Index)
    {
        const bool bPairPresent = KnownFacts.Contains(OrderedFacts[Index]) &&
            FactReceipts.Contains(OrderedReceipts[Index]);
        bChainValid = bChainValid && bPairPresent;
        if (!bChainValid)
        {
            KnownFacts.Remove(OrderedFacts[Index]);
            FactReceipts.Remove(OrderedReceipts[Index]);
        }
    }

    const bool bShutdownValid = KnownFacts.Contains(WyrmCogspireFacts::CoercionShutdown) &&
        FactReceipts.Contains(WyrmCogspireFacts::CoercionShutdownReceipt);
    bCoercionGovernorActive = !bShutdownValid;

    // Civic infrastructure is never a valid casualty of this selective shutdown.
    bCivicMachineryOperational = true;

    const FName OptionalFacts[] = {
        WyrmCogspireFacts::HouseMarkVictim,
        WyrmCogspireFacts::HouseMarkRelic,
        WyrmCogspireFacts::HouseMarkIdentity,
        WyrmCogspireFacts::HouseMarkConfrontation,
        WyrmCogspireFacts::HouseMarkDefeated,
        WyrmCogspireFacts::Deathmark};
    const FName OptionalReceipts[] = {
        WyrmCogspireFacts::HouseMarkVictimReceipt,
        WyrmCogspireFacts::HouseMarkRelicReceipt,
        WyrmCogspireFacts::HouseMarkIdentityReceipt,
        WyrmCogspireFacts::HouseMarkConfrontationReceipt,
        WyrmCogspireFacts::HouseMarkDefeatedReceipt,
        WyrmCogspireFacts::DeathmarkReceipt};
    bool bOptionalChainValid = KnownFacts.Contains(WyrmCogspireFacts::Arrival) &&
        FactReceipts.Contains(WyrmCogspireFacts::ArrivalReceipt);
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(OptionalFacts); ++Index)
    {
        const bool bPairPresent = KnownFacts.Contains(OptionalFacts[Index]) &&
            FactReceipts.Contains(OptionalReceipts[Index]);
        bOptionalChainValid = bOptionalChainValid && bPairPresent;
        if (!bOptionalChainValid)
        {
            KnownFacts.Remove(OptionalFacts[Index]);
            FactReceipts.Remove(OptionalReceipts[Index]);
        }
    }
}
