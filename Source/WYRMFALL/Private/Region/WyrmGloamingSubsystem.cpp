#include "Region/WyrmGloamingSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

namespace WyrmGloamingFacts
{
    const FName Arrival(TEXT("gloaming.arrival"));
    const FName AshgraveSealResolved(TEXT("gloaming.ashgrave_extraction_seal_resolved"));
    const FName ArrivalReceipt(TEXT("gloaming.arrival.recorded"));
    const FName AshgraveSealReceipt(TEXT("gloaming.ashgrave_extraction_seal.resolved"));
    const FName MalvaineResolved(TEXT("gloaming.malvaine_encounter_resolved"));
    const FName MalvaineResolvedReceipt(TEXT("gloaming.malvaine.encounter_resolved"));
    const FName HollowTwinsResolved(TEXT("gloaming.hollow_twins_resolved"));
    const FName HollowTwinsResolvedReceipt(TEXT("gloaming.hollow_twins.encounter_resolved"));
    const FName SanguineStrikeUnlocked(TEXT("echo.sanguine_strike"));
    const FName SanguineStrikeUnlockReceipt(TEXT("gloaming.malvaine.sanguine_strike_manifested"));
    const FName SecondTurnUnlocked(TEXT("echo.second_turn"));
    const FName SecondTurnUnlockReceipt(TEXT("gloaming.hollow_twins.second_turn_manifested"));
    const FName MichaelMireResolved(TEXT("gloaming.michael_mire_resolved"));
    const FName MichaelMireResolvedReceipt(TEXT("gloaming.michael_mire.living_submission"));
    const FName MacheteMasonResolved(TEXT("gloaming.machete_mason_resolved"));
    const FName MacheteMasonResolvedReceipt(TEXT("gloaming.machete_mason.disarmed_submission"));
    const FName PleatherfaceResolved(TEXT("gloaming.pleatherface_resolved"));
    const FName PleatherfaceResolvedReceipt(TEXT("gloaming.pleatherface.disarmed_submission"));
    const FName WherewolfResolved(TEXT("gloaming.wherewolf_resolved"));
    const FName WherewolfResolvedReceipt(TEXT("gloaming.wherewolf.calmed_submission"));
    const FName AnnieWailsResolved(TEXT("gloaming.annie_wails_resolved"));
    const FName AnnieWailsResolvedReceipt(TEXT("gloaming.annie_wails.disarmed_surrender"));
}

void UWyrmGloamingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ResetGloamingState();
}

UWyrmGloamingSubsystem* UWyrmGloamingSubsystem::GetGloamingSubsystem(UObject* WorldContextObject)
{
    if (!WorldContextObject || !GEngine)
    {
        return nullptr;
    }
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            return GameInstance->GetSubsystem<UWyrmGloamingSubsystem>();
        }
    }
    return nullptr;
}

void UWyrmGloamingSubsystem::ResetGloamingState()
{
    KnownFacts.Reset();
    FactReceipts.Reset();
}

bool UWyrmGloamingSubsystem::RecordArrival()
{
    return CommitFact(WyrmGloamingFacts::Arrival, WyrmGloamingFacts::ArrivalReceipt);
}

bool UWyrmGloamingSubsystem::ResolveAshgraveExtractionSeal()
{
    if (!HasFact(WyrmGloamingFacts::Arrival))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::AshgraveSealResolved, WyrmGloamingFacts::AshgraveSealReceipt);
}

