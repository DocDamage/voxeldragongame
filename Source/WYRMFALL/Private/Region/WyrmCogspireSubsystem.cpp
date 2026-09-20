#include "Region/WyrmCogspireSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

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
