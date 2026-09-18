#include "Region/WyrmRegion01Subsystem.h"

#include "Dragon/WyrmDragonCharacter.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

namespace WyrmRegion01
{
    const FName LandmarkHeart(TEXT("LM-HEART"));
    const FName LandmarkTamsin(TEXT("LM-TAMSIN"));
    const FName LandmarkTidecross(TEXT("LM-TIDECROSS"));
    const FName LandmarkQuietwater(TEXT("LM-QUIETWATER"));
    const FName LandmarkCamp(TEXT("LM-CAMP"));
    const FName LandmarkCutting(TEXT("LM-CUTTING"));
    const FName LandmarkSella(TEXT("LM-SELLA"));
    const FName LandmarkControl(TEXT("LM-CONTROL"));
    const FName LandmarkArena(TEXT("LM-ARENA"));
    const FName LandmarkTerrace(TEXT("LM-TERRACE"));
    const FName LandmarkTownEntry(TEXT("LM-TOWNENTRY"));
    const FName LandmarkCompactCave(TEXT("LM-COMPACTCAVE"));
    const FName LandmarkSilentLanding(TEXT("LM-SILENTLANDING"));

    const FName FactHeartExit(TEXT("heart.exit_reached"));
    const FName FactTidecrossVisited(TEXT("tidecross.visited"));
    const FName FactNoticeRead(TEXT("notice.read"));
    const FName FactPellSecured(TEXT("worker.pell.secured"));
    const FName FactIvenSecured(TEXT("worker.iven.secured"));
    const FName FactSellaSecured(TEXT("worker.sella.secured"));
    const FName FactMachineSeen(TEXT("evidence.machine_seen"));
    const FName FactRecords(TEXT("evidence.records"));
    const FName FactSellaAccount(TEXT("evidence.sella_account"));
    const FName FactAuxDisabled(TEXT("quarry.aux_disabled"));
    const FName FactRuskOutcome(TEXT("rusk.outcome"));
    const FName FactExtractionStopped(TEXT("quarry.extraction_stopped"));
    const FName FactVerdanceDefeatedAlive(TEXT("verdance.defeated_alive"));
    const FName FactVerdanceClaimBroken(TEXT("verdance.claim_broken"));
    const FName FactVerdanceBondAccepted(TEXT("verdance.bond_accepted"));
    const FName FactHeartfoldAvailable(TEXT("dragon.heartfold_available"));
    const FName FactReliefResolved(TEXT("relief.resolved"));
    const FName FactHomecomingComplete(TEXT("homecoming.complete"));
    const FName FactWageRecovered(TEXT("wage.recovered"));
    const FName FactCacheRecovered(TEXT("cache.recovered"));
    const FName FactCaveServiceUnlocked(TEXT("cave.service_unlocked"));
}

void UWyrmRegion01Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ResetRegion01State();
}

UWyrmRegion01Subsystem* UWyrmRegion01Subsystem::GetRegion01Subsystem(UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        return GameInstance->GetSubsystem<UWyrmRegion01Subsystem>();
    }
    return nullptr;
}

void UWyrmRegion01Subsystem::ResetRegion01State()
{
    State = FWyrmRegion01SaveRecord();
}

const TArray<FWyrmRegion01LandmarkDefinition>& UWyrmRegion01Subsystem::GetDefinitions()
{
    using namespace WyrmRegion01;

    static const TArray<FWyrmRegion01LandmarkDefinition> Definitions = {
        {LandmarkHeart, {LandmarkTamsin}, false, true},
        {LandmarkTamsin, {LandmarkHeart, LandmarkTidecross}, false, true},
        {LandmarkTidecross, {LandmarkTamsin, LandmarkQuietwater, LandmarkCamp, LandmarkCutting, LandmarkTownEntry, LandmarkSilentLanding}, false, true},
        {LandmarkQuietwater, {LandmarkTidecross}, true, true},
        {LandmarkCamp, {LandmarkTidecross}, true, true},
        {LandmarkCutting, {LandmarkTidecross, LandmarkSella, LandmarkControl}, false, true},
        {LandmarkSella, {LandmarkCutting, LandmarkControl}, false, true},
        {LandmarkControl, {LandmarkCutting, LandmarkSella, LandmarkArena}, false, true},
        {LandmarkArena, {LandmarkControl, LandmarkTerrace}, false, true},
        {LandmarkTerrace, {LandmarkArena, LandmarkTownEntry}, false, true},
        {LandmarkTownEntry, {LandmarkTidecross, LandmarkTerrace, LandmarkCompactCave}, false, true},
        {LandmarkCompactCave, {LandmarkTownEntry}, true, true},
        // A separate optional horror route, fact-gated after real local closure.
        {LandmarkSilentLanding, {LandmarkTidecross}, true, true},
    };
    return Definitions;
}

