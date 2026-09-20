#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WyrmCogspireSubsystem.generated.h"

/** Bounded runtime-only Cogspire fact owner. Persistence is intentionally out of scope. */
UCLASS(BlueprintType, Category="WYRMFALL|Cogspire")
class WYRMFALL_API UWyrmCogspireSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintPure, Category="Cogspire", meta=(WorldContext="WorldContextObject"))
    static UWyrmCogspireSubsystem* GetCogspireSubsystem(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="Cogspire|Facts")
    void ResetCogspireState();

    UFUNCTION(BlueprintCallable, Category="Cogspire|Arrival")
    bool RecordArrival();

    UFUNCTION(BlueprintCallable, Category="Cogspire|Observation")
    bool RecordPublicMachineryObserved();

    UFUNCTION(BlueprintCallable, Category="Cogspire|Observation")
    bool RecordCoercionDiversionObserved();

    UFUNCTION(BlueprintCallable, Category="Cogspire|Baron")
    bool RecordBaronAcknowledgment();

    UFUNCTION(BlueprintPure, Category="Cogspire|Facts")
    bool HasFact(FName FactId) const;

    UFUNCTION(BlueprintPure, Category="Cogspire|Facts")
    bool HasReceipt(FName ReceiptId) const;

private:
    bool CommitFact(FName FactId, FName ReceiptId);

    UPROPERTY(VisibleInstanceOnly, Category="Cogspire")
    TArray<FName> KnownFacts;

    UPROPERTY(VisibleInstanceOnly, Category="Cogspire")
    TArray<FName> FactReceipts;
};
