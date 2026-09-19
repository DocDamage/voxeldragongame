#include "Region/WyrmJadePeaksSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UWyrmJadePeaksSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ResetJadePeaksState();
}

UWyrmJadePeaksSubsystem* UWyrmJadePeaksSubsystem::GetJadePeaksSubsystem(UObject* WorldContextObject)
{
    if (!WorldContextObject || !GEngine)
    {
        return nullptr;
    }
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UWyrmJadePeaksSubsystem>();
        }
    }
    return nullptr;
}

void UWyrmJadePeaksSubsystem::ResetJadePeaksState()
{
    State = FWyrmJadePeaksSaveRecord();
}

const TArray<FWyrmJadePeaksLandmarkDefinition>& UWyrmJadePeaksSubsystem::GetDefinitions()
{
    static const TArray<FWyrmJadePeaksLandmarkDefinition> Definitions = {
        { FName(TEXT("LM-JADE-ARRIVAL")), { FName(TEXT("LM-JADE-PALACE")), FName(TEXT("LM-JADE-RETURN")) }, false },
        { FName(TEXT("LM-JADE-PALACE")), { FName(TEXT("LM-JADE-ARRIVAL")), FName(TEXT("LM-JADE-STORMCOURT")), FName(TEXT("LM-JADE-MIRRORWELL")) }, false },
        { FName(TEXT("LM-JADE-STORMCOURT")), { FName(TEXT("LM-JADE-PALACE")), FName(TEXT("LM-JADE-AERIE")) }, true },
        { FName(TEXT("LM-JADE-MIRRORWELL")), { FName(TEXT("LM-JADE-PALACE")), FName(TEXT("LM-JADE-AERIE")) }, false },
        { FName(TEXT("LM-JADE-AERIE")), { FName(TEXT("LM-JADE-STORMCOURT")), FName(TEXT("LM-JADE-MIRRORWELL")) }, true },
        { FName(TEXT("LM-JADE-RETURN")), { FName(TEXT("LM-JADE-ARRIVAL")) }, false },
    };
    return Definitions;
}

TArray<FWyrmJadePeaksLandmarkDefinition> UWyrmJadePeaksSubsystem::GetLandmarkDefinitions() const
{
    return GetDefinitions();
}

bool UWyrmJadePeaksSubsystem::HasValidLandmarkGraph() const
{
    const TArray<FWyrmJadePeaksLandmarkDefinition>& Definitions = GetDefinitions();
    if (Definitions.Num() != 6)
    {
        return false;
    }
    TSet<FName> Ids;
    for (const FWyrmJadePeaksLandmarkDefinition& Definition : Definitions)
    {
        if (Definition.LandmarkId.IsNone() || Ids.Contains(Definition.LandmarkId))
        {
            return false;
        }
        Ids.Add(Definition.LandmarkId);
    }
    for (const FWyrmJadePeaksLandmarkDefinition& Definition : Definitions)
    {
        for (const FName Connected : Definition.ConnectedLandmarks)
        {
            const FWyrmJadePeaksLandmarkDefinition* Reverse = Definitions.FindByPredicate(
                [Connected](const FWyrmJadePeaksLandmarkDefinition& Candidate) { return Candidate.LandmarkId == Connected; });
            if (!Reverse || !Reverse->ConnectedLandmarks.Contains(Definition.LandmarkId))
            {
                return false;
            }
        }
    }
    return true;
}

bool UWyrmJadePeaksSubsystem::IsKnownLandmark(FName LandmarkId) const
{
    return GetDefinitions().ContainsByPredicate(
        [LandmarkId](const FWyrmJadePeaksLandmarkDefinition& Definition) { return Definition.LandmarkId == LandmarkId; });
}

bool UWyrmJadePeaksSubsystem::VisitLandmark(FName LandmarkId)
{
    if (!IsKnownLandmark(LandmarkId) || State.VisitedLandmarks.Contains(LandmarkId))
    {
        return false;
    }
    State.VisitedLandmarks.Add(LandmarkId);
    if (LandmarkId == FName(TEXT("LM-JADE-ARRIVAL")))
    {
        CommitFact(FName(TEXT("jade.arrival")), FName(TEXT("jadepeaks.arrival")));
    }
    return true;
}

bool UWyrmJadePeaksSubsystem::HasVisitedLandmark(FName LandmarkId) const
{
    return State.VisitedLandmarks.Contains(LandmarkId);
}

bool UWyrmJadePeaksSubsystem::RecordJadefangArrival()
{
    return CommitFact(FName(TEXT("jadefang.arrived")), FName(TEXT("jadepeaks.jadefang.arrived")));
}

bool UWyrmJadePeaksSubsystem::RecordDiscipleResolution(bool bTrustedResolution)
{
    return CommitFact(
        bTrustedResolution ? FName(TEXT("disciple.trusted")) : FName(TEXT("disciple.defeated_alive")),
        bTrustedResolution ? FName(TEXT("jadepeaks.disciple.trusted")) : FName(TEXT("jadepeaks.disciple.defeated_alive")));
}

