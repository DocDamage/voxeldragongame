#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Terrain/WyrmTerrainProvider.h"
#include "WyrmGeoForgeAdapter.generated.h"

class AGeoForgeInfiniteTerrainActor;

/**
 * Authoritative project adapter for GeoForgeRuntime terrain.
 * Implements IWyrmTerrainProvider with verified collision & navigation completion,
 * occupied-fill protection, finite resource yield transactions, and save serialization.
 */
UCLASS(BlueprintType, Blueprintable, Category="WYRMFALL|Terrain")
class WYRMFALL_API AWyrmGeoForgeAdapter : public AActor, public IWyrmTerrainProvider
{
    GENERATED_BODY()

public:
    AWyrmGeoForgeAdapter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    /** Bound GeoForge terrain actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
    TWeakObjectPtr<AGeoForgeInfiniteTerrainActor> TerrainActor;

    /** Fired when an edit request completes */
    UPROPERTY(BlueprintAssignable, Category="Terrain|Events")
    FWyrmOnTerrainEditCompleted OnTerrainEditCompleted;

    /** Fired when collision geometry is confirmed ready */
    UPROPERTY(BlueprintAssignable, Category="Terrain|Events")
    FWyrmOnTerrainCollisionReady OnTerrainCollisionReady;

    /** Fired when navigation surface is confirmed ready */
    UPROPERTY(BlueprintAssignable, Category="Terrain|Events")
    FWyrmOnTerrainNavReady OnTerrainNavReady;

    /** Binds a GeoForge terrain actor */
    UFUNCTION(BlueprintCallable, Category="Terrain")
    void BindTerrainActor(AGeoForgeInfiniteTerrainActor* InActor);

    // --- IWyrmTerrainProvider interface ---
    virtual FWyrmTerrainCapabilities GetTerrainCapabilities_Implementation() const override;
    virtual EWyrmTerrainSubmitResult SubmitTerrainEdit_Implementation(const FWyrmTerrainEditRequest& Request) override;
    virtual bool IsVolumeOccupied_Implementation(const FVector& Center, float RadiusCm) const override;
    virtual bool GetLastYield_Implementation(const FGuid& ActionId, FWyrmVoxelYield& OutYield) const override;

    // --- Direct invocation helpers ---
    UFUNCTION(BlueprintCallable, Category="Terrain")
    FWyrmTerrainCapabilities GetCapabilities() const { return GetTerrainCapabilities_Implementation(); }

    UFUNCTION(BlueprintCallable, Category="Terrain")
    EWyrmTerrainSubmitResult ExecuteTerrainEdit(const FWyrmTerrainEditRequest& Request) { return SubmitTerrainEdit_Implementation(Request); }

    UFUNCTION(BlueprintCallable, Category="Terrain")
    bool CheckVolumeOccupied(const FVector& Center, float RadiusCm) const { return IsVolumeOccupied_Implementation(Center, RadiusCm); }

    UFUNCTION(BlueprintCallable, Category="Terrain")
    bool QueryLastYield(const FGuid& ActionId, FWyrmVoxelYield& OutYield) const { return GetLastYield_Implementation(ActionId, OutYield); }

    UFUNCTION(BlueprintPure, Category="Terrain")
    bool HasPendingTerrainEdits() const { return PendingCompletionRequests.Num() > 0; }

    // --- Persistence ---
    /** Captures current terrain state and yield tracking into a binary payload */
    UFUNCTION(BlueprintCallable, Category="Terrain|Persistence")
    bool BuildSavePayload(TArray<uint8>& OutBytes);

    /** Restores terrain state and yield tracking from a binary payload */
    UFUNCTION(BlueprintCallable, Category="Terrain|Persistence")
    bool ApplySavePayload(const TArray<uint8>& InBytes);

    /** Set of processed action IDs to prevent duplicate transactions */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Terrain|State")
    TSet<FGuid> ProcessedActionIds;

    /** Recorded yields keyed by action ID */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Terrain|State")
    TMap<FGuid, FWyrmVoxelYield> ActionYields;

private:
    bool IsTerrainGeometryReady() const;
    bool TryFinalizePendingTerrainEdits();
    int32 CountFilledCellsInSphere(const FVector& Center, float RadiusCm, float& OutCellVolumeCm3) const;
    void FailPendingTerrainEdits();

    TArray<FWyrmTerrainEditRequest> PendingCompletionRequests;
    bool bNavigationRefreshSubmitted = false;
    float PendingCompletionAgeSeconds = 0.f;
    static constexpr float CompletionTimeoutSeconds = 30.f;
};
