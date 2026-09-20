#include "Region/WyrmGloamingSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Dragon/WyrmDragonCharacter.h"
#include "Region/WyrmWorldTravelSubsystem.h"
#include "Algo/AllOf.h"

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
    const FName ScarrieResolved(TEXT("gloaming.scarrie_resolved"));
    const FName ScarrieResolvedReceipt(TEXT("gloaming.scarrie.living_submission"));
    const FName ChucklesResolved(TEXT("gloaming.chuckles_resolved"));
    const FName ChucklesResolvedReceipt(TEXT("gloaming.chuckles.contained_submission"));
    const FName CountDripulaResolved(TEXT("gloaming.count_dripula_resolved"));
    const FName CountDripulaResolvedReceipt(TEXT("gloaming.count_dripula.bloodless_surrender"));
    const FName FrankNShrineResolved(TEXT("gloaming.frank_n_shrine_resolved"));
    const FName FrankNShrineResolvedReceipt(TEXT("gloaming.frank_n_shrine.grounded_submission"));
    const FName FinalRequiredHorrorResolved(TEXT("gloaming.pyre_midhead_resolved"));
    const FName RegionComplete(TEXT("gloaming.region_complete"));
    const FName RegionCompleteReceipt(TEXT("gloaming.region.completion_committed"));

    struct FRequiredHorrorFact
    {
        FName Prerequisite;
        FName Resolved;
        FName Receipt;
    };

    const FRequiredHorrorFact* GetRequiredHorrorFact(EWyrmRequiredHorrorIdentity Identity)
    {
        static const FRequiredHorrorFact Facts[] = {
            {FrankNShrineResolved, FName(TEXT("gloaming.ail_yen_resolved")), FName(TEXT("gloaming.ail_yen.living_submission"))},
            {FName(TEXT("gloaming.ail_yen_resolved")), FName(TEXT("gloaming.bellraiser_resolved")), FName(TEXT("gloaming.bellraiser.living_submission"))},
            {FName(TEXT("gloaming.bellraiser_resolved")), FName(TEXT("gloaming.sad_echo_resolved")), FName(TEXT("gloaming.sad_echo.living_submission"))},
            {FName(TEXT("gloaming.sad_echo_resolved")), FName(TEXT("gloaming.dreadator_resolved")), FName(TEXT("gloaming.dreadator.living_submission"))},
            {FName(TEXT("gloaming.dreadator_resolved")), FName(TEXT("gloaming.roastface_resolved")), FName(TEXT("gloaming.roastface.disarmed_submission"))},
            {FName(TEXT("gloaming.roastface_resolved")), FName(TEXT("gloaming.gravy_daughters_resolved")), FName(TEXT("gloaming.gravy_daughters.paired_submission"))},
            {FName(TEXT("gloaming.gravy_daughters_resolved")), FName(TEXT("gloaming.knit_resolved")), FName(TEXT("gloaming.knit.balloon_surrender"))},
            {FName(TEXT("gloaming.knit_resolved")), FName(TEXT("gloaming.canniball_resolved")), FName(TEXT("gloaming.canniball.restrained_submission"))},
            {FName(TEXT("gloaming.canniball_resolved")), FName(TEXT("gloaming.mums_the_wyrd_resolved")), FName(TEXT("gloaming.mums_the_wyrd.unbound_submission"))},
            {FName(TEXT("gloaming.mums_the_wyrd_resolved")), FName(TEXT("gloaming.dready_freddie_resolved")), FName(TEXT("gloaming.dready_freddie.waking_submission"))},
            {FName(TEXT("gloaming.dready_freddie_resolved")), FName(TEXT("gloaming.pyre_midhead_resolved")), FName(TEXT("gloaming.pyre_midhead.disarmed_submission"))},
        };
        const uint8 Index = static_cast<uint8>(Identity);
        return Index < UE_ARRAY_COUNT(Facts) ? &Facts[Index] : nullptr;
    }
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

