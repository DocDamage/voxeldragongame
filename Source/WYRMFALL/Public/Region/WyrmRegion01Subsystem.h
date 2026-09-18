#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Region/WyrmRegion01Types.h"
#include "WyrmRegion01Subsystem.generated.h"

class AWyrmDragonCharacter;

/**
 * Region 01's narrowly-scoped fact ledger. It coordinates no persistence on
 * its own: UWyrmSaveSubsystem remains the authoritative save owner.
 */
UCLASS(BlueprintType, Category="WYRMFALL|Region01")
class WYRMFALL_API UWyrmRegion01Subsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** Engine-side lookup for Blueprint/Python callers where GameInstance has no reflected generic accessor. */
    UFUNCTION(BlueprintPure, Category="Region 01", meta=(WorldContext="WorldContextObject"))
    static UWyrmRegion01Subsystem* GetRegion01Subsystem(UObject* WorldContextObject);

    /** Test/new-session reset. Production flows should only add facts. */
    UFUNCTION(BlueprintCallable, Category="Region 01|Facts")
    void ResetRegion01State();

    /** Returns the authored Region 01 relationship graph, not map-placement proof. */
    UFUNCTION(BlueprintPure, Category="Region 01|Landmarks")
    TArray<FWyrmRegion01LandmarkDefinition> GetLandmarkDefinitions() const;

    UFUNCTION(BlueprintPure, Category="Region 01|Landmarks")
    bool HasValidLandmarkGraph() const;

    UFUNCTION(BlueprintPure, Category="Region 01|Landmarks")
    bool IsKnownLandmark(FName LandmarkId) const;

    /** Logical routes exist independently of the fact that makes an optional site available. */
    UFUNCTION(BlueprintPure, Category="Region 01|Landmarks")
    bool IsLandmarkCurrentlyAvailable(FName LandmarkId) const;

    /** True when the graph supplies a route; it does not prescribe trigger order. */
    UFUNCTION(BlueprintPure, Category="Region 01|Landmarks")
    bool HasRouteBetweenLandmarks(FName FromLandmark, FName ToLandmark) const;

    /** Records an authored landmark visit once; no road/corridor prerequisite is imposed. */
    UFUNCTION(BlueprintCallable, Category="Region 01|Landmarks")
    bool VisitLandmark(FName LandmarkId);

    UFUNCTION(BlueprintPure, Category="Region 01|Landmarks")
    bool HasVisitedLandmark(FName LandmarkId) const;

    UFUNCTION(BlueprintCallable, Category="Region 01|Facts")
    bool RecordHeartExitReached();

    UFUNCTION(BlueprintCallable, Category="Region 01|Facts")
    bool ReadCrownNotice();

    /** Every worker can be secured in any order, including Sella before Tidecross. */
    UFUNCTION(BlueprintCallable, Category="Region 01|Workers")
    bool SecureWorker(EWyrmRegion01Worker Worker);

    UFUNCTION(BlueprintCallable, Category="Region 01|Evidence")
    bool DiscoverEvidence(EWyrmRegion01Evidence Evidence);

    /** Sella is the only prerequisite for the optional restraint shutdown. */
    UFUNCTION(BlueprintCallable, Category="Region 01|Evidence")
    bool DisableAuxiliaryRestraint();

    /**
     * Records a custody outcome without forcing an encounter before the
     * player reaches the arena. Surrendered custody is a post-bond branch.
     */
    UFUNCTION(BlueprintCallable, Category="Region 01|Rusk")
    bool ResolveRusk(EWyrmRegion01RuskOutcome DesiredOutcome);

    UFUNCTION(BlueprintCallable, Category="Region 01|Quarry")
    bool StopCrowncutExtraction();

    UFUNCTION(BlueprintCallable, Category="Region 01|Verdance")
    bool RecordVerdanceDefeatedAlive();

    UFUNCTION(BlueprintCallable, Category="Region 01|Verdance")
    bool BreakVerdanceClaim();

    /** Requires the existing dragon authority to have issued its real bond receipt. */
    UFUNCTION(BlueprintCallable, Category="Region 01|Verdance")
    bool RecordVerdanceBondAccepted(AWyrmDragonCharacter* Verdance);

    UFUNCTION(BlueprintCallable, Category="Region 01|Quarry")
    bool RecordReliefResolved();

    /** Optional restitution is one receipt and never starts a town-economy loop. */
    UFUNCTION(BlueprintCallable, Category="Region 01|Quarry")
    bool RecoverOptionalWageRecord();

    /** Service cache in compact cave ('A Smaller Kind of Strength') commits cache restitution and route unlock. */
    UFUNCTION(BlueprintCallable, Category="Region 01|Cave")
    bool RecoverServiceCache();

    UFUNCTION(BlueprintPure, Category="Region 01|Cave")
    bool IsServiceCacheRecovered() const;

    UFUNCTION(BlueprintPure, Category="Region 01|Facts")
    bool HasFact(FName FactId) const;

    UFUNCTION(BlueprintPure, Category="Region 01|Facts")
    bool HasConflictEvidence() const;

    UFUNCTION(BlueprintPure, Category="Region 01|Facts")
    bool IsAuxiliaryShutdownAvailable() const;

    UFUNCTION(BlueprintPure, Category="Region 01|Facts")
    bool IsBondedDragonAvailable() const;

    UFUNCTION(BlueprintPure, Category="Region 01|Facts")
    bool IsPartialDebriefAvailable() const;

    /** All local closure facts are present; the hub still records the actual homecoming event. */
    UFUNCTION(BlueprintPure, Category="Region 01|Facts")
    bool IsHomecomingReady() const;

    /** Commits homecoming only at Tidecross after every local closure fact is true. */
    UFUNCTION(BlueprintCallable, Category="Region 01|Facts")
    bool CompleteHomecoming();

    UFUNCTION(BlueprintPure, Category="Region 01|Facts")
    bool IsHomecomingComplete() const;

    UFUNCTION(BlueprintPure, Category="Region 01|Facts")
    EWyrmRegion01RuskOutcome GetRuskOutcome() const { return State.RuskOutcome; }

    UFUNCTION(BlueprintCallable, Category="Region 01|Echo")
    bool RecordEchoRelentlessAdvance();

    UFUNCTION(BlueprintPure, Category="Region 01|Echo")
    bool IsEchoRelentlessAdvanceUnlocked() const;

    UFUNCTION(BlueprintCallable, Category="Region 01|Facts")
    bool CommitFact(FName FactId, FName ReceiptId);

    /** Called by the unified save owner; this subsystem never writes slots itself. */
    void BuildSaveRecord(FWyrmRegion01SaveRecord& OutRecord) const;
    void RestoreFromSaveRecord(const FWyrmRegion01SaveRecord& InRecord);

private:
    void EnsureDerivedFact(FName FactId, FName ReceiptId);
    bool HasReceipt(FName ReceiptId) const;
    void RefreshDerivedFacts();
    void NormalizeRestoredState();
    static const TArray<FWyrmRegion01LandmarkDefinition>& GetDefinitions();
    static FName GetWorkerFact(EWyrmRegion01Worker Worker);
    static FName GetWorkerReceipt(EWyrmRegion01Worker Worker);
    static FName GetEvidenceFact(EWyrmRegion01Evidence Evidence);
    static FName GetEvidenceReceipt(EWyrmRegion01Evidence Evidence);

    UPROPERTY(VisibleInstanceOnly, Category="Region 01")
    FWyrmRegion01SaveRecord State;
};
