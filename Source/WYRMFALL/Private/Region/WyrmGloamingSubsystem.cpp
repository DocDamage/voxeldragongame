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
