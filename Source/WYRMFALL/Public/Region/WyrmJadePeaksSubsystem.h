#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Region/WyrmJadePeaksTypes.h"
#include "WyrmJadePeaksSubsystem.generated.h"

/** Jade Peaks facts and landmark graph only; UWyrmSaveSubsystem owns persistence. */
UCLASS(BlueprintType, Category="WYRMFALL|JadePeaks")
class WYRMFALL_API UWyrmJadePeaksSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintPure, Category="Jade Peaks", meta=(WorldContext="WorldContextObject"))
    static UWyrmJadePeaksSubsystem* GetJadePeaksSubsystem(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Facts")
    void ResetJadePeaksState();

    UFUNCTION(BlueprintPure, Category="Jade Peaks|Landmarks")
    TArray<FWyrmJadePeaksLandmarkDefinition> GetLandmarkDefinitions() const;

    UFUNCTION(BlueprintPure, Category="Jade Peaks|Landmarks")
    bool HasValidLandmarkGraph() const;

    UFUNCTION(BlueprintPure, Category="Jade Peaks|Landmarks")
    bool IsKnownLandmark(FName LandmarkId) const;

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Landmarks")
    bool VisitLandmark(FName LandmarkId);

    UFUNCTION(BlueprintPure, Category="Jade Peaks|Landmarks")
    bool HasVisitedLandmark(FName LandmarkId) const;

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Facts")
    bool RecordJadefangArrival();

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Facts")
    bool RecordDiscipleResolution(bool bTrustedResolution);

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Facts")
    bool RecordMirrorStepUnlock();

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Facts")
    bool RecordUnseenHandUnlock();

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Facts")
    bool RecordImperialPactResolution(bool bDiplomacy);

    UFUNCTION(BlueprintPure, Category="Jade Peaks|Facts")
    bool IsRegionalClosureComplete() const;

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Facts")
    bool RecordReturnRouteReady();

    UFUNCTION(BlueprintCallable, Category="Jade Peaks|Facts")
    bool CommitFact(FName FactId, FName ReceiptId);

    UFUNCTION(BlueprintPure, Category="Jade Peaks|Facts")
    bool HasFact(FName FactId) const;

    void BuildSaveRecord(FWyrmJadePeaksSaveRecord& OutRecord) const;
    void RestoreFromSaveRecord(const FWyrmJadePeaksSaveRecord& InRecord);

private:
    static const TArray<FWyrmJadePeaksLandmarkDefinition>& GetDefinitions();
    bool HasReceipt(FName ReceiptId) const;
    void NormalizeRestoredState();

    UPROPERTY(VisibleInstanceOnly, Category="Jade Peaks")
    FWyrmJadePeaksSaveRecord State;
};