TArray<FWyrmRegion01LandmarkDefinition> UWyrmRegion01Subsystem::GetLandmarkDefinitions() const
{
    return GetDefinitions();
}

bool UWyrmRegion01Subsystem::IsKnownLandmark(FName LandmarkId) const
{
    return GetDefinitions().ContainsByPredicate([LandmarkId](const FWyrmRegion01LandmarkDefinition& Definition)
    {
        return Definition.LandmarkId == LandmarkId;
    });
}

bool UWyrmRegion01Subsystem::HasValidLandmarkGraph() const
{
    const TArray<FWyrmRegion01LandmarkDefinition>& Definitions = GetDefinitions();
    if (Definitions.Num() != 13)
    {
        return false;
    }

    TSet<FName> KnownLandmarks;
    for (const FWyrmRegion01LandmarkDefinition& Definition : Definitions)
    {
        if (Definition.LandmarkId.IsNone() || KnownLandmarks.Contains(Definition.LandmarkId))
        {
            return false;
        }
        KnownLandmarks.Add(Definition.LandmarkId);
    }

    for (const FWyrmRegion01LandmarkDefinition& Definition : Definitions)
    {
        for (const FName Connected : Definition.ConnectedLandmarks)
        {
            if (!KnownLandmarks.Contains(Connected))
            {
                return false;
            }

            const FWyrmRegion01LandmarkDefinition* Reverse = Definitions.FindByPredicate(
                [Connected](const FWyrmRegion01LandmarkDefinition& Candidate)
                {
                    return Candidate.LandmarkId == Connected;
                });
            if (!Reverse || !Reverse->ConnectedLandmarks.Contains(Definition.LandmarkId))
            {
                return false;
            }
        }
    }

    // Every logical landmark remains reachable from the heart exit. Optional
    // availability is controlled by facts, not by deleting a route or making
    // it an opening-story choke point.
    TSet<FName> Reached;
    TArray<FName> Pending{WyrmRegion01::LandmarkHeart};
    while (!Pending.IsEmpty())
    {
        const FName Current = Pending.Pop(EAllowShrinking::No);
        if (Reached.Contains(Current))
        {
            continue;
        }
        Reached.Add(Current);

        const FWyrmRegion01LandmarkDefinition* CurrentDefinition = Definitions.FindByPredicate(
            [Current](const FWyrmRegion01LandmarkDefinition& Candidate)
            {
                return Candidate.LandmarkId == Current;
            });
        if (CurrentDefinition)
        {
            Pending.Append(CurrentDefinition->ConnectedLandmarks);
        }
    }

    return Definitions.Num() == Reached.Num();
}

bool UWyrmRegion01Subsystem::IsLandmarkCurrentlyAvailable(FName LandmarkId) const
{
    if (!IsKnownLandmark(LandmarkId))
    {
        return false;
    }
    if (LandmarkId == WyrmRegion01::LandmarkCompactCave)
    {
        return IsBondedDragonAvailable();
    }
    if (LandmarkId == WyrmRegion01::LandmarkSilentLanding)
    {
        return IsHomecomingComplete();
    }
    return true;
}

bool UWyrmRegion01Subsystem::HasRouteBetweenLandmarks(FName FromLandmark, FName ToLandmark) const
{
    if (!IsKnownLandmark(FromLandmark) || !IsKnownLandmark(ToLandmark))
    {
        return false;
    }
    if (FromLandmark == ToLandmark)
    {
        return true;
    }

    const TArray<FWyrmRegion01LandmarkDefinition>& Definitions = GetDefinitions();
    TSet<FName> Reached;
    TArray<FName> Pending{FromLandmark};
    while (!Pending.IsEmpty())
    {
        const FName Current = Pending.Pop(EAllowShrinking::No);
        if (Current == ToLandmark)
        {
            return true;
        }
        if (Reached.Contains(Current))
        {
            continue;
        }
        Reached.Add(Current);

        const FWyrmRegion01LandmarkDefinition* CurrentDefinition = Definitions.FindByPredicate(
            [Current](const FWyrmRegion01LandmarkDefinition& Candidate)
            {
                return Candidate.LandmarkId == Current;
            });
        if (CurrentDefinition)
        {
            Pending.Append(CurrentDefinition->ConnectedLandmarks);
        }
    }
    return false;
}