bool UWyrmJadePeaksSubsystem::RecordMirrorStepUnlock()
{
    return CommitFact(FName(TEXT("echo.mirror_step")), FName(TEXT("jadepeaks.echo.mirror_step")));
}

bool UWyrmJadePeaksSubsystem::RecordUnseenHandUnlock()
{
    if (!HasFact(FName(TEXT("disciple.trusted"))) && !HasFact(FName(TEXT("disciple.defeated_alive"))))
    {
        return false;
    }
    return CommitFact(FName(TEXT("echo.unseen_hand")), FName(TEXT("jadepeaks.echo.unseen_hand")));
}

bool UWyrmJadePeaksSubsystem::RecordImperialPactResolution(bool bDiplomacy)
{
    if (!HasFact(FName(TEXT("jadefang.arrived"))) || HasFact(FName(TEXT("jade.imperial_pact_resolved"))))
    {
        return false;
    }
    const FName RouteFact = bDiplomacy ? FName(TEXT("jade.emperor_persuaded")) : FName(TEXT("jade.emperor_defeated_alive"));
    const FName RouteReceipt = bDiplomacy ? FName(TEXT("jadepeaks.emperor.persuaded")) : FName(TEXT("jadepeaks.emperor.defeated_alive"));
    if (!CommitFact(RouteFact, RouteReceipt)) return false;
    return CommitFact(FName(TEXT("jade.imperial_pact_resolved")), FName(TEXT("jadepeaks.imperial_pact_resolved")));
}

bool UWyrmJadePeaksSubsystem::IsRegionalClosureComplete() const
{
    return HasFact(FName(TEXT("jade.imperial_pact_resolved"))) &&
           HasFact(FName(TEXT("echo.unseen_hand"))) &&
           HasFact(FName(TEXT("jade.return_route_ready")));
}

bool UWyrmJadePeaksSubsystem::RecordReturnRouteReady()
{
    if (!HasFact(FName(TEXT("echo.mirror_step"))) && !HasFact(FName(TEXT("echo.unseen_hand"))))
    {
        return false;
    }
    return CommitFact(FName(TEXT("jade.return_route_ready")), FName(TEXT("jadepeaks.return_route_ready")));
}

bool UWyrmJadePeaksSubsystem::CommitFact(FName FactId, FName ReceiptId)
{
    if (FactId.IsNone() || ReceiptId.IsNone() || HasReceipt(ReceiptId))
    {
        return false;
    }
    State.KnownFacts.AddUnique(FactId);
    FWyrmJadePeaksFactReceipt Receipt;
    Receipt.FactId = FactId;
    Receipt.ReceiptId = ReceiptId;
    State.FactReceipts.Add(Receipt);
    return true;
}

bool UWyrmJadePeaksSubsystem::HasFact(FName FactId) const
{
    return State.KnownFacts.Contains(FactId);
}

bool UWyrmJadePeaksSubsystem::HasReceipt(FName ReceiptId) const
{
    return State.FactReceipts.ContainsByPredicate(
        [ReceiptId](const FWyrmJadePeaksFactReceipt& Receipt) { return Receipt.ReceiptId == ReceiptId; });
}

void UWyrmJadePeaksSubsystem::BuildSaveRecord(FWyrmJadePeaksSaveRecord& OutRecord) const
{
    OutRecord = State;
}

void UWyrmJadePeaksSubsystem::RestoreFromSaveRecord(const FWyrmJadePeaksSaveRecord& InRecord)
{
    State = InRecord;
    NormalizeRestoredState();
}

void UWyrmJadePeaksSubsystem::NormalizeRestoredState()
{
    TSet<FName> UniqueFacts;
    State.KnownFacts.RemoveAll([&UniqueFacts](FName Fact)
    {
        return Fact.IsNone() || (!UniqueFacts.Contains(Fact) ? (UniqueFacts.Add(Fact), false) : true);
    });

    TSet<FName> UniqueReceipts;
    State.FactReceipts.RemoveAll([this, &UniqueReceipts](const FWyrmJadePeaksFactReceipt& Receipt)
    {
        if (Receipt.ReceiptId.IsNone() || Receipt.FactId.IsNone() || UniqueReceipts.Contains(Receipt.ReceiptId))
        {
            return true;
        }
        UniqueReceipts.Add(Receipt.ReceiptId);
        State.KnownFacts.AddUnique(Receipt.FactId);
        return false;
    });

    TSet<FName> UniqueLandmarks;
    State.VisitedLandmarks.RemoveAll([this, &UniqueLandmarks](FName Landmark)
    {
        if (!IsKnownLandmark(Landmark) || UniqueLandmarks.Contains(Landmark))
        {
            return true;
        }
        UniqueLandmarks.Add(Landmark);
        return false;
    });
}
