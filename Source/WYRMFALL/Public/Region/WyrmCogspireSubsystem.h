#pragma once

#include "CoreMinimal.h"
#include "Region/WyrmCogspireTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WyrmCogspireSubsystem.generated.h"

class AWyrmCharacter;
class AWyrmDragonCharacter;
class UWyrmWorldTravelSubsystem;

/** Bounded Cogspire fact owner. UWyrmSaveSubsystem remains the sole serializer and slot owner. */
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

    UFUNCTION(BlueprintCallable, Category="Cogspire|Cogfang")
    bool RecordCogfangEncounterStarted(AWyrmDragonCharacter* Cogfang);

    UFUNCTION(BlueprintCallable, Category="Cogspire|Cogfang")
    bool RecordCogfangLivingDefeat(AWyrmDragonCharacter* Cogfang);

    UFUNCTION(BlueprintCallable, Category="Cogspire|Engine")
    bool ShutdownCoercionGovernor(AWyrmDragonCharacter* Cogfang);

    UFUNCTION(BlueprintCallable, Category="Cogspire|Cogfang")
    bool BondCogfang(AWyrmDragonCharacter* Cogfang, AWyrmCharacter* Humanoid);

    UFUNCTION(BlueprintPure, Category="Cogspire|Completion")
    bool CanCompleteRegion(const AWyrmDragonCharacter* Cogfang, const UWyrmWorldTravelSubsystem* Travel) const;

    UFUNCTION(BlueprintCallable, Category="Cogspire|Completion")
    bool RecordRegionalCompletion(AWyrmDragonCharacter* Cogfang, UWyrmWorldTravelSubsystem* Travel);

    UFUNCTION(BlueprintPure, Category="Cogspire|Engine")
    bool IsCoercionGovernorActive() const { return bCoercionGovernorActive; }

    UFUNCTION(BlueprintPure, Category="Cogspire|Engine")
    bool IsCivicMachineryOperational() const { return bCivicMachineryOperational; }

    UFUNCTION(BlueprintPure, Category="Cogspire|Facts")
    bool HasFact(FName FactId) const;

    UFUNCTION(BlueprintPure, Category="Cogspire|Facts")
    bool HasReceipt(FName ReceiptId) const;

    void BuildSaveRecord(FWyrmCogspireSaveRecord& OutRecord) const;
    void RestoreFromSaveRecord(const FWyrmCogspireSaveRecord& InRecord);

private:
    bool CommitFact(FName FactId, FName ReceiptId);
    void NormalizeRestoredState();

    UPROPERTY(VisibleInstanceOnly, Category="Cogspire")
    TArray<FName> KnownFacts;

    UPROPERTY(VisibleInstanceOnly, Category="Cogspire")
    TArray<FName> FactReceipts;

    UPROPERTY(VisibleInstanceOnly, Category="Cogspire|Engine")
    bool bCoercionGovernorActive = true;

    UPROPERTY(VisibleInstanceOnly, Category="Cogspire|Engine")
    bool bCivicMachineryOperational = true;
};
