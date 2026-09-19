#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WyrmGloamingSubsystem.generated.h"

/** Bounded Gloaming Marches fact owner; persistence remains out of scope for this slice. */
UCLASS(BlueprintType, Category="WYRMFALL|Gloaming")
class WYRMFALL_API UWyrmGloamingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintPure, Category="Gloaming", meta=(WorldContext="WorldContextObject"))
    static UWyrmGloamingSubsystem* GetGloamingSubsystem(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="Gloaming|Facts")
    void ResetGloamingState();

    UFUNCTION(BlueprintCallable, Category="Gloaming|Arrival")
    bool RecordArrival();

    UFUNCTION(BlueprintCallable, Category="Gloaming|Ashgrave")
    bool ResolveAshgraveExtractionSeal();

    UFUNCTION(BlueprintCallable, Category="Gloaming|Malvaine")
    bool RecordMalvaineResolution(bool bParley);

    UFUNCTION(BlueprintPure, Category="Gloaming|Facts")
    bool HasFact(FName FactId) const;

    UFUNCTION(BlueprintPure, Category="Gloaming|Facts")
    bool HasReceipt(FName ReceiptId) const;

private:
    bool CommitFact(FName FactId, FName ReceiptId);

    UPROPERTY(VisibleInstanceOnly, Category="Gloaming")
    TArray<FName> KnownFacts;

    UPROPERTY(VisibleInstanceOnly, Category="Gloaming")
    TArray<FName> FactReceipts;
};
