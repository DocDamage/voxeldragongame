#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Save/WyrmSaveGame.h"
#include "WyrmWorldTravelSubsystem.generated.h"

class AWyrmCharacter;

/** Allowlisted regional route authority. Persistence stays in UWyrmSaveSubsystem. */
UCLASS(BlueprintType, Category="WYRMFALL|Travel")
class WYRMFALL_API UWyrmWorldTravelSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintPure, Category="Travel", meta=(WorldContext="WorldContextObject"))
    static UWyrmWorldTravelSubsystem* GetWorldTravelSubsystem(UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category="Travel")
    static FName RegionIdForMapName(const FString& MapName);

    UFUNCTION(BlueprintPure, Category="Travel")
    bool IsAllowedRoute(FName FromRegionId, FName ToRegionId) const;

    /** Records a validated departure. Call OpenPreparedDestination to perform the map change. */
    UFUNCTION(BlueprintCallable, Category="Travel")
    bool PrepareTravel(FName FromRegionId, FName ToRegionId, FName ReturnLandmarkId);

    /** Safe departure path: validates transient state and writes the recovery snapshot before mutation. */
    UFUNCTION(BlueprintCallable, Category="Travel")
    bool PrepareTravelWithSnapshot(FName FromRegionId, FName ToRegionId, FName ReturnLandmarkId,
        const FString& RecoverySlotName, AWyrmCharacter* Character, AActor* TerrainProviderActor = nullptr);

    UFUNCTION(BlueprintCallable, Category="Travel")
    bool OpenPreparedDestination();

    UFUNCTION(BlueprintCallable, Category="Travel")
    bool CompleteArrival(FName RegionId, FName ArrivalLandmarkId);

    /** Starts fail-safe recovery; opens the recorded source map when necessary. */
    UFUNCTION(BlueprintCallable, Category="Travel")
    bool BeginTravelRecovery(const FString& RecoverySlotName);

    /** Applies the unified snapshot after the recorded source map is active. */
    UFUNCTION(BlueprintCallable, Category="Travel")
    bool CompleteTravelRecovery(AWyrmCharacter* Character, AActor* TerrainProviderActor = nullptr);

    UFUNCTION(BlueprintPure, Category="Travel")
    FString GetLastTravelFailureReason() const { return LastTravelFailureReason; }

    UFUNCTION(BlueprintPure, Category="Travel")
    FName GetCurrentRegionId() const { return State.CurrentRegionId; }

    UFUNCTION(BlueprintPure, Category="Travel")
    FName GetArrivalLandmarkId() const { return State.ArrivalLandmarkId; }

    void BuildSaveRecord(FWyrmWorldTravelSaveRecord& OutRecord) const { OutRecord = State; }
    void RestoreFromSaveRecord(const FWyrmWorldTravelSaveRecord& InRecord);

private:
    static FName DestinationMapForRegion(FName RegionId);
    static FName DefaultArrivalForRegion(FName RegionId);
    static FName ExpectedReturnLandmark(FName FromRegionId, FName ToRegionId);

    UPROPERTY(VisibleInstanceOnly, Category="Travel")
    FWyrmWorldTravelSaveRecord State;

    UPROPERTY(VisibleInstanceOnly, Category="Travel")
    FName PreparedDestinationRegion = NAME_None;

    UPROPERTY(VisibleInstanceOnly, Category="Travel")
    FString LastTravelFailureReason;

    UPROPERTY(VisibleInstanceOnly, Category="Travel")
    FString PendingRecoverySlotName;

    UPROPERTY(VisibleInstanceOnly, Category="Travel")
    FName PendingRecoveryRegion = NAME_None;
};