bool UWyrmRegion01Subsystem::VisitLandmark(FName LandmarkId)
{
    if (!IsLandmarkCurrentlyAvailable(LandmarkId) || State.VisitedLandmarks.Contains(LandmarkId))
    {
        return false;
    }

    const FName VisitFact(*FString::Printf(TEXT("landmark.%s.visited"), *LandmarkId.ToString().ToLower()));
    const FName VisitReceipt(*FString::Printf(TEXT("region01.landmark.%s.visit"), *LandmarkId.ToString().ToLower()));
    if (!CommitFact(VisitFact, VisitReceipt))
    {
        return false;
    }
    State.VisitedLandmarks.Add(LandmarkId);

    if (LandmarkId == WyrmRegion01::LandmarkTidecross)
    {
        CommitFact(WyrmRegion01::FactTidecrossVisited, FName(TEXT("region01.tidecross.first_visit")));
    }
    RefreshDerivedFacts();
    return true;
}

bool UWyrmRegion01Subsystem::HasVisitedLandmark(FName LandmarkId) const
{
    return State.VisitedLandmarks.Contains(LandmarkId);
}

bool UWyrmRegion01Subsystem::RecordHeartExitReached()
{
    return CommitFact(WyrmRegion01::FactHeartExit, FName(TEXT("region01.heart.exit")));
}

bool UWyrmRegion01Subsystem::ReadCrownNotice()
{
    return CommitFact(WyrmRegion01::FactNoticeRead, FName(TEXT("region01.tidecross.notice")));
}

FName UWyrmRegion01Subsystem::GetWorkerFact(EWyrmRegion01Worker Worker)
{
    switch (Worker)
    {
    case EWyrmRegion01Worker::Pell: return WyrmRegion01::FactPellSecured;
    case EWyrmRegion01Worker::Iven: return WyrmRegion01::FactIvenSecured;
    case EWyrmRegion01Worker::Sella: return WyrmRegion01::FactSellaSecured;
    default: return NAME_None;
    }
}

FName UWyrmRegion01Subsystem::GetWorkerReceipt(EWyrmRegion01Worker Worker)
{
    switch (Worker)
    {
    case EWyrmRegion01Worker::Pell: return FName(TEXT("region01.worker.pell.secure"));
    case EWyrmRegion01Worker::Iven: return FName(TEXT("region01.worker.iven.secure"));
    case EWyrmRegion01Worker::Sella: return FName(TEXT("region01.worker.sella.secure"));
    default: return NAME_None;
    }
}

bool UWyrmRegion01Subsystem::SecureWorker(EWyrmRegion01Worker Worker)
{
    if (!CommitFact(GetWorkerFact(Worker), GetWorkerReceipt(Worker)))
    {
        return false;
    }

    if (Worker == EWyrmRegion01Worker::Sella)
    {
        // The rescued witness makes the technical explanation and the
        // optional restraint interaction available immediately.
        CommitFact(WyrmRegion01::FactSellaAccount, FName(TEXT("region01.sella.account")));
    }
    RefreshDerivedFacts();
    return true;
}

FName UWyrmRegion01Subsystem::GetEvidenceFact(EWyrmRegion01Evidence Evidence)
{
    switch (Evidence)
    {
    case EWyrmRegion01Evidence::Machine: return WyrmRegion01::FactMachineSeen;
    case EWyrmRegion01Evidence::Records: return WyrmRegion01::FactRecords;
    case EWyrmRegion01Evidence::SellaAccount: return WyrmRegion01::FactSellaAccount;
    default: return NAME_None;
    }
}