bool UWyrmGloamingSubsystem::RecordScarrieResolution()
{
    if (!HasFact(WyrmGloamingFacts::AnnieWailsResolved) ||
        HasFact(WyrmGloamingFacts::ScarrieResolved) ||
        HasReceipt(WyrmGloamingFacts::ScarrieResolvedReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::ScarrieResolved, WyrmGloamingFacts::ScarrieResolvedReceipt);
}

bool UWyrmGloamingSubsystem::RecordChucklesResolution()
{
    if (!HasFact(WyrmGloamingFacts::ScarrieResolved) ||
        HasFact(WyrmGloamingFacts::ChucklesResolved) ||
        HasReceipt(WyrmGloamingFacts::ChucklesResolvedReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::ChucklesResolved, WyrmGloamingFacts::ChucklesResolvedReceipt);
}

bool UWyrmGloamingSubsystem::RecordCountDripulaResolution()
{
    if (!HasFact(WyrmGloamingFacts::ChucklesResolved) ||
        HasFact(WyrmGloamingFacts::CountDripulaResolved) ||
        HasReceipt(WyrmGloamingFacts::CountDripulaResolvedReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::CountDripulaResolved, WyrmGloamingFacts::CountDripulaResolvedReceipt);
}

bool UWyrmGloamingSubsystem::RecordFrankNShrineResolution()
{
    if (!HasFact(WyrmGloamingFacts::CountDripulaResolved) ||
        HasFact(WyrmGloamingFacts::FrankNShrineResolved) ||
        HasReceipt(WyrmGloamingFacts::FrankNShrineResolvedReceipt))
    {
        return false;
    }
    return CommitFact(WyrmGloamingFacts::FrankNShrineResolved, WyrmGloamingFacts::FrankNShrineResolvedReceipt);
}

bool UWyrmGloamingSubsystem::CanResolveRequiredHorror(EWyrmRequiredHorrorIdentity Identity) const
{
    const WyrmGloamingFacts::FRequiredHorrorFact* Fact = WyrmGloamingFacts::GetRequiredHorrorFact(Identity);
    return Fact && HasFact(Fact->Prerequisite) && !HasFact(Fact->Resolved) && !HasReceipt(Fact->Receipt);
}

bool UWyrmGloamingSubsystem::RecordRequiredHorrorResolution(EWyrmRequiredHorrorIdentity Identity)
{
    if (!CanResolveRequiredHorror(Identity))
    {
        return false;
    }
    const WyrmGloamingFacts::FRequiredHorrorFact* Fact = WyrmGloamingFacts::GetRequiredHorrorFact(Identity);
    return Fact && CommitFact(Fact->Resolved, Fact->Receipt);
}

bool UWyrmGloamingSubsystem::CanCompleteRegion(
    const AWyrmDragonCharacter* Nyxaroth, const UWyrmWorldTravelSubsystem* Travel) const
{
    static const FName GloamingMarches(TEXT("GloamingMarches"));
    static const FName Region01(TEXT("Region01"));
    static const FName GloamingArrival(TEXT("LM-GLOAMING-ARRIVAL"));
    static const FName RequiredFacts[] = {
        WyrmGloamingFacts::MichaelMireResolved, WyrmGloamingFacts::MacheteMasonResolved,
        WyrmGloamingFacts::PleatherfaceResolved, WyrmGloamingFacts::WherewolfResolved,
        WyrmGloamingFacts::AnnieWailsResolved, WyrmGloamingFacts::ScarrieResolved,
        WyrmGloamingFacts::ChucklesResolved, WyrmGloamingFacts::CountDripulaResolved,
        WyrmGloamingFacts::FrankNShrineResolved,
        FName(TEXT("gloaming.ail_yen_resolved")), FName(TEXT("gloaming.bellraiser_resolved")),
        FName(TEXT("gloaming.sad_echo_resolved")), FName(TEXT("gloaming.dreadator_resolved")),
        FName(TEXT("gloaming.roastface_resolved")), FName(TEXT("gloaming.gravy_daughters_resolved")),
        FName(TEXT("gloaming.knit_resolved")), FName(TEXT("gloaming.canniball_resolved")),
        FName(TEXT("gloaming.mums_the_wyrd_resolved")), FName(TEXT("gloaming.dready_freddie_resolved")),
        WyrmGloamingFacts::FinalRequiredHorrorResolved};
    const bool bHasEveryRequiredHorror = Algo::AllOf(RequiredFacts,
        [this](FName Fact) { return HasFact(Fact); });
    return bHasEveryRequiredHorror &&
        !HasFact(WyrmGloamingFacts::RegionComplete) &&
        !HasReceipt(WyrmGloamingFacts::RegionCompleteReceipt) &&
        Nyxaroth && Nyxaroth->DragonId == FName(TEXT("Nyxaroth")) &&
        Nyxaroth->HasSupportedRigProfile() && Nyxaroth->HasBondReceipt() &&
        Nyxaroth->GetDragonRole() == EWyrmDragonRole::AlliedCompanion &&
        Travel && Travel->GetCurrentRegionId() == GloamingMarches &&
        Travel->GetArrivalLandmarkId() == GloamingArrival &&
        Travel->IsAllowedRoute(GloamingMarches, Region01);
}

bool UWyrmGloamingSubsystem::RecordRegionalCompletion(
    AWyrmDragonCharacter* Nyxaroth, UWyrmWorldTravelSubsystem* Travel)
{
    return CanCompleteRegion(Nyxaroth, Travel) &&
        CommitFact(WyrmGloamingFacts::RegionComplete, WyrmGloamingFacts::RegionCompleteReceipt);
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