bool UWyrmGloamingSubsystem::RecordMalvaineResolution(bool bParley)
{
    if (!HasFact(WyrmGloamingFacts::AshgraveSealResolved) ||
        HasFact(WyrmGloamingFacts::MalvaineResolved) ||
        HasReceipt(WyrmGloamingFacts::MalvaineResolvedReceipt))
    {
        return false;
    }
    const FName RouteFact = bParley
        ? FName(TEXT("gloaming.malvaine_parley"))
        : FName(TEXT("gloaming.malvaine_living_defeat"));
    const FName RouteReceipt = bParley
        ? FName(TEXT("gloaming.malvaine.parley"))
        : FName(TEXT("gloaming.malvaine.living_defeat"));
    if (HasReceipt(RouteReceipt) || !CommitFact(RouteFact, RouteReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::MalvaineResolved, WyrmGloamingFacts::MalvaineResolvedReceipt);
}

bool UWyrmGloamingSubsystem::RecordHollowTwinsResolution(bool bReleased)
{
    if (!HasFact(WyrmGloamingFacts::MalvaineResolved) ||
        HasFact(WyrmGloamingFacts::HollowTwinsResolved) ||
        HasReceipt(WyrmGloamingFacts::HollowTwinsResolvedReceipt))
    {
        return false;
    }
    const FName RouteFact = bReleased
        ? FName(TEXT("gloaming.hollow_twins_released"))
        : FName(TEXT("gloaming.hollow_twins_living_submission"));
    const FName RouteReceipt = bReleased
        ? FName(TEXT("gloaming.hollow_twins.released"))
        : FName(TEXT("gloaming.hollow_twins.living_submission"));
    if (HasReceipt(RouteReceipt) || !CommitFact(RouteFact, RouteReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::HollowTwinsResolved, WyrmGloamingFacts::HollowTwinsResolvedReceipt);
}

bool UWyrmGloamingSubsystem::RecordSanguineStrikeUnlock()
{
    if (!HasFact(WyrmGloamingFacts::MalvaineResolved) ||
        HasFact(WyrmGloamingFacts::SanguineStrikeUnlocked) ||
        HasReceipt(WyrmGloamingFacts::SanguineStrikeUnlockReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::SanguineStrikeUnlocked, WyrmGloamingFacts::SanguineStrikeUnlockReceipt);
}

bool UWyrmGloamingSubsystem::RecordMichaelMireResolution()
{
    if (!HasFact(WyrmGloamingFacts::HollowTwinsResolved) ||
        HasFact(WyrmGloamingFacts::MichaelMireResolved) ||
        HasReceipt(WyrmGloamingFacts::MichaelMireResolvedReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::MichaelMireResolved, WyrmGloamingFacts::MichaelMireResolvedReceipt);
}

bool UWyrmGloamingSubsystem::RecordMacheteMasonResolution()
{
    if (!HasFact(WyrmGloamingFacts::MichaelMireResolved) ||
        HasFact(WyrmGloamingFacts::MacheteMasonResolved) ||
        HasReceipt(WyrmGloamingFacts::MacheteMasonResolvedReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::MacheteMasonResolved, WyrmGloamingFacts::MacheteMasonResolvedReceipt);
}

bool UWyrmGloamingSubsystem::RecordPleatherfaceResolution()
{
    if (!HasFact(WyrmGloamingFacts::MacheteMasonResolved) ||
        HasFact(WyrmGloamingFacts::PleatherfaceResolved) ||
        HasReceipt(WyrmGloamingFacts::PleatherfaceResolvedReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::PleatherfaceResolved, WyrmGloamingFacts::PleatherfaceResolvedReceipt);
}

bool UWyrmGloamingSubsystem::RecordWherewolfResolution()
{
    if (!HasFact(WyrmGloamingFacts::PleatherfaceResolved) ||
        HasFact(WyrmGloamingFacts::WherewolfResolved) ||
        HasReceipt(WyrmGloamingFacts::WherewolfResolvedReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::WherewolfResolved, WyrmGloamingFacts::WherewolfResolvedReceipt);
}

bool UWyrmGloamingSubsystem::RecordAnnieWailsResolution()
{
    if (!HasFact(WyrmGloamingFacts::WherewolfResolved) ||
        HasFact(WyrmGloamingFacts::AnnieWailsResolved) ||
        HasReceipt(WyrmGloamingFacts::AnnieWailsResolvedReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::AnnieWailsResolved, WyrmGloamingFacts::AnnieWailsResolvedReceipt);
}

bool UWyrmGloamingSubsystem::RecordSecondTurnUnlock()
{
    if (!HasFact(WyrmGloamingFacts::HollowTwinsResolved) ||
        HasFact(WyrmGloamingFacts::SecondTurnUnlocked) ||
        HasReceipt(WyrmGloamingFacts::SecondTurnUnlockReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::SecondTurnUnlocked, WyrmGloamingFacts::SecondTurnUnlockReceipt);
}

bool UWyrmGloamingSubsystem::CommitFact(FName FactId, FName ReceiptId)
{
    if (FactId.IsNone() || ReceiptId.IsNone() || HasReceipt(ReceiptId))
    {
        return false;
    }
    KnownFacts.AddUnique(FactId);
    FactReceipts.Add(ReceiptId);
    return true;
}

bool UWyrmGloamingSubsystem::HasFact(FName FactId) const
{
    return KnownFacts.Contains(FactId);
}

bool UWyrmGloamingSubsystem::HasReceipt(FName ReceiptId) const
{
    return FactReceipts.Contains(ReceiptId);
}

void UWyrmGloamingSubsystem::BuildSaveRecord(FWyrmGloamingSaveRecord& OutRecord) const
{
    OutRecord.KnownFacts = KnownFacts;
    OutRecord.FactReceipts = FactReceipts;
}

void UWyrmGloamingSubsystem::RestoreFromSaveRecord(const FWyrmGloamingSaveRecord& InRecord)
{
    KnownFacts = InRecord.KnownFacts;
    FactReceipts = InRecord.FactReceipts;
    NormalizeRestoredState();
}

void UWyrmGloamingSubsystem::NormalizeRestoredState()
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
}