FName UWyrmRegion01Subsystem::GetEvidenceReceipt(EWyrmRegion01Evidence Evidence)
{
    switch (Evidence)
    {
    case EWyrmRegion01Evidence::Machine: return FName(TEXT("region01.evidence.machine"));
    case EWyrmRegion01Evidence::Records: return FName(TEXT("region01.evidence.records"));
    case EWyrmRegion01Evidence::SellaAccount: return FName(TEXT("region01.evidence.sella_account"));
    default: return NAME_None;
    }
}

bool UWyrmRegion01Subsystem::DiscoverEvidence(EWyrmRegion01Evidence Evidence)
{
    if (Evidence == EWyrmRegion01Evidence::SellaAccount && !HasFact(WyrmRegion01::FactSellaSecured))
    {
        return false;
    }
    return CommitFact(GetEvidenceFact(Evidence), GetEvidenceReceipt(Evidence));
}

bool UWyrmRegion01Subsystem::DisableAuxiliaryRestraint()
{
    if (!IsAuxiliaryShutdownAvailable())
    {
        return false;
    }
    return CommitFact(WyrmRegion01::FactAuxDisabled, FName(TEXT("region01.quarry.aux_disabled")));
}

bool UWyrmRegion01Subsystem::ResolveRusk(EWyrmRegion01RuskOutcome DesiredOutcome)
{
    if (State.RuskOutcome != EWyrmRegion01RuskOutcome::Unresolved || DesiredOutcome == EWyrmRegion01RuskOutcome::Unresolved)
    {
        return false;
    }
    if (DesiredOutcome == EWyrmRegion01RuskOutcome::SurrenderedCustody && !HasFact(WyrmRegion01::FactVerdanceBondAccepted))
    {
        return false;
    }

    const FName OutcomeReceipt = DesiredOutcome == EWyrmRegion01RuskOutcome::DefeatedCustody
        ? FName(TEXT("region01.rusk.defeated_custody"))
        : FName(TEXT("region01.rusk.surrendered_custody"));
    if (!CommitFact(WyrmRegion01::FactRuskOutcome, OutcomeReceipt))
    {
        return false;
    }
    State.RuskOutcome = DesiredOutcome;
    return true;
}

bool UWyrmRegion01Subsystem::StopCrowncutExtraction()
{
    // A valid control interaction can stop the works before the player
    // confronts Rusk. The caller is responsible for representing that actual
    // interaction; this fact layer must not turn custody into a route gate.
    return CommitFact(WyrmRegion01::FactExtractionStopped, FName(TEXT("region01.quarry.control_shutdown")));
}

bool UWyrmRegion01Subsystem::RecordVerdanceDefeatedAlive()
{
    return CommitFact(WyrmRegion01::FactVerdanceDefeatedAlive, FName(TEXT("region01.verdance.defeated_alive")));
}

bool UWyrmRegion01Subsystem::BreakVerdanceClaim()
{
    if (!HasFact(WyrmRegion01::FactVerdanceDefeatedAlive))
    {
        return false;
    }
    if (!CommitFact(WyrmRegion01::FactVerdanceClaimBroken, FName(TEXT("region01.verdance.claim_broken"))))
    {
        return false;
    }

    // Breaking the primary claim is an independent valid way to end
    // extraction when the player has bypassed the control works/Rusk.
    CommitFact(WyrmRegion01::FactExtractionStopped, FName(TEXT("region01.quarry.primary_claim_destroyed")));
    return true;
}

bool UWyrmRegion01Subsystem::RecordVerdanceBondAccepted(AWyrmDragonCharacter* Verdance)
{
    if (!Verdance || Verdance->DragonId != FName(TEXT("Verdance")) || !Verdance->HasBondReceipt() ||
        !HasFact(WyrmRegion01::FactVerdanceClaimBroken))
    {
        return false;
    }
    const bool bCommitted = CommitFact(WyrmRegion01::FactVerdanceBondAccepted, FName(TEXT("region01.verdance.bond_accepted")));
    if (bCommitted)
    {
        EnsureDerivedFact(WyrmRegion01::FactHeartfoldAvailable, FName(TEXT("region01.dragon.heartfold_available")));
    }
    return bCommitted;
}

bool UWyrmRegion01Subsystem::RecordReliefResolved()
{
    if (!HasFact(WyrmRegion01::FactVerdanceBondAccepted) || !HasFact(WyrmRegion01::FactExtractionStopped))
    {
        return false;
    }
    return CommitFact(WyrmRegion01::FactReliefResolved, FName(TEXT("region01.relief.resolved")));
}

bool UWyrmRegion01Subsystem::RecoverOptionalWageRecord()
{
    return CommitFact(WyrmRegion01::FactWageRecovered, FName(TEXT("region01.wage.recovered")));
}

bool UWyrmRegion01Subsystem::RecoverServiceCache()
{
    if (!IsBondedDragonAvailable())
    {
        return false;
    }
    if (!CommitFact(WyrmRegion01::FactCacheRecovered, FName(TEXT("region01.cache.recovered"))))
    {
        return false;
    }
    EnsureDerivedFact(WyrmRegion01::FactCaveServiceUnlocked, FName(TEXT("region01.cave.service_unlocked")));
    return true;
}

bool UWyrmRegion01Subsystem::IsServiceCacheRecovered() const
{
    return HasFact(WyrmRegion01::FactCacheRecovered);
}

bool UWyrmRegion01Subsystem::HasFact(FName FactId) const
{
    return State.KnownFacts.Contains(FactId);
}

bool UWyrmRegion01Subsystem::HasConflictEvidence() const
{
    int32 Sources = 0;
    Sources += HasFact(WyrmRegion01::FactMachineSeen) ? 1 : 0;
    Sources += HasFact(WyrmRegion01::FactRecords) ? 1 : 0;
    Sources += HasFact(WyrmRegion01::FactSellaAccount) ? 1 : 0;
    return Sources >= 2;
}

bool UWyrmRegion01Subsystem::IsAuxiliaryShutdownAvailable() const
{
    return HasFact(WyrmRegion01::FactSellaSecured) && !HasFact(WyrmRegion01::FactAuxDisabled);
}

bool UWyrmRegion01Subsystem::IsBondedDragonAvailable() const
{
    return HasFact(WyrmRegion01::FactVerdanceBondAccepted);
}

bool UWyrmRegion01Subsystem::IsPartialDebriefAvailable() const
{
    return HasFact(WyrmRegion01::FactTidecrossVisited) && !IsHomecomingComplete() &&
        (IsBondedDragonAvailable() || HasFact(WyrmRegion01::FactPellSecured) ||
            HasFact(WyrmRegion01::FactIvenSecured) || HasFact(WyrmRegion01::FactSellaSecured));
}

bool UWyrmRegion01Subsystem::IsHomecomingReady() const
{
    const bool bAllWorkersSecured = HasFact(WyrmRegion01::FactPellSecured) &&
        HasFact(WyrmRegion01::FactIvenSecured) && HasFact(WyrmRegion01::FactSellaSecured);
    const bool bLocalResolution = State.RuskOutcome != EWyrmRegion01RuskOutcome::Unresolved &&
        HasFact(WyrmRegion01::FactReliefResolved);
    return bAllWorkersSecured && HasFact(WyrmRegion01::FactExtractionStopped) &&
        HasFact(WyrmRegion01::FactVerdanceBondAccepted) && bLocalResolution;
}

bool UWyrmRegion01Subsystem::CompleteHomecoming()
{
    if (!HasFact(WyrmRegion01::FactTidecrossVisited) || !IsHomecomingReady())
    {
        return false;
    }
    return CommitFact(WyrmRegion01::FactHomecomingComplete, FName(TEXT("region01.homecoming.complete")));
}

bool UWyrmRegion01Subsystem::IsHomecomingComplete() const
{
    return HasFact(WyrmRegion01::FactHomecomingComplete);
}

void UWyrmRegion01Subsystem::BuildSaveRecord(FWyrmRegion01SaveRecord& OutRecord) const
{
    OutRecord = State;
}

void UWyrmRegion01Subsystem::RestoreFromSaveRecord(const FWyrmRegion01SaveRecord& InRecord)
{
    State = InRecord;
    NormalizeRestoredState();
    RefreshDerivedFacts();
}

bool UWyrmRegion01Subsystem::CommitFact(FName FactId, FName ReceiptId)
{
    if (FactId.IsNone() || ReceiptId.IsNone() || HasFact(FactId) || HasReceipt(ReceiptId))
    {
        return false;
    }
    State.KnownFacts.Add(FactId);
    State.FactReceipts.Add({ReceiptId, FactId});
    RefreshDerivedFacts();
    return true;
}

void UWyrmRegion01Subsystem::EnsureDerivedFact(FName FactId, FName ReceiptId)
{
    if (!HasFact(FactId))
    {
        State.KnownFacts.Add(FactId);
    }
    if (!HasReceipt(ReceiptId))
    {
        State.FactReceipts.Add({ReceiptId, FactId});
    }
}

bool UWyrmRegion01Subsystem::HasReceipt(FName ReceiptId) const
{
    return State.FactReceipts.ContainsByPredicate([ReceiptId](const FWyrmRegion01FactReceipt& Receipt)
    {
        return Receipt.ReceiptId == ReceiptId;
    });
}

void UWyrmRegion01Subsystem::RefreshDerivedFacts()
{
    if (HasFact(WyrmRegion01::FactVerdanceBondAccepted))
    {
        EnsureDerivedFact(WyrmRegion01::FactHeartfoldAvailable, FName(TEXT("region01.dragon.heartfold_available")));
    }
    else
    {
        State.KnownFacts.Remove(WyrmRegion01::FactHeartfoldAvailable);
        State.FactReceipts.RemoveAll([](const FWyrmRegion01FactReceipt& Receipt)
        {
            return Receipt.ReceiptId == FName(TEXT("region01.dragon.heartfold_available"));
        });
    }
}

void UWyrmRegion01Subsystem::NormalizeRestoredState()
{
    TSet<FName> UniqueFacts;
    State.KnownFacts.RemoveAll([&UniqueFacts](FName FactId)
    {
        if (FactId.IsNone() || UniqueFacts.Contains(FactId))
        {
            return true;
        }
        UniqueFacts.Add(FactId);
        return false;
    });

    TSet<FName> UniqueLandmarks;
    State.VisitedLandmarks.RemoveAll([this, &UniqueLandmarks](FName LandmarkId)
    {
        if (!IsKnownLandmark(LandmarkId) || UniqueLandmarks.Contains(LandmarkId))
        {
            return true;
        }
        UniqueLandmarks.Add(LandmarkId);
        return false;
    });

    TSet<FName> UniqueReceipts;
    State.FactReceipts.RemoveAll([&UniqueReceipts](const FWyrmRegion01FactReceipt& Receipt)
    {
        if (Receipt.ReceiptId.IsNone() || Receipt.FactId.IsNone() || UniqueReceipts.Contains(Receipt.ReceiptId))
        {
            return true;
        }
        UniqueReceipts.Add(Receipt.ReceiptId);
        return false;
    });
    for (const FWyrmRegion01FactReceipt& Receipt : State.FactReceipts)
    {
        if (!State.KnownFacts.Contains(Receipt.FactId))
        {
            State.KnownFacts.Add(Receipt.FactId);
        }
    }

    if (State.RuskOutcome != EWyrmRegion01RuskOutcome::Unresolved &&
        State.RuskOutcome != EWyrmRegion01RuskOutcome::DefeatedCustody &&
        State.RuskOutcome != EWyrmRegion01RuskOutcome::SurrenderedCustody)
    {
        State.RuskOutcome = EWyrmRegion01RuskOutcome::Unresolved;
    }
    if (State.RuskOutcome != EWyrmRegion01RuskOutcome::Unresolved)
    {
        EnsureDerivedFact(WyrmRegion01::FactRuskOutcome,
            State.RuskOutcome == EWyrmRegion01RuskOutcome::DefeatedCustody
                ? FName(TEXT("region01.rusk.defeated_custody"))
                : FName(TEXT("region01.rusk.surrendered_custody")));
    }

    // A serialized completion receipt cannot turn a partial/corrupt record
    // into a false town resolution. Completion is an actual Tidecross event
    // after the full local closure predicate has become true.
    if (!HasFact(WyrmRegion01::FactTidecrossVisited) || !IsHomecomingReady())
    {
        State.KnownFacts.Remove(WyrmRegion01::FactHomecomingComplete);
        State.FactReceipts.RemoveAll([](const FWyrmRegion01FactReceipt& Receipt)
        {
            return Receipt.ReceiptId == FName(TEXT("region01.homecoming.complete"));
        });
    }
}
